#include "stdafx.h"
#include "ShimDirect3D9.h"
#include "ShimDirect3DDevice9.h"
#include "TRAWindowed.h"
#include <unordered_map>

namespace
{
    std::unordered_map<UINT, std::vector<D3DDISPLAYMODE>> adapter_display_modes;

    const std::vector<UINT> wanted_refresh_rates{ 20, 30, 40, 50, 60, 70, 75, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200 };

    std::vector<D3DDISPLAYMODE> store_display_modes(IDirect3D9& d3d9, UINT adapter, D3DFORMAT format)
    {
        auto found = adapter_display_modes.find(adapter);
        if (adapter_display_modes.find(adapter) != adapter_display_modes.end())
        {
            return found->second;
        }

        std::vector<D3DDISPLAYMODE> modes;
        uint32_t actual_count = d3d9.GetAdapterModeCount(adapter, format);
        for (uint32_t i = 0; i < actual_count; ++i)
        {
            D3DDISPLAYMODE mode {};
            d3d9.EnumAdapterModes(adapter, format, i, &mode);
            modes.push_back(mode);
        }

        // TODO: Find the missing framerates - we want to provide 20-200
        // TODO: Add the missing framerates.
        for (auto rate : wanted_refresh_rates)
        {
            bool found = false;
            for (const auto& mode : modes)
            {
                if (mode.Width == 1024 && mode.Height == 768 && mode.RefreshRate == rate)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                modes.push_back(D3DDISPLAYMODE{ 1024, 768, rate, format });
            }
        }

        adapter_display_modes[adapter] = modes;
        return modes;
    }
}

HRESULT ShimDirect3D9::CreateDevice(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    D3DDISPLAYMODE mode;
    _d3d9->GetAdapterDisplayMode(Adapter, &mode);

    if (hFocusWindow)
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

UINT ShimDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    auto modes = store_display_modes(*_d3d9, Adapter, Format);
    return modes.size();
}

HRESULT ShimDirect3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
    auto modes = store_display_modes(*_d3d9, Adapter, Format);
    *pMode = modes[Mode];
    return D3D_OK;
}
