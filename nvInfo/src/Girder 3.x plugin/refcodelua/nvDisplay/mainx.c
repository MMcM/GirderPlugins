/* Example of how to implement Lua extensions in Girder plugins
   6 October 2002
   Copyright 2002 (c) Ron Bessems
*/


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <lua.h>
#include <lauxlib.h>
#include <girder.h>

#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <stdarg.h>
#include "NvCpl.h"



HWND TargetWND=0;

t_functions_2 sf;
HINSTANCE     hInstance;

int detect(char *result);


// **************************************************************************
// Exported Lua functions 
// **************************************************************************


// In: (string)text, (string)caption, (number)type.
// text, the text that appears on the dialog
// caption, the caption on the dialog
// type:
/*
2 MB_ABORTRETRYIGNORE The message box contains three push buttons: Abort, Retry, and Ignore. 
0 MB_OK The message box contains one push button: OK. This is the default. 
1 MB_OKCANCEL The message box contains two push buttons: OK and Cancel. 
5 MB_RETRYCANCEL The message box contains two push buttons: Retry and Cancel. 
4 MB_YESNO The message box contains two push buttons: Yes and No. 
3 MB_YESNOCANCEL 
0x30 MB_ICONEXCLAMATION, 
0x30 MB_ICONWARNING An exclamation-point icon appears in the message box. 
0x40 MB_ICONINFORMATION, 
0x40 MB_ICONASTERISK An icon consisting of a lowercase letter i in a circle appears in the message box. 
0x20 MB_ICONQUESTION A question-mark icon appears in the message box. 
0x10 MB_ICONSTOP, 
0x10 MB_ICONERROR, 
0x10 MB_ICONHAND 
0     MB_DEFBUTTON1 The first button is the default button. 
0x100 MB_DEFBUTTON2 The second button is the default button. 
0x200 MB_DEFBUTTON3 The third button is the default button. 
0x300 MB_DEFBUTTON4 The fourth button is the default button. 
*/

// Out: button pressed can be:
/*

3 IDABORT Abort button was selected. 
2 IDCANCEL Cancel button was selected. 
5 IDIGNORE Ignore button was selected. 
7 IDNO No button was selected. 
1 IDOK OK button was selected. 
4 IDRETRY Retry button was selected. 
6 IDYES Yes button was selected. 
*/ 

static int lua_NVInfo (lua_State *L)
{
   int o,n=MB_ICONINFORMATION;
   char buffer [DEVSSTRING];
        
   n = lua_gettop(L);

   //lets leave args in here for future use, no args now
   if ( n > 0)
   {
      lua_error(L, "No args allowed yet...");
      return 0;
   }

//   if ( n == 1 )
//   {
//
//      if ( lua_isnumber(L, 1) )
//      {
//         o = (int)lua_tonumber(L,1);
//      }
//
//   }

   
   if (o == 0) {
	if (detect( buffer ))
		lua_error(L, "Error opening nVidia CPL API");
   }
//   else
//    if (o==1) 
//	{
//    _strtime( buffer );
//	}
	else
	   {
		lua_error(L, "Incorrect value of arguments");
        return 0;
		}

   // return value
   lua_pushstring(L,buffer);
   return 1;
}

static int lua_Delay (lua_State *L)
{
   int n=MB_ICONINFORMATION;
   long milli;
   clock_t end;
     
   n = lua_gettop(L);

   if ( n == 0 || n > 1 )
   {
      lua_error(L, "Incorrect number of arguments");
      return 0;
   }

   if ( n == 1 )
   {

      if ( lua_isnumber(L, 1) )
      {
         milli = (int)lua_tonumber(L,1);
      }

   }

   end = clock() + milli;

   while ( clock() < end ) ;


   return 1;
}

// **************************************************************************
// Lua support functions
// **************************************************************************



static const struct luaL_reg lib[] = {
{"NVInfo", lua_TimeDate},
//{"Delay", lua_Delay},
};



