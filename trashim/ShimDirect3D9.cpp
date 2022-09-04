#include "stdafx.h"
#include "ShimDirect3D9.h"
#include "ShimDirect3DDevice9.h"
#include "TRAWindowed.h"
#include <unordered_map>
#include <set>
#include <functional>

namespace
{
    struct Resolution
    {
        UINT width;
        UINT height;
        auto operator<=>(const Resolution&) const = default;
    };
}

template <>
struct std::hash<Resolution>
{
    std::size_t operator()(const Resolution& res) const noexcept
    {
        std::size_t h1 = std::hash<UINT>{}(res.width);
        std::size_t h2 = std::hash<UINT>{}(res.height);
        return h1 ^ (h2 << 1);
    }
};

namespace
{
    std::unordered_map<UINT, std::vector<D3DDISPLAYMODE>> adapter_display_modes;

    const std::vector<UINT> wanted_refresh_rates{ 20, 40, 60, 100, 144 };

    std::vector<D3DDISPLAYMODE> store_display_modes(IDirect3D9& d3d9, UINT adapter, D3DFORMAT format)
    {
        auto found = adapter_display_modes.find(adapter);
        if (adapter_display_modes.find(adapter) != adapter_display_modes.end())
        {
            return found->second;
        }

        std::unordered_map<Resolution, std::set<UINT>> framerates;
        for (uint32_t i = 0; i < d3d9.GetAdapterModeCount(adapter, format); ++i)
        {
            D3DDISPLAYMODE mode{};
            d3d9.EnumAdapterModes(adapter, format, i, &mode);
            framerates[{mode.Width, mode.Height}].insert(mode.RefreshRate);
        }

        for (auto& resolution : framerates)
        {
            for (const auto& rate : wanted_refresh_rates)
            {
                resolution.second.insert(rate);
            }
        }

        std::vector<D3DDISPLAYMODE> modes;
        for (const auto& resolution : framerates)
        {
            for (const auto& rate : resolution.second)
            {
                modes.push_back(
                    {
                        resolution.first.width,
                        resolution.first.height,
                        rate,
                        format
                    });
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
