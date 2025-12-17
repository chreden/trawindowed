#include "stdafx.h"
#include "TRAWindowed.h"

#include <detours.h>
#include <windowsx.h>
#include <shellapi.h>
#include <algorithm>
#include <string>

namespace trashim
{
    namespace
    {
        // Whether or not to capture the mouse when the cursor is moved (apply a clip rectangle).
        // This is disabled when the window has lost focus.
        bool mouse_captured{ false };
        bool capture_allowed{ true };
        bool camera_fix{ false };

        // Style used to toggle on and off the bordered windowed mode.
        const LONG_PTR windowed_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        HWND game_window{ nullptr };
        WNDPROC original_wndproc{ nullptr };

        uint32_t window_width{ 0u };
        uint32_t window_height{ 0u };

        // The width and height of the display adapter - the game is expecting the mouse to be moving around an
        // area of these dimensions, so if we are playing in a window smaller than this size the distance the
        // cursor has moved has to be scaled appropriately. e.g - 1920x1080 window on a 2560x1440 display requires
        // a scaling factor of 1.3333x1.3333, otherwise the mouse will move too slowly.
        // The game also regularly sets the cursor to the centre of the display adapter - and believes that the 
        // cursor is at that position. We therefore have to lie to the game and report all movement as relative to
        // the centre of the display adapter, regardless of the position and size of the window.
        uint32_t device_width{ 0u };
        uint32_t device_height{ 0u };

        uint32_t last_cursor_x{ 0u };
        uint32_t last_cursor_y{ 0u };

        // Framerate management

        // The number of nanoseconds between each frame.
        uint32_t  desired_frame_interval{ 0u };
        int64_t   timer_frequency{ 0ll };
        int64_t   timer_previous{ 0ll };
        bool      vsync_enabled{ false };
        bool      frame_timer_enabled{ false };

        // Real versions of the functions that are detoured. They are called directly by the shim dll when appropriate
        BOOL(WINAPI * RealGetCursorPos)(LPPOINT) = GetCursorPos;
        BOOL(WINAPI * RealSetCursorPos)(int, int) = SetCursorPos;
        BOOL(WINAPI * RealClipCursor)(const RECT*) = ClipCursor;

        // Stops the game from doing any cursor clipping - we do this ourselves based on whether
        // we are currently capturing the mouse.
        BOOL WINAPI FixedClipCursor(const RECT* rect)
        {
            return TRUE;
        }

        // Just adds a conversion from screen space into client space - the game does not do this
        // as it is not expecting there to be a difference.
        BOOL WINAPI FixedGetCursorPos(LPPOINT output)
        {
            auto result = RealGetCursorPos(output);
            if (!camera_fix)
            {
                ScreenToClient(game_window, output);
            }
            return result;
        }

        // Version of SetCursorPos that also does cursor clipping - but based on the position of the window
        // as well as the size. Only clips when the game has focus.
        BOOL WINAPI FixedSetCursorPos(int x, int y)
        {
            if (mouse_captured)
            {
                RECT cr;
                GetClientRect(game_window, &cr);

                POINT pt{ 0,0 };
                ClientToScreen(game_window, &pt);

                cr.left += pt.x;
                cr.right += pt.x;
                cr.top += pt.y;
                cr.bottom += pt.y;

                RealClipCursor(&cr);
            }

            return TRUE;
        }

        // Resize the window to have the desired client area, taking in to account whether the border
        // is currently active or not.
        void resize_window(uint32_t desired_width, uint32_t desired_height)
        {
            RECT rect{ 0, 0, static_cast<LONG>(desired_width), static_cast<LONG>(desired_height) };
            LONG_PTR style = GetWindowLongPtr(game_window, GWL_STYLE);
            AdjustWindowRect(&rect, style, false);
            SetWindowPos(game_window, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_NOMOVE);
        }