int WINAPI lua_NVInfoLibOpen(lua_State *L)
{
   luaL_openl(L, lib);
   return 0;
}


int WINAPI lua_NVInfoLibClose(lua_State *L)
{
   luaL_closel(L, lib);
   return 0;
}


// **************************************************************************
// Girder functions
// **************************************************************************


int WINAPI gir_requested_api(int maxapi)
{
	return 2;
}

int WINAPI gir_devicenum()
{
	return 2000;
}

void WINAPI gir_description(PCHAR Buffer, BYTE Length)
{
	strncpy(Buffer, "Plugin written Sundanse, Help from nVidia ref code", Length);
}

void WINAPI gir_name(PCHAR Buffer, BYTE Length)
{
	strncpy(Buffer, "Nvidia Display info plugin", Length);
}

void WINAPI gir_version(PCHAR Buffer, BYTE Length)
{
	strncpy(Buffer, "1.0", Length);
}



int WINAPI gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions_2 p)
{
   void *L;
   
   p_functions_2 h = (p_functions_2 )	p;
	if ( h->size != sizeof( t_functions_2 ) )
	{
		return GIR_FALSE;
	}

   sf = *p;

   TargetWND = sf.parent_hwnd;

   L = sf.get_script_state();

   if ( L!=NULL ) 
     lua_NVInfoLibOpen( L );
   else
      return GIR_FALSE;

   return GIR_TRUE;
}




int WINAPI gir_close()
{
   void *L;

   L = sf.get_script_state();

   if ( L!=NULL ) 
     lua_NVInfoLibClose( L );
   else
     return GIR_FALSE;

   return GIR_TRUE;
}




int WINAPI gir_info(int message, int wparam, int lparam)
{
   void *L;

   switch ( message )
   {
   case GIRINFO_SCRIPT_STATE: 
      // this means the lua scripting engine was reset ( Reset variables from Girder )
      // we now have to re-register our functions.
      L = (void*)lparam;

      if ( L!=NULL ) 
        lua_NVInfoLibOpen( L );
      else
         return GIR_FALSE;      
      
      break;
   case GIRINFO_MAINWIN_OPEN:
      TargetWND =(HWND)wparam;
      break;
   case GIRINFO_MAINWIN_CLOSE:
      TargetWND = sf.parent_hwnd;
      break;
   }

     
   return GIR_TRUE;
}





/* Called by windows */
BOOL WINAPI DllMain( HANDLE hModule, 
                        DWORD fdwreason,  LPVOID lpReserved )
{

	switch(fdwreason) {
    case DLL_PROCESS_ATTACH:
		hInstance=(HINSTANCE)hModule;
    break;
    case DLL_THREAD_ATTACH:
    break;
    case DLL_THREAD_DETACH:
    break;
    case DLL_PROCESS_DETACH:		
    break;
    }
    return TRUE;
}


/*******************************************************************************

    NV control panel code

*******************************************************************************/

static void InitInfo(HINSTANCE);
static void DesktopConfiguration(HINSTANCE);
static void GPUConfiguration(HINSTANCE);


int detect(char *result)
{
	if (!result)
		return;

    // Load NVCPL library
    HINSTANCE hLib = ::LoadLibrary("NVCPL.dll");
    if (hLib == 0) {
        return LOADLIBERR;
    }
    // MMW: It's possible to load nvcpl even when current the graphics 
    // card is not Nvidia: therefore also need to check for Nvidia 
    // card to make sure the results of these queries reflect reality.

	//not calling this yet
	//InitInfo(hLib);

	// Query desktop settings
    DesktopConfiguration(hLib, result);

    // Free NVCPL library
    ::FreeLibrary(hLib);

	return 0;
}

/*******************************************************************************

    Initial Info Display
	*Not called yet*

*******************************************************************************/

