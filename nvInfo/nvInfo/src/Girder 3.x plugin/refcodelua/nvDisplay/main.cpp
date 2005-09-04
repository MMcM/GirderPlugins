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

int detect(char *result);

HWND TargetWND=0;

t_functions_2 sf;
HINSTANCE     hInstance;


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

static int lua_nvInfo (lua_State *L)
{
   int n,o=MB_ICONINFORMATION;
   const char *c1=NULL;
   const char *c2="nvInfo";
   char Display[64] = {0};

   n = lua_gettop(L);

   if ( n == 0 || n > 1 )
   {
      lua_error(L, "Incorrect number of arguments to nvInfo");
      return 0;
   }
   
   if ( lua_isstring(L, 1) )
   {
     c1 = lua_tostring(L,1);
   }
   else
   {
     lua_error(L, "First Argument to nvInfo must be a string");
     return 0;
   }

   
 /*  if ( n == 2 || n==3 )
   {

      if ( lua_isstring(L, 2) )
      {
         c2 = lua_tostring(L,2);
      }
      else
      {
         lua_error(L, "Second Argument to MessageBox must be a string");
         return 0;
      }
   }

   if ( n == 3 )
   {

      if ( lua_isnumber(L, 3) )
      {
         o = (int)lua_tonumber(L,3);
      }

   }
*/

   //o = MessageBox(TargetWND,c1,c2,o);
   detect(Display);
   // return value
   //lua_pushnumber(L,o);
   lua_pushstring (L, (const char *)Display);
   return 1;
}


// **************************************************************************
// Lua support functions
// **************************************************************************



static const struct luaL_reg lib[] = {
{"nvInfo", lua_nvInfo},
};



int WINAPI lua_nvInfoLibOpen(lua_State *L)
{
   luaL_openl(L, lib);
   return 0;
}


int WINAPI lua_nvInfoClose(lua_State *L)
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
	strncpy(Buffer, "Plugin written Sundance", Length);
}

void WINAPI gir_name(PCHAR Buffer, BYTE Length)
{
	strncpy(Buffer, "Lua-NvInfo plugin", Length);
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
     lua_nvInfoLibOpen( (lua_State *)L );
   else
      return GIR_FALSE;

   return GIR_TRUE;
}




