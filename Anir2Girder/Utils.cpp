/*********************************************************************
 *  @file   : Utils.cpp
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : Global utils and macros
 *
 *********************************************************************
 * $Id$
 ********************************************************************/


#define STRICT
#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#include <stdio.h> 
#include <stdarg.h>
#include "Utils.h"



HRESULT My_ReadStringRegKey( HKEY hKey, TCHAR* strRegName, TCHAR* strValue, 
                                 DWORD dwLength, TCHAR* strDefault )
{
    DWORD dwType;

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)strValue, &dwLength ) )
    {
        _tcscpy( strValue, strDefault );
    }

    return S_OK;
}




HRESULT My_WriteStringRegKey( HKEY hKey, TCHAR* strRegName,
                                  TCHAR* strValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_SZ, 
                                        (BYTE*)strValue, 
                                        (_tcslen(strValue)+1)*sizeof(TCHAR) ) )
        return E_FAIL;

    return S_OK;
}




HRESULT My_ReadIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD* pdwValue, 
                              DWORD dwDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(DWORD);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)pdwValue, &dwLength ) )
    {
        *pdwValue = dwDefault;
		return E_FAIL;
    }

    return S_OK;
}





/*!
 * Write int registry key
 *
 * @param hKey : Key
 * @param strRegName : Key name
 * @param dwValue : Key valkue
 *
 * @return HRESULT  : 
 */
HRESULT My_WriteIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD dwValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_DWORD, 
                                        (BYTE*)&dwValue, sizeof(DWORD) ) )
        return E_FAIL;

    return S_OK;
}




HRESULT My_ReadBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL* pbValue, 
                              BOOL bDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(BOOL);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)pbValue, &dwLength ) )
    {
        *pbValue = bDefault;
    }

    return S_OK;
}





/*!
 * Write bool registry key
 *
 * @param hKey : 
 * @param strRegName : 
 * @param bValue : 
 *
 * @return HRESULT  : 
 */
HRESULT My_WriteBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL bValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_DWORD, 
                                        (BYTE*)&bValue, sizeof(BOOL) ) )
        return E_FAIL;

    return S_OK;
}




HRESULT My_ReadGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID* pGuidValue, 
                               GUID& guidDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(GUID);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (LPBYTE) pGuidValue, &dwLength ) )
    {
        *pGuidValue = guidDefault;
    }

    return S_OK;
}





/*!
 * Write GUID registry key
 *
 * @param hKey : 
 * @param strRegName : 
 * @param guidValue : 
 *
 * @return HRESULT  : 
 */
HRESULT My_WriteGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID guidValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_BINARY, 
                                        (BYTE*)&guidValue, sizeof(GUID) ) )
        return E_FAIL;

    return S_OK;
}




/*!
 * Output debug info
 *
 * @param strFile : 
 * @param dwLine : 
 * @param hr : 
 * @param strMsg : 
 *
 * @return HRESULT  : 
 */
HRESULT _DbgOut( TCHAR* strFile, DWORD dwLine, HRESULT hr, TCHAR* strMsg )
{
    TCHAR buffer[256];
    wsprintf( buffer, _T("%s(%ld): "), strFile, dwLine );
	
    OutputDebugString( _T("*** ") );
    OutputDebugString( buffer );
    OutputDebugString( strMsg );

    if( hr )
    {
        wsprintf( buffer, _T("(hr=%08lx)\n"), hr );
        OutputDebugString( buffer );
    }

    OutputDebugString( _T("\n") );

    return hr;
}



/*!
 * Debug trace function
 *
 * @param strMsg : 
 * @param ... : 
 *
 * @return VOID  : 
 */
VOID My_Trace( TCHAR* strMsg, ... )
{
#if defined(DEBUG) | defined(_DEBUG)
    TCHAR strBuffer[512];

    va_list args;
    va_start(args, strMsg);
    _vsntprintf( strBuffer, 512, strMsg, args );
    va_end(args);


	OutputDebugString(_T("===> "));
    OutputDebugString( strBuffer );
#else
    UNREFERENCED_PARAMETER(strMsg);
#endif
}



/*!
 * Report error
 *
 * @param err : 
 *
 * @return void  : 
 */
void reportError(DWORD err)
{
	CString str = getErrorString(err);
	reportError(str);
}



/*!
 * Report error
 *
 * @param strErr : 
 *
 * @return void  : 
 */
void reportError(CString strErr)
{
	MYTRACE("ERROR: %s\n", strErr);
	AfxMessageBox("ERROR: " + strErr, MB_ICONERROR | MB_OK);
}




/*!
 * Report warning
 *
 * @param strWarn : 
 *
 * @return void  : 
 */
void reportWarning(CString strWarn)
{
	MYTRACE("WARN: %s\n", strWarn);
	AfxMessageBox("Warn: " + strWarn, MB_ICONWARNING | MB_OK);
}




/*!
 * Report info
 *
 * @param strInfo : 
 *
 * @return void  : 
 */
void reportInfo(CString strInfo)
{
	MYTRACE("INFO: %s\n", strInfo);
	AfxMessageBox("INFO: " + strInfo, MB_ICONINFORMATION | MB_OK);
}



/*!
 * Get error string from DWORD
 *
 * @param err : 
 *
 * @return CString  : 
 */
CString getErrorString(DWORD err)
{
	CString s;
    LPTSTR msg;
    if(::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		0,
		(LPTSTR)&msg,
		0,
		NULL) == 0)
	{ /* FormatMessage failed */
		 CString fmt;
		 fmt.LoadString(IDS_FORMAT_ERROR);
		 s.Format(fmt, err, (err & 0x0FFFFFFF));
		 return s;
	} /* FormatMessage failed */
     else
	{ /* Success */
		 // Delete the gratuitous CRLF
		 LPTSTR p = _tcschr(msg, _T('\r'));
		 if(p != NULL)
			*p = _T('\0');
		 s = msg;
		 ::LocalFree(msg);
		 return s;
	} /* Success */
}


