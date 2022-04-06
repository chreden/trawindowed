#pragma once

#include <d3d9.h>
#include <atlbase.h>
#include <cstdint>

struct ShimDirect3DDevice9 : IDirect3DDevice9
{
    CComPtr<IDirect3DDevice9> _device;
    uint32_t _ref{ 1 };

    ShimDirect3DDevice9(IDirect3DDevice9* device)
        : _device(device)
    {
    }

    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
    {
        return _device->QueryInterface(riid, ppvObj);
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

    STDMETHOD(TestCooperativeLevel)(THIS);

    STDMETHOD_(UINT, GetAvailableTextureMem)(THIS)
    {
        return _device->GetAvailableTextureMem();
    }

    STDMETHOD(EvictManagedResources)(THIS)
    {
        return _device->EvictManagedResources();
    }

    STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9)
    {
        return _device->GetDirect3D(ppD3D9);
    }

    STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps)
    {
        return _device->GetDeviceCaps(pCaps);
    }

    STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode)
    {
        return _device->GetDisplayMode(iSwapChain, pMode);
    }

    STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters)
    {
        return _device->GetCreationParameters(pParameters);
    }

    STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
    {
        return _device->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
    }

    STDMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags)
    {
        return _device->SetCursorPosition(X, Y, Flags);
    }

    STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow)
    {
        return _device->ShowCursor(bShow);
    }

    STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
    {
        return _device->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
    }

    STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
    {
        return _device->GetSwapChain(iSwapChain, pSwapChain);
    }

    STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS)
    {
        return _device->GetNumberOfSwapChains();
    }

    STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);

    STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
    {
        return _device->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
    {
        return _device->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
    }

    STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
    {
        return _device->GetRasterStatus(iSwapChain, pRasterStatus);
    }

    STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs)
    {
        return _device->SetDialogBoxMode(bEnableDialogs);
    }

    STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
    {
        return _device->SetGammaRamp(iSwapChain, Flags, pRamp);
    }

    STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
    {
        return _device->GetGammaRamp(iSwapChain, pRamp);
    }

    STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
    {
        return _device->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
    }

    STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
    {
        return _device->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
    }

    STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
    {
        return _device->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
    }

    STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
    {
        return _device->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
    }

    STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
    {
        return _device->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
    }

    STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
    {
        return _device->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
    }

    STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
    {
        return _device->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
    }

    STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
    {
        return _device->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
    }

    STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
    {
        return _device->UpdateTexture(pSourceTexture, pDestinationTexture);
    }

    STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
    {
        return _device->GetRenderTargetData(pRenderTarget, pDestSurface);
    }

    STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
    {
        return _device->GetFrontBufferData(iSwapChain, pDestSurface);
    }

    STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
    {
        return _device->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    }

    STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
    {
        return _device->ColorFill(pSurface, pRect, color);
    }

    STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
    {
        return _device->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
    }

    STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
    {
        return _device->SetRenderTarget(RenderTargetIndex, pRenderTarget);
    }

    STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
    {
        return _device->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
    }

    STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil)
    {
        return _device->SetDepthStencilSurface(pNewZStencil);
    }

    STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface)
    {
        return _device->GetDepthStencilSurface(ppZStencilSurface);
    }

    STDMETHOD(BeginScene)(THIS)
    {
        return _device->BeginScene();
    }

    STDMETHOD(EndScene)(THIS)
    {
        return _device->EndScene();
    }
    
    STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);

    STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
    {
        return _device->SetTransform(State, pMatrix);
    }

    STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
    {
        return _device->GetTransform(State, pMatrix);
    }

    STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
    {
        return _device->MultiplyTransform(State, pMatrix);
    }

    STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport)
    {
        return _device->SetViewport(pViewport);
    }

    STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport)
    {
        return _device->GetViewport(pViewport);
    }

    STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial)
    {
        return _device->SetMaterial(pMaterial);
    }

    STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial)
    {
        return _device->GetMaterial(pMaterial);
    }

    STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9* light)
    {
        return _device->SetLight(Index, light);
    }

    STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT9* light)
    {
        return _device->GetLight(Index, light);
    }

    STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable)
    {
        return _device->LightEnable(Index, Enable);
    }

    STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable)
    {
        return _device->GetLightEnable(Index, pEnable);
    }

    STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane)
    {
        return _device->SetClipPlane(Index, pPlane);
    }

    STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane)
    {
        return _device->GetClipPlane(Index, pPlane);
    }

    STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value)
    {
        return _device->SetRenderState(State, Value);
    }

    STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue)
    {
        return _device->GetRenderState(State, pValue);
    }

    STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
    {
        return _device->CreateStateBlock(Type, ppSB);
    }

    STDMETHOD(BeginStateBlock)(THIS)
    {
        return _device->BeginStateBlock();
    }

    STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB)
    {
        return _device->EndStateBlock(ppSB);
    }

    STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus)
    {
        return _device->SetClipStatus(pClipStatus);
    }

    STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus)
    {
        return _device->GetClipStatus(pClipStatus);
    }

    STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9** ppTexture)
    {
        return _device->GetTexture(Stage, ppTexture);
    }

    STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture9* pTexture)
    {
        return _device->SetTexture(Stage, pTexture);
    }

    STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
    {
        return _device->GetTextureStageState(Stage, Type, pValue);
    }

    STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
    {
        return _device->SetTextureStageState(Stage, Type, Value);
    }

    STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
    {
        return _device->GetSamplerState(Sampler, Type, pValue);
    }

    STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
    {
        return _device->SetSamplerState(Sampler, Type, Value);
    }

    STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses)
    {
        return _device->ValidateDevice(pNumPasses);
    }

    STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
    {
        return _device->SetPaletteEntries(PaletteNumber, pEntries);
    }

    STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries)
    {
        return _device->GetPaletteEntries(PaletteNumber, pEntries);
    }

    STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber)
    {
        return _device->SetCurrentTexturePalette(PaletteNumber);
    }

    STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber)
    {
        return _device->GetCurrentTexturePalette(PaletteNumber);
    }

    STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect)
    {
        return _device->SetScissorRect(pRect);
    }

    STDMETHOD(GetScissorRect)(THIS_ RECT* pRect)
    {
        return _device->GetScissorRect(pRect);
    }

    STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware)
    {
        return _device->SetSoftwareVertexProcessing(bSoftware);
    }

    STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS)
    {
        return _device->GetSoftwareVertexProcessing();
    }

    STDMETHOD(SetNPatchMode)(THIS_ float nSegments)
    {
        return _device->SetNPatchMode(nSegments);
    }

    STDMETHOD_(float, GetNPatchMode)(THIS)
    {
        return _device->GetNPatchMode();
    }

    STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
    {
        return _device->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }

    STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
    {
        return _device->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
    }

    STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
    {
        return _device->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
    {
        return _device->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
    {
        return _device->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
    }

    STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
    {
        return _device->CreateVertexDeclaration(pVertexElements, ppDecl);
    }

    STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl)
    {
        return _device->SetVertexDeclaration(pDecl);
    }

    STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl)
    {
        return _device->GetVertexDeclaration(ppDecl);
    }

    STDMETHOD(SetFVF)(THIS_ DWORD FVF)
    {
        return _device->SetFVF(FVF);
    }

    STDMETHOD(GetFVF)(THIS_ DWORD* pFVF)
    {
        return _device->GetFVF(pFVF);
    }

    STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
    {
        return _device->CreateVertexShader(pFunction, ppShader);
    }

    STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader)
    {
        return _device->SetVertexShader(pShader);
    }

    STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader)
    {
        return _device->GetVertexShader(ppShader);
    }

    STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
    {
        return _device->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
    {
        return _device->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
    {
        return _device->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
    {
        return _device->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
    {
        return _device->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
    {
        return _device->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
    {
        return _device->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
    }

    STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride)
    {
        return _device->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
    }

    STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting)
    {
        return _device->SetStreamSourceFreq(StreamNumber, Setting);
    }

    STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting)
    {
        return _device->GetStreamSourceFreq(StreamNumber, pSetting);
    }

    STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData)
    {
        return _device->SetIndices(pIndexData);
    }

    STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData)
    {
        return _device->GetIndices(ppIndexData);
    }

    STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
    {
        return _device->CreatePixelShader(pFunction, ppShader);
    }

    STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader)
    {
        return _device->SetPixelShader(pShader);
    }

    STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader)
    {
        return _device->GetPixelShader(ppShader);
    }

    STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
    {
        return _device->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
    {
        return _device->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
    }

    STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
    {
        return _device->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
    {
        return _device->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
    }

    STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
    {
        return _device->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
    {
        return _device->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
    }

    STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
    {
        return _device->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
    }

    STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
    {
        return _device->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
    }

    STDMETHOD(DeletePatch)(THIS_ UINT Handle)
    {
        return _device->DeletePatch(Handle);
    }

    STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
    {
        return _device->CreateQuery(Type, ppQuery);
    }

};