int WINAPI gir_close()
{
   void *L;

   L = sf.get_script_state();

   if ( L!=NULL ) 
     lua_nvInfoClose( (lua_State *)L );
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
        lua_nvInfoLibOpen( (lua_State *)L );
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
static void DesktopConfiguration(HINSTANCE hLib, char *result);


int detect(char *result)
{
	if (!result)
		return LOADLIBERR;

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
			printf("Unable to retrieve display info\n");
        else {
			printf("Display Adapter : %s\n", displayInfo.szAdapterName);
			if ((displayInfo.dwOutputFields1 & NVDISPLAYINFO1_BOARDTYPE) != 0) // not supported by all drivers
			{
				printf("Display Board : ");
				switch (displayInfo.dwBoardType)
				{
					case NVBOARDTYPE_GEFORCE:
						printf("GeForce");
						break;
					case NVBOARDTYPE_QUADRO:
						printf("Quadro");
						break;
					case NVBOARDTYPE_NVS:
						printf("NVS");
						break;
					default:
						printf("0x%08lX", displayInfo.dwBoardType);
						break;
				}
				printf("\n");
			}
			printf("Display Driver : %s\n", displayInfo.szDriverVersion);
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
    printf("\nDESKTOP CONFIGURATION:\n");

    // Get the NvCplGetRealConnectedDevicesString function pointer from the library
    fNvCplGetRealConnectedDevicesString pfNvCplGetRealConnectedDevicesString = (fNvCplGetRealConnectedDevicesString)::GetProcAddress(hLib, "NvCplGetRealConnectedDevicesString");
    if (pfNvCplGetRealConnectedDevicesString == NULL)
        printf("- Unable to get a pointer to NvCplGetRealConnectedDevicesString\n");
    else {
        char connectedDevices[1024];
        //pfNvCplGetRealConnectedDevicesString(connectedDevices, 1024, FALSE);
        //printf("- Connected devices: %s\n", connectedDevices);
        pfNvCplGetRealConnectedDevicesString(connectedDevices, DEVSSTRING/*1024*/, TRUE);
        //printf("- Active connected devices: ", connectedDevices);
        //printf("%s\n", connectedDevices);
		//return active connected devices string
		strcpy(result , (const char *)connectedDevices);
		return;
    }

    return;

	/*************Not using the functionality below*************/
	// Get the NvGetDisplayInfo function pointer from the library
	NVDISPLAYINFO displayInfo = {0};
    int currentDisplayState = NVDISPLAYMODE_NONE;
    int testDisplayState = NVDISPLAYMODE_NONE;
    fNvGetDisplayInfo pfNvGetDisplayInfo = (fNvGetDisplayInfo)::GetProcAddress(hLib, "NvGetDisplayInfo");
    if (pfNvGetDisplayInfo == NULL)
        printf("- Unable to get a pointer to NvGetDisplayInfo\n");
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
			printf("- Unable to retrieve display info\n");
        else {
			printf ("- Primary Display:\n");
			// Get the current mode view and arbitrarily decide of a new test mode
			switch (displayInfo.nDisplayMode) {
			case NVDISPLAYMODE_STANDARD:
	            printf("    Display Mode: Standard\n");
				testDisplayState = NVDISPLAYMODE_CLONE;
				break;
			case NVDISPLAYMODE_DUALVIEW:
				printf("    Display Mode: Dualview\n");
				testDisplayState = NVDISPLAYMODE_CLONE;
				break;
			case NVDISPLAYMODE_CLONE:
	            printf("    Display Mode: Clone\n");
				testDisplayState = NVDISPLAYMODE_HSPAN;
				break;
			case NVDISPLAYMODE_HSPAN:
	            printf("    Display Mode: Horizontal Span\n");
				testDisplayState = NVDISPLAYMODE_VSPAN;
				break;
			case NVDISPLAYMODE_VSPAN:
	            printf("    Display Mode: Vertical Span\n");
				testDisplayState = NVDISPLAYMODE_STANDARD;
				break;
			case NVDISPLAYMODE_NONE:
			default:
	            printf("    Display Mode: None\n");
				break;
			}
			if (displayInfo.nDisplayType != NVDISPLAYTYPE_NONE)
			{
				printf("    Display Type: ");
				switch (displayInfo.nDisplayType & NVDISPLAYTYPE_CLASS_MASK)
				{
					case NVDISPLAYTYPE_CRT:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_CRT)
						{
							printf("Cathode Ray Tube (CRT)");
						}
						else
						{
							printf("Cathode Ray Tube (CRT) [subtype: 0x%04X]",
							displayInfo.nDisplayType);
						}
						break;
					case NVDISPLAYTYPE_DFP:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_DFP)
						{
							printf("Digital Flat Panel (DFP)");
						}
						else if (displayInfo.nDisplayType == NVDISPLAYTYPE_DFP_LAPTOP)
						{
							printf("Laptop Display Panel");
						}
						else
						{
							printf("Digital Flat Panel (DFP) [subtype: 0x%04X]",
							displayInfo.nDisplayType);
						}
						break;
					case NVDISPLAYTYPE_TV:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_TV)
						{
							printf("Television");
						}
						else if (displayInfo.nDisplayType == NVDISPLAYTYPE_TV_HDTV)
						{
							printf("High-Definition Television (HDTV)");
						}
						else
						{
							printf("Television [subtype: 0x%04X]", displayInfo.nDisplayType);
						}
						break;
					default:
						printf("0x%04X", displayInfo.nDisplayType);
						break;
				}
				printf("\n");
			}
			// Dump current mode information to stdout
			printf("    Current Resolution: %ld x %ld pixels\n" , displayInfo.dwVisiblePelsWidth, displayInfo.dwVisiblePelsHeight);
			printf("    Current Depth: %ld-bit\n" , displayInfo.dwBitsPerPel);
			printf("    Current Refresh Rate: %ld Hz\n" , displayInfo.dwDisplayFrequency);
			printf("    Current Rotation: %ld-degrees\n" , displayInfo.dwDegreesRotation);
			printf("    Current Pannable: %ld x %ld pixels\n" , displayInfo.dwPelsWidth, displayInfo.dwPelsHeight);
			printf("    Current Rectangle: (%ld,%ld)-(%ld,%ld)\n", displayInfo.rcDisplayRect.left, displayInfo.rcDisplayRect.top, displayInfo.rcDisplayRect.right, displayInfo.rcDisplayRect.bottom);
		}
	}

	return;
}

