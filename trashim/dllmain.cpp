﻿// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ShimDirect3D9.h"

extern "C"
{
    IDirect3D9 * WINAPI Direct3DCreate9(UINT SDKVersion)
    {
        // Get System32 directory
        TCHAR windir[MAX_PATH];
        GetWindowsDirectory(windir, MAX_PATH);
        LPWSTR dllpath = lstrcat(windir, L"\\System32\\d3d9.dll");
        
        // Load the real d3d9 dll.
        HMODULE mod = LoadLibraryEx(dllpath, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        FARPROC proc = GetProcAddress(mod, "Direct3DCreate9");

        // Get the real d3d9 create.
        typedef IDirect3D9* (WINAPI *func)(UINT);
        return new ShimDirect3D9(((func)proc)(SDKVersion));
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