        // Creates an adjusted LPARAM for WM_MOUSEMOVE based on where the game thinks the cursor should be, 
        // where we know it was and where it has moved to.
        // x, y: The client space coordinates of the mouse cursor.
        LPARAM move_mouse(int32_t x, int32_t y)
        {
            // Find how far the mouse has moved since the last time we saw it, based on our last
            // recorded client space position.
            int32_t diff_x = x - last_cursor_x;
            int32_t diff_y = y - last_cursor_y;

            // Apply the scaling required to translate window cursor movement into device cursor movement.
            float scale_x = static_cast<float>(device_width) / window_width;
            float scale_y = static_cast<float>(device_height) / window_height;

            diff_x = static_cast<int32_t>(diff_x * scale_x);
            diff_y = static_cast<int32_t>(diff_y * scale_y);

            // The game expects the cursor to be in the centre of the device, so report all cursor positions
            // as relative to that coordinate.
            auto new_x = device_width / 2 + diff_x;
            auto new_y = device_height / 2 + diff_y;

            // Set the cursor position to the centre of the actual client (so user can click and
            // still be in the window).
            POINT pt = { static_cast<LONG>(window_width / 2), static_cast<LONG>(window_height / 2) };

            // Store client space coordinates for comparison next time (before we make them screen space).
            last_cursor_x = pt.x;
            last_cursor_y = pt.y;

            // Pin the mouse in the centre of the client space.
            ClientToScreen(game_window, &pt);
            RealSetCursorPos(pt.x, pt.y);

            return MAKELPARAM(new_x, new_y);
        }

        void hide_cursor()
        {
            while (ShowCursor(false) >= 0) {}
        }

        void show_cursor()
        {
            while (ShowCursor(true) < 0) {}
        }

        void capture_mouse()
        {
            // Allow the user to give control
            if (!mouse_captured && capture_allowed)
            {
                mouse_captured = true;
                hide_cursor();
            }
        }

