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

#ifndef PSM_INSERTPAGE
// If not compiled with a recent PRSHT.H, include some updates here.
// These messages should be present if a recent version of IE has been
// installed.

#define PSM_INSERTPAGE          (WM_USER + 119)
#define PropSheet_InsertPage(hDlg, index, hpage) \
        SNDMSG(hDlg, PSM_INSERTPAGE, (WPARAM)(index), (LPARAM)(hpage))


#if (_WIN32_IE >= 0x0500)
#define PSM_SETHEADERTITLEA     (WM_USER + 125)
#define PSM_SETHEADERTITLEW     (WM_USER + 126)

#ifdef UNICODE
#define PSM_SETHEADERTITLE      PSM_SETHEADERTITLEW
#else
#define PSM_SETHEADERTITLE      PSM_SETHEADERTITLEA
#endif

#define PropSheet_SetHeaderTitle(hDlg, index, lpszText) \
        SNDMSG(hDlg, PSM_SETHEADERTITLE, (WPARAM)(index), (LPARAM)(lpszText))


#define PSM_SETHEADERSUBTITLEA     (WM_USER + 127)
#define PSM_SETHEADERSUBTITLEW     (WM_USER + 128)

#ifdef UNICODE
#define PSM_SETHEADERSUBTITLE      PSM_SETHEADERSUBTITLEW
#else
#define PSM_SETHEADERSUBTITLE      PSM_SETHEADERSUBTITLEA
#endif

#define PropSheet_SetHeaderSubTitle(hDlg, index, lpszText) \
        SNDMSG(hDlg, PSM_SETHEADERSUBTITLE, (WPARAM)(index), (LPARAM)(lpszText))

#define PSM_HWNDTOINDEX            (WM_USER + 129)
#define PropSheet_HwndToIndex(hDlg, hwnd) \
        (int)SNDMSG(hDlg, PSM_HWNDTOINDEX, (WPARAM)(hwnd), 0)

#define PSM_INDEXTOHWND            (WM_USER + 130)
#define PropSheet_IndexToHwnd(hDlg, i) \
        (HWND)SNDMSG(hDlg, PSM_INDEXTOHWND, (WPARAM)(i), 0)

#define PSM_PAGETOINDEX            (WM_USER + 131)
#define PropSheet_PageToIndex(hDlg, hpage) \
        (int)SNDMSG(hDlg, PSM_PAGETOINDEX, 0, (LPARAM)(hpage))

#define PSM_INDEXTOPAGE            (WM_USER + 132)
#define PropSheet_IndexToPage(hDlg, i) \
        (HPROPSHEETPAGE)SNDMSG(hDlg, PSM_INDEXTOPAGE, (WPARAM)(i), 0)

#define PSM_IDTOINDEX              (WM_USER + 133)
#define PropSheet_IdToIndex(hDlg, id) \
        (int)SNDMSG(hDlg, PSM_IDTOINDEX, 0, (LPARAM)(id))

#define PSM_INDEXTOID              (WM_USER + 134)
#define PropSheet_IndexToId(hDlg, i) \
        SNDMSG(hDlg, PSM_INDEXTOID, (WPARAM)(i), 0)

#define PSM_GETRESULT              (WM_USER + 135)
#define PropSheet_GetResult(hDlg) \
        SNDMSG(hDlg, PSM_GETRESULT, 0, 0)

#define PSM_RECALCPAGESIZES        (WM_USER + 136)
#define PropSheet_RecalcPageSizes(hDlg) \
        SNDMSG(hDlg, PSM_RECALCPAGESIZES, 0, 0)
#endif // 0x0500

#endif

#include <girder4.h>
#include <dui.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#define countof(x) sizeof(x)/sizeof(x[0])

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_)
