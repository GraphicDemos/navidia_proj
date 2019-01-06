//-----------------------------------------------------------------------------
// File: DXUtil.cpp
//
// Desc: Shortcut macros and functions for using DirectX objects
//
// Copyright (c) Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------

#include "Textures.h"

#include <tchar.h>
#include <stdarg.h>

#include "DXUtil.h"


//-----------------------------------------------------------------------------
// Name: DXUtil_ConvertAnsiStringToWide
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       WCHAR string. 
//       cchDestChar is the size in TCHARs of wstrDestination.  Be careful not to 
//       pass in sizeof(strDest) 
//-----------------------------------------------------------------------------
HRESULT DXUtil_ConvertAnsiStringToWide( WCHAR* wstrDestination, const CHAR* strSource, 
                                     int cchDestChar )
{
    if( wstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

    int nResult = MultiByteToWideChar( CP_ACP, 0, strSource, -1, 
                                       wstrDestination, cchDestChar );
    wstrDestination[cchDestChar-1] = 0;
    
    if( nResult == 0 )
        return E_FAIL;
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DXUtil_GetDXSDKMediaPath()
// Desc: Returns the DirectX SDK media path
//-----------------------------------------------------------------------------
const TCHAR* DXUtil_GetDXSDKMediaPath()
{
    static TCHAR strNull[2] = {0};
    static TCHAR strPath[MAX_PATH + 10];
    HKEY  hKey=0;
    DWORD type=0, size=MAX_PATH;

    strPath[0] = 0;     // Initialize to NULL
    
    // Open the appropriate registry key
    LONG result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                _T("Software\\Microsoft\\DirectX SDK"),
                                0, KEY_READ, &hKey );
    if( ERROR_SUCCESS != result )
        return strNull;

    result = RegQueryValueEx( hKey, _T("DX9S4SDK Samples Path"), NULL,
                              &type, (BYTE*)strPath, &size );

    if( ERROR_SUCCESS != result )
    {
        size = MAX_PATH;    // Reset size field
        result = RegQueryValueEx( hKey, _T("DX81SDK Samples Path"), NULL,
                                  &type, (BYTE*)strPath, &size );

        if( ERROR_SUCCESS != result )
        {
            size = MAX_PATH;    // Reset size field
            result = RegQueryValueEx( hKey, _T("DX8SDK Samples Path"), NULL,
                                      &type, (BYTE*)strPath, &size );

            if( ERROR_SUCCESS != result )
            {
                RegCloseKey( hKey );
                return strNull;
            }
        }
    }

    RegCloseKey( hKey );
    lstrcat( strPath, _T("\\Media\\\0") );

    return strPath;
}