static void InitInfo(HINSTANCE hLib)
{	
	// Get the NvGetDisplayInfo function pointer from the library
	NVDISPLAYINFO displayInfo = {0};
    fNvGetDisplayInfo pfNvGetDisplayInfo = (fNvGetDisplayInfo)::GetProcAddress(hLib, "NvGetDisplayInfo");
    if (pfNvGetDisplayInfo == NULL)
        return;
    else {
		// displayInfo.cbSize must be set to size of structure
		// displayInfo.dwInputFields1 must be set before call to indicate which fields to retrieve
		// displayInfo.dwOutputFields1 will be set on return to indicate which fields were successfully retrived
		// see NVDISPLAYINFO1_* bit definitions for field information, use 0xffffffff to retrieve all fields
		memset(&displayInfo, 0, sizeof(displayInfo));
		displayInfo.cbSize = sizeof(displayInfo);
		displayInfo.dwInputFields1 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		displayInfo.dwInputFields2 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		if (!pfNvGetDisplayInfo("0", &displayInfo))
			printt("Unable to retrieve display info\n");
        else {
			printt("Display Adapter : %s\n", displayInfo.szAdapterName);
			if ((displayInfo.dwOutputFields1 & NVDISPLAYINFO1_BOARDTYPE) != 0) // not supported by all drivers
			{
				printt("Display Board : ");
				switch (displayInfo.dwBoardType)
				{
					case NVBOARDTYPE_GEFORCE:
						printt("GeForce");
						break;
					case NVBOARDTYPE_QUADRO:
						printt("Quadro");
						break;
					case NVBOARDTYPE_NVS:
						printt("NVS");
						break;
					default:
						printt("0x%08lX", displayInfo.dwBoardType);
						break;
				}
				printt("\n");
			}
			printt("Display Driver : %s\n", displayInfo.szDriverVersion);
		}
	}
}


/*******************************************************************************

    Desktop configuration

*******************************************************************************/

static void TestSetting(fdtcfgex, const char*, const char*, const char*, float);
static void SetPrimaryDisplayState(fdtcfgex, int);

