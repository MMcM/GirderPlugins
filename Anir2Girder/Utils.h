/*********************************************************************
 *  @file   : Utils.h
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

#ifndef __UTILS_H__
#define __UTILS_H__

// 
// Resource
//
#define IDS_FORMAT_ERROR                WM_USER + 101



//
// Delete utils
//
//! Safe delete macro
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
//! Safe delete array macro
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }


//
// Registry utils
//
HRESULT My_WriteStringRegKey( HKEY hKey, TCHAR* strRegName, TCHAR* strValue );
HRESULT My_WriteIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD dwValue );
HRESULT My_WriteGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID guidValue );
HRESULT My_WriteBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL bValue );

HRESULT My_ReadStringRegKey( HKEY hKey, TCHAR* strRegName, TCHAR* strValue, DWORD dwLength, TCHAR* strDefault );
HRESULT My_ReadIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD* pdwValue, DWORD dwDefault );
HRESULT My_ReadGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID* pGuidValue, GUID& guidDefault );
HRESULT My_ReadBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL* pbValue, BOOL bDefault );

//
// Debug utils
//

VOID My_Trace( TCHAR* strMsg, ... );
HRESULT _DbgOut( TCHAR*, DWORD, HRESULT, TCHAR* );

//! MYTRACE macro
#if defined(DEBUG) | defined(_DEBUG)
    #define MYTRACE           My_Trace
#else
    #define MYTRACE           sizeof
#endif

#if defined(DEBUG) | defined(_DEBUG)
    #define DEBUG_MSG(str)    _DbgOut( __FILE__, (DWORD)__LINE__, 0, str )
#else
    #define DEBUG_MSG(str)    (0L)
#endif


//
// Error help
//
void reportError(DWORD err);
void reportError(CString strErr);
void reportWarning(CString strWarn);
void reportInfo(CString strInfo);
CString getErrorString(DWORD err);


#endif __UTILS_H__