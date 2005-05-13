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
#include <stdio.h>
#include <stdlib.h>
#include <dbt.h>
#include <dshow.h>

#define VTSTT_VOBS_ATTRIB_OFFSET        0x200

#define VIDEO_ATTRIB_STANDARD_PAL       0x10

#define VIDEO_ATTRIB_ASPECT_MASK        0x0c
#define VIDEO_ATTRIB_ASPECT_4X3         0x00
#define VIDEO_ATTRIB_ASPECT_16X9        0x0c

#define VIDEO_ATTRIB_DISPLAY_PANSCAN    0x02
#define VIDEO_ATTRIB_DISPLAY_LETTERBOX  0x01

#define VMG_CATEGORY_REGION_MASK_OFFSET 0x23

#include "DisplaySpyHook.h"

#include <girder4.h>
#include <dui.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C628136E_3849_4D68_B605_EBA9DE32DF55__INCLUDED_)
