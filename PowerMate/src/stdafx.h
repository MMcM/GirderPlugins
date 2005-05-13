// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
// $Header$

#if !defined(AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_)
#define AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Insert your headers here
#include <windows.h>
#include <winioctl.h>

extern "C" {
#include <setupapi.h>
#include <hidsdi.h>
}

#include <stdio.h>
#include <stdlib.h>

#include <girder4.h>
#include <dui.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_)
