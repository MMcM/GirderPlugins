// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__9BCA5FB7_66F3_4557_85BD_37C761365E4C__INCLUDED_)
#define AFX_STDAFX_H__9BCA5FB7_66F3_4557_85BD_37C761365E4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <afxwin.h>
#include <afxdisp.h>

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>

#import "..\..\..\..\J River\Media Jukebox\Media Jukebox.tlb" no_namespace, named_guids

#define REGISTRY_PATH_MJ_PLUGINS_INTERFACE	"Software\\J. River\\Music Exchange\\1.0\\Media Jukebox\\Plugins\\Interface\\"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9BCA5FB7_66F3_4557_85BD_37C761365E4C__INCLUDED)