static void DesktopConfiguration(HINSTANCE hLib, char *result)
{
    printt("\nDESKTOP CONFIGURATION:\n");

    // Get the NvCplGetRealConnectedDevicesString function pointer from the library
    fNvCplGetRealConnectedDevicesString pfNvCplGetRealConnectedDevicesString = (fNvCplGetRealConnectedDevicesString)::GetProcAddress(hLib, "NvCplGetRealConnectedDevicesString");
    if (pfNvCplGetRealConnectedDevicesString == NULL)
        printt("- Unable to get a pointer to NvCplGetRealConnectedDevicesString\n");
    else {
        char connectedDevices[1024];
        //pfNvCplGetRealConnectedDevicesString(connectedDevices, 1024, FALSE);
        //printt("- Connected devices: %s\n", connectedDevices);
        pfNvCplGetRealConnectedDevicesString(connectedDevices, DEVSSTRING/*1024*/, TRUE);
        //printt("- Active connected devices: ", connectedDevices);
        //printf("%s\n", connectedDevices);
		//return active connected devices string
		return connectedDevices;
    }

    return;

	/*************Not using the functionality below*************/
	// Get the NvGetDisplayInfo function pointer from the library
	NVDISPLAYINFO displayInfo = {0};
    int currentDisplayState = NVDISPLAYMODE_NONE;
    int testDisplayState = NVDISPLAYMODE_NONE;
    fNvGetDisplayInfo pfNvGetDisplayInfo = (fNvGetDisplayInfo)::GetProcAddress(hLib, "NvGetDisplayInfo");
    if (pfNvGetDisplayInfo == NULL)
        printt("- Unable to get a pointer to NvGetDisplayInfo\n");
    else {
		// displayInfo.cbSize must be set to size of structure
		// displayInfo.dwInputFields1 must be set before call to indicate which fields to retrieve
		// displayInfo.dwOutputFields1 will be set on return to indicate which fields were successfully retrived
		// see NVDISPLAYINFO1_* bit definitions for field information, use 0xffffffff to retrieve all fields
		memset(&displayInfo, 0, sizeof(displayInfo));
		displayInfo.cbSize = sizeof(displayInfo);
		displayInfo.dwInputFields1 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		displayInfo.dwInputFields2 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		if (!pfNvGetDisplayInfo("0", &displayInfo))
			printt("- Unable to retrieve display info\n");
        else {
			printt ("- Primary Display:\n");
			// Get the current mode view and arbitrarily decide of a new test mode
			switch (displayInfo.nDisplayMode) {
			case NVDISPLAYMODE_STANDARD:
	            printt("    Display Mode: Standard\n");
				testDisplayState = NVDISPLAYMODE_CLONE;
				break;
			case NVDISPLAYMODE_DUALVIEW:
				printt("    Display Mode: Dualview\n");
				testDisplayState = NVDISPLAYMODE_CLONE;
				break;
			case NVDISPLAYMODE_CLONE:
	            printt("    Display Mode: Clone\n");
				testDisplayState = NVDISPLAYMODE_HSPAN;
				break;
			case NVDISPLAYMODE_HSPAN:
	            printt("    Display Mode: Horizontal Span\n");
				testDisplayState = NVDISPLAYMODE_VSPAN;
				break;
			case NVDISPLAYMODE_VSPAN:
	            printt("    Display Mode: Vertical Span\n");
				testDisplayState = NVDISPLAYMODE_STANDARD;
				break;
			case NVDISPLAYMODE_NONE:
			default:
	            printt("    Display Mode: None\n");
				break;
			}
			if (displayInfo.nDisplayType != NVDISPLAYTYPE_NONE)
			{
				printt("    Display Type: ");
				switch (displayInfo.nDisplayType & NVDISPLAYTYPE_CLASS_MASK)
				{
					case NVDISPLAYTYPE_CRT:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_CRT)
						{
							printt("Cathode Ray Tube (CRT)");
						}
						else
						{
							printt("Cathode Ray Tube (CRT) [subtype: 0x%04X]",
							displayInfo.nDisplayType);
						}
						break;
					case NVDISPLAYTYPE_DFP:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_DFP)
						{
							printt("Digital Flat Panel (DFP)");
						}
						else if (displayInfo.nDisplayType == NVDISPLAYTYPE_DFP_LAPTOP)
						{
							printt("Laptop Display Panel");
						}
						else
						{
							printt("Digital Flat Panel (DFP) [subtype: 0x%04X]",
							displayInfo.nDisplayType);
						}
						break;
					case NVDISPLAYTYPE_TV:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_TV)
						{
							printt("Television");
						}
						else if (displayInfo.nDisplayType == NVDISPLAYTYPE_TV_HDTV)
						{
							printt("High-Definition Television (HDTV)");
						}
						else
						{
							printt("Television [subtype: 0x%04X]", displayInfo.nDisplayType);
						}
						break;
					default:
						printt("0x%04X", displayInfo.nDisplayType);
						break;
				}
				printt("\n");
			}
			// Dump current mode information to stdout
			printt("    Current Resolution: %ld x %ld pixels\n" , displayInfo.dwVisiblePelsWidth, displayInfo.dwVisiblePelsHeight);
			printt("    Current Depth: %ld-bit\n" , displayInfo.dwBitsPerPel);
			printt("    Current Refresh Rate: %ld Hz\n" , displayInfo.dwDisplayFrequency);
			printt("    Current Rotation: %ld-degrees\n" , displayInfo.dwDegreesRotation);
			printt("    Current Pannable: %ld x %ld pixels\n" , displayInfo.dwPelsWidth, displayInfo.dwPelsHeight);
			printt("    Current Rectangle: (%ld,%ld)-(%ld,%ld)\n", displayInfo.rcDisplayRect.left, displayInfo.rcDisplayRect.top, displayInfo.rcDisplayRect.right, displayInfo.rcDisplayRect.bottom);
		}
	}

	return;
}