        void centre_window()
        {
            RECT rect{ 0, 0, static_cast<LONG>(window_width), static_cast<LONG>(window_height) };
            LONG_PTR style = GetWindowLongPtr(game_window, GWL_STYLE);
            AdjustWindowRect(&rect, style, false);

            const HMONITOR monitor = MonitorFromWindow(game_window, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO info{ .cbSize = sizeof(MONITORINFO) };
            GetMonitorInfo(monitor, &info);

            const LONG screen_width = info.rcMonitor.right - info.rcMonitor.left;
            const LONG screen_height = info.rcMonitor.bottom - info.rcMonitor.top;
            const LONG width = rect.right - rect.left;
            const LONG height = rect.bottom - rect.top;

            const LONG x = info.rcMonitor.left + (screen_width / 2) - (width / 2);
            const LONG y = info.rcMonitor.top + (screen_height / 2) - (height / 2);
            SetWindowPos(game_window, nullptr, x, y, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
        }

        void release_mouse()
        {
            mouse_captured = false;
            RealClipCursor(nullptr);
            show_cursor();
        }

        void toggle_border()
        {
            auto style = GetWindowLongPtr(game_window, GWL_STYLE);
            SetWindowLongPtr(game_window, GWL_STYLE, style & windowed_style ? (style & ~windowed_style) : (style | windowed_style));
            resize_window(window_width, window_height);
        }

        void toggle_on_top()
        {
            const auto style = GetWindowLongPtr(game_window, GWL_EXSTYLE);
            if (style & WS_EX_TOPMOST)
            {
                SetWindowPos(game_window, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
                BringWindowToTop(game_window);
            }
            else
            {
                SetWindowPos(game_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
            }
        }

        void toggle_capture()
        {
            capture_allowed = !capture_allowed;
            if (capture_allowed)
            {
                capture_mouse();
            }
            else
            {
                release_mouse();
            }
        }

        void initialise_timer()
        {
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency(&frequency);
            timer_frequency = frequency.QuadPart;

            LARGE_INTEGER counter;
            QueryPerformanceCounter(&counter);
            timer_previous = counter.QuadPart;
        }

        void update_timer()
        {
            LARGE_INTEGER current_time;
            QueryPerformanceCounter(&current_time);
            timer_previous = current_time.QuadPart;
        }

        LRESULT CALLBACK subclass_wndproc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg)
            {
                case WM_KEYDOWN:
                {
                    if (wParam == VK_F6)
                    {
                        if (GetKeyState(VK_SHIFT) & 0x8000)
                        {
                            centre_window();
                        }
                        else
                        {
                            toggle_border();
                        }
                    }
                    else if (wParam == VK_F7)
                    {
                        toggle_on_top();
                    }
                    else if (wParam == VK_F8)
                    {
                        toggle_capture();
                    }
                    break;
                }
                case WM_SETFOCUS:
                {
                    capture_mouse();
                    break;
                }
                case WM_KILLFOCUS:
                {
                    release_mouse();
                    break;
                }
                case WM_LBUTTONDOWN:
                {
                    capture_mouse();
                    break;
                }
                case WM_MOUSEMOVE:
                {
                    if (mouse_captured)
                    {
                        int32_t x = GET_X_LPARAM(lParam);
                        int32_t y = GET_Y_LPARAM(lParam);
                        lParam = move_mouse(x, y);
                        break;
                    }
                    else
                    {
                        // Don't let the game handle this message - everything would go very badly wrong.
                        return DefWindowProc(window, msg, wParam, lParam);
                    }
                }
            }
            return CallWindowProc(original_wndproc, window, msg, wParam, lParam);
        }

        bool is_arg_present(const wchar_t* arg)
        {
            int number_of_arguments = 0;
            const auto args = CommandLineToArgvW(GetCommandLine(), &number_of_arguments);
            return args != nullptr && std::any_of(args, args + number_of_arguments, [arg](auto str) { return wcsstr(str, arg) != nullptr; });
        }

        bool should_start_borderless()
        {
            return is_arg_present(L"-borderless");
        }

        bool is_camera_fix_enabled()
        {
            return is_arg_present(L"-camerafix");
        }

        bool is_centering_enabled()
        {
            return is_arg_present(L"-autocenter");
        }

        bool is_aspect_fix_disabled()
        {
            return is_arg_present(L"-disableaspectfix");
        }

        void apply_aspect_ratio()
        {
            wchar_t path[MAX_PATH];
            if (!is_aspect_fix_disabled() &&
                0 != GetModuleFileName(NULL, path, MAX_PATH) &&
                std::wstring(path).contains(L"tru.exe"))
            {
                // Some sort of graphics related struct pointer - at 0x170 offset is the aspect ratio x 10000.
                char* ptr = *reinterpret_cast<char**>(0x00ad75e4);
                int* aspect_ratio = reinterpret_cast<int*>(ptr + 0x170);
                *aspect_ratio = (static_cast<float>(window_width) / static_cast<float>(window_height)) * 10000;
            }
        }

        bool should_start_ontop()
        {
            return is_arg_present(L"-ontop");
        }

        bool should_start_without_mouse_capture()
        {
            return is_arg_present(L"-nomousecapture");
        }
    }

    void initialise_shim(HWND window, uint32_t back_buffer_width, uint32_t back_buffer_height, uint32_t display_width, uint32_t display_height, bool vsync, uint32_t framerate)
    {
        // Only set the windowed style the first time the window is seen.
        bool need_auto_centering = false;
        if (!game_window)
        {
            camera_fix = is_camera_fix_enabled();

            game_window = window;
            SetWindowLongPtr(game_window, GWL_STYLE, windowed_style);
            if (should_start_borderless())
            {
                toggle_border();
            }

            if (!should_start_ontop())
            {
                toggle_on_top();
            }

            if (!should_start_without_mouse_capture())
            {
                capture_mouse();
            }

            need_auto_centering = is_centering_enabled();

            original_wndproc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)subclass_wndproc);

            // If our window is already the foreground window, hide the cursor now.
            if (GetForegroundWindow() == game_window)
            {
                hide_cursor();
            }

            // Only detour the first time.
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)RealGetCursorPos, FixedGetCursorPos);
            DetourAttach(&(PVOID&)RealSetCursorPos, FixedSetCursorPos);
            DetourAttach(&(PVOID&)RealClipCursor, FixedClipCursor);
            DetourTransactionCommit();

            // Create the framerate timer.
            initialise_timer();
        }

        device_width = display_width;
        device_height = display_height;
        window_width = back_buffer_width;
        window_height = back_buffer_height;
        desired_frame_interval = 1.0f / framerate * 1e7;
        vsync_enabled = vsync;
        resize_window(window_width, window_height);
        initialise_timer();
        apply_aspect_ratio();

        if (need_auto_centering)
        {
            centre_window();
        }
    }

    uint64_t get_frame_difference()
    {
        LARGE_INTEGER current_time;
        QueryPerformanceCounter(&current_time);
        int64_t difference = current_time.QuadPart - timer_previous;
        return (static_cast<double>(difference) / timer_frequency) * 1e7;
    }

    void wait_for_frame()
    {
        // Only perform fixed framerate emulation when vsync is enabled.
        if (!vsync_enabled || !frame_timer_enabled)
        {
            return;
        }

        while (get_frame_difference() < desired_frame_interval)
        {
            // Busy wait.
        }

        update_timer();
    }

    void start_fps_emulation()
    {
        frame_timer_enabled = true;
    }
}