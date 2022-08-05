#include "stdafx.h"
#include "ShimDirect3D9.h"
#include "ShimDirect3DDevice9.h"
#include "TRAWindowed.h"
#include <intrin.h>
#include <tlhelp32.h>
#include <tuple>

#pragma intrinsic(_ReturnAddress)

namespace
{
    std::tuple<uint32_t, uint32_t> get_action_range(HANDLE action)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, GetProcessId(GetCurrentProcess()));
        MODULEENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Module32First(snapshot, &entry))
        {
            do
            {
                if (entry.hModule == action)
                {
                    CloseHandle(snapshot);
                    return { reinterpret_cast<uint32_t>(entry.modBaseAddr), reinterpret_cast<uint32_t>(entry.modBaseAddr) + entry.modBaseSize };
                }
            } while (Module32Next(snapshot, &entry));
        }
        CloseHandle(snapshot);
        return { 0, 0 };
    }

    bool should_initialise_shim(void* caller)
    {
        HANDLE action = GetModuleHandle(L"action_x86.dll");
        if (action == INVALID_HANDLE_VALUE)
        {
            return true;
        }
        auto range = get_action_range(action);
        uint32_t return_address = reinterpret_cast<uint32_t>(caller);
        return return_address < std::get<0>(range) || return_address > std::get<1>(range);
    }
}

HRESULT ShimDirect3D9::CreateDevice(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    D3DDISPLAYMODE mode;
    _d3d9->GetAdapterDisplayMode(Adapter, &mode);

    OutputDebugString(L"Create device called\n");

    if (should_initialise_shim(_ReturnAddress()))
    {
        trashim::initialise_shim(hFocusWindow, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, mode.Width, mode.Height, pPresentationParameters->PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE, pPresentationParameters->FullScreen_RefreshRateInHz);
    }

    pPresentationParameters->Windowed = TRUE;
    pPresentationParameters->FullScreen_RefreshRateInHz = 0;
    pPresentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    IDirect3DDevice9* device = nullptr;
    HRESULT hr = _d3d9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &device);
    *ppReturnedDeviceInterface = new ShimDirect3DDevice9(device);
    return hr;
}

