#include "stdafx.h"
#include "ShimDirect3DDevice9.h"
#include "TRAWindowed.h"

namespace
{
    void* source = nullptr;
}

HRESULT ShimDirect3DDevice9::TestCooperativeLevel(THIS)
{
    void* caller = nullptr;
    __asm
    {
        mov eax, dword ptr[ebp]
        mov caller, eax
    }
    
    if (!source)
    {
        source = caller;
    }

    if (caller == source)
    {
        trashim::wait_for_frame();
    }
    return _device->TestCooperativeLevel();
}

HRESULT ShimDirect3DDevice9::Clear(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
    if (Color)
    {
        trashim::start_fps_emulation();
    }
    return _device->Clear(Count, pRects, Flags, Color, Z, Stencil);
}
