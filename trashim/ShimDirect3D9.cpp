#include "stdafx.h"
#include "ShimDirect3D9.h"
#include "ShimDirect3DDevice9.h"
#include "TRAWindowed.h"

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

