// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
// $Header$

#if !defined(AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_)
#define AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <shlwapi.h>

#include <windowsx.h>
#define TrackBar_GetPos(hwndCtl)          ((LONG)(DWORD)SNDMSG((hwndCtl), TBM_GETPOS, 0L, 0L))
#define TrackBar_SetPos(hwndCtl, bRedraw, lPos)    ((void)(DWORD)SNDMSG((hwndCtl), TBM_SETPOS, (WPARAM)(BOOL)(bRedraw), (LPARAM)(LONG)(lPos)))
#define TrackBar_GetRange(hwndCtl)                      ((DWORD)SNDMSG((hwndCtl), TBM_GETRANGE, 0L, 0L))
#define TrackBar_SetRange(hwndCtl, nMinimum, nMaximum)  ((void)SNDMSG((hwndCtl), TBM_SETRANGE, 0L, (LPARAM)MAKELONG((nMinimum), (nMaximum))))
#define UpDown_GetPos(hwndCtl)          ((short)(DWORD)SNDMSG((hwndCtl), UDM_GETPOS, 0L, 0L))
#define UpDown_SetPos(hwndCtl, nPos)    ((short)(DWORD)SNDMSG((hwndCtl), UDM_SETPOS, 0L, (LPARAM)MAKELONG((short)nPos, 0)))
#define UpDown_GetRange(hwndCtl)                        ((DWORD)SNDMSG((hwndCtl), UDM_GETRANGE, 0L, 0L))
#define UpDown_SetRange(hwndCtl, nUpper, nLower)        ((void)SNDMSG((hwndCtl), UDM_SETRANGE, 0L, (LPARAM)MAKELONG((nUpper), (nLower))))

#include <girder.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_)
