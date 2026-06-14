# AGENTS.md

## Overview

TRAWindowed is a **proxy/shim DLL** (`d3d9.dll`) that intercepts DirectX 9 calls to enable windowed and borderless mode for the Tomb Raider "Legend era" games (Legend, Anniversary, Underworld). The games expect exclusive fullscreen mode; this DLL tricks them into running in a window while remapping mouse, window style, framerate, and display modes.

## Build

- **Toolchain**: Visual Studio (solution `trashim.sln`). Platform toolset `v145`.
- **Target platform**: Must build as **Win32 (x86)** for the target games (32-bit). The project also defines x64 configurations but the games are 32-bit.
- **Configuration**: Use **Release | Win32**. Output is a `d3d9.dll` in the `Release/` folder (the `TargetName` is `d3d9` — it must be named `d3d9.dll` to work).
- **C++ standard**: `stdcpplatest` (C++23) — uses `<=>`, `std::ranges`-style patterns, designated initializers.
- **Precompiled headers**: `stdafx.h` is mandatory (forced PCH via `stdafx.cpp`). All `.cpp` files begin with `#include "stdafx.h"`.
- **Detours**: Microsoft Detours library is vendored at `Detours/`. The `detours.lib` is consumed from `$(SolutionDir)/detours` and headers from `$(SolutionDir)/detours/include`. This is a **pre-built 32-bit static lib** — do not replace it with an x64 build unless you also retarget the games.
- **Module definition**: `trashim.def` exports `Direct3DCreate9 @36`. The ordinal `36` matches the real d3d9.dll export and must not change.

## Architecture & Data Flow

The DLL masquerades as the system `d3d9.dll`. Control flow on injection:

1. **`dllmain.cpp`** — exports `Direct3DCreate9`. It loads the *real* `d3d9.dll` from `System32`, calls the real `Direct3DCreate9`, then wraps the returned `IDirect3D9*` in a `ShimDirect3D9`.
2. **`ShimDirect3D9`** (`ShimDirect3D9.h/.cpp`) — implements `IDirect3D9` by forwarding all calls to the real object. The only methods with custom logic:
   - `CreateDevice` — calls `trashim::initialise_shim(...)` (hooks window mode setup), forces `Windowed=TRUE`, zeroes `FullScreen_RefreshRateInHz`, and (unless `-nolimiter` is set) forces `PresentationInterval=IMMEDIATE` before creating the real device. Returns a `ShimDirect3DDevice9` wrapper.
   - `GetAdapterModeCount`/`EnumAdapterModes` — injects synthetic display modes by combining real resolutions with desired refresh rates `[20, 40, 60, 100, 144]` Hz so the game's resolution picker shows more options.
3. **`ShimDirect3DDevice9`** (`ShimDirect3DDevice9.h/.cpp`) — implements `IDirect3DDevice9` by forwarding all calls. Custom overrides:
   - `TestCooperativeLevel` — uses inline x86 `__asm` to read the **return address** from the stack (`[ebp]`) to identify which caller is the game's main loop. The first caller is recorded as `source`; only calls from that address trigger `wait_for_frame()` (fixed framerate emulation). **This inline assembly is x86-only and will not compile for x64.**
   - `Clear` — calls `start_fps_emulation()` on the first real clear with a non-zero color, which activates the frame-timer busy-wait in `wait_for_frame()`.
4. **`TRAWindowed.cpp/.h`** — the core windowed-mode logic. Key responsibilities:
   - Subclasses the game window (`SetWindowLongPtr(GWLP_WNDPROC)`) to intercept keyboard/mouse/focus messages.
   - Uses **Microsoft Detours** to hook `GetCursorPos`, `SetCursorPos`, `ClipCursor`, and `SetWindowPos`. Real pointers saved as `RealGetCursorPos` etc. The `SetWindowPos` hook redirects game-initiated `HWND_TOPMOST` calls to `HWND_NOTOPMOST` when the user hasn't requested always-on-top (the game repeatedly tries to set topmost as part of its fullscreen logic). The shim's own `set_on_top` calls `RealSetWindowPos` to bypass the hook.
   - Mouse coordinate scaling: the game thinks the cursor is on a fullscreen device (e.g. 1920×1080), but the window may be smaller. Movement deltas are scaled by `device_size / window_size`, and reported positions are relative to device center.
   - Hotkeys (handled in `subclass_wndproc`): **F6** = toggle border, **Shift+F6** = center window, **F7** = toggle always-on-top, **F8** = toggle mouse capture.
   - Command-line args (parsed via `CommandLineToArgvW`): `-borderless`, `-camerafix`, `-autocenter`, `-disableaspectfix`, `-ontop`, `-nomousecapture`, `-nolimiter`.
   - **TRU-specific memory hack**: `apply_aspect_ratio()` directly writes to a memory address (`0x00ad75e4` → offset `0x170`) in the `tru.exe` process to override the aspect ratio. This is a hardcoded address for the TRU executable only.

## Conventions

- All windowed-mode internals are in an anonymous namespace inside the `trashim` namespace in `TRAWindowed.cpp`. Public API (3 functions) is in `TRAWindowed.h`.
- `ShimDirect3D9` and `ShimDirect3DDevice9` are structs (not classes) with public COM interface methods; they forward to a `CComPtr` held real object.
- Reference counting is manual via `InterlockedIncrement`/`InterlockedDecrement` on a `_ref` field; `Release` self-deletes at ref 0.
- File encoding: `stdafx.h` and `resource.h` and `trashim.cpp` are **UTF-8 with BOM**. New files should match (MSVC with PCH is sensitive to encoding).

## Gotchas

- **x86 only**: The inline `__asm` in `TestCooperativeLevel` reads `[ebp]` to get the caller address — this is 32-bit stack-frame-specific and will not compile under x64. The x64 build configs exist in the project but are effectively non-functional for the games.
- **Detours lib is 32-bit**: `Detours/detours.lib` is a pre-built x86 static library. If you change platforms, you must rebuild Detours separately.
- **`TestCooperativeLevel` as a framerate hook**: The shim detects the game's main render loop by checking which address calls `TestCooperativeLevel`. The first caller is recorded; only that caller's calls trigger the framerate limiter. This means if another code path calls `TestCooperativeLevel` first (e.g. a startup check), the wrong address is recorded and frame limiting won't work correctly.
- **`initialise_shim` idempotency**: The window setup and Detour attachment run only once (guarded by `if (!game_window)`), but resolution/timer/framerate values are updated on every `CreateDevice` / device reset call.
- **On-top state requires Detours hook**: The game repeatedly calls `SetWindowPos(HWND_TOPMOST)` as part of its fullscreen initialization, overriding any window style the shim sets. The `FixedSetWindowPos` Detours hook intercepts these calls and redirects them to `HWND_NOTOPMOST` when `want_on_top` is false. Simply setting the window style once (e.g. in `initialise_shim`) is not sufficient — the game will re-assert topmost later.
- **Mouse capture defaults**: As of recent commits, the default state is **not on top** and **capturing mouse** (toggled by `-nomousecapture`).
- **Frame limiter is opt-out**: The `-nolimiter` flag skips forcing `PresentationInterval = IMMEDIATE` and disables the busy-wait in `wait_for_frame()`, letting D3D9/DWM handle vsync natively. Without the flag, the original behavior (custom CPU-based limiter) is preserved. This matters because the custom limiter can cause stutter on modern Windows (confirmed with TRU on Win 11).
