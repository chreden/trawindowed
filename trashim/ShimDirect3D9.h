#pragma once

#include <d3d9.h>
#include <atlbase.h>
#include <cstdint>

struct ShimDirect3D9 : IDirect3D9
{
    CComPtr<IDirect3D9> _d3d9;
    uint32_t _ref{ 1 };

    ShimDirect3D9(IDirect3D9* d3d9)
        : _d3d9(d3d9)
    {
    }

    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
    {
        return _d3d9->QueryInterface(riid, ppvObj);
    }

    STDMETHOD_(ULONG, AddRef)(THIS)
    {
        InterlockedIncrement(&_ref);
        return _ref;
    }

    STDMETHOD_(ULONG, Release)(THIS)
    {
        ULONG ulRefCount = InterlockedDecrement(&_ref);
        if (0 == _ref)
        {
            delete this;
        }
        return ulRefCount;
    }

    /*** IDirect3D9 methods ***/
    STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction)
    {
        return _d3d9->RegisterSoftwareDevice(pInitializeFunction);
    }

    STDMETHOD_(UINT, GetAdapterCount)(THIS)
    {
        return _d3d9->GetAdapterCount();
    }

    STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier)
    {
        return _d3d9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
    }

    STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter, D3DFORMAT Format)
    {
        return _d3d9->GetAdapterModeCount(Adapter, Format);
    }

    STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
    {
        return _d3d9->EnumAdapterModes(Adapter, Format, Mode, pMode);
    }

    STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE* pMode)
    {
        return _d3d9->GetAdapterDisplayMode(Adapter, pMode);
    }

    STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
    {
        return _d3d9->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
    }

    STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
    {
        return _d3d9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
    }

    STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
    {
        return _d3d9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
    }

    STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
    {
        return _d3d9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
    }

    STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
    {
        return _d3d9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
    }

    STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps)
    {
        return _d3d9->GetDeviceCaps(Adapter, DeviceType, pCaps);
    }

    STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter)
    {
        return _d3d9->GetAdapterMonitor(Adapter);
    }

    STDMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);
};
