
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <stdarg.h>
#include "NvCpl.h"

void printt(const char *fmt, ...);

/*******************************************************************************

    Main

*******************************************************************************/

static void InitInfo(HINSTANCE);
static void DesktopConfiguration(HINSTANCE);
static void GPUConfiguration(HINSTANCE);
//ugly global bool
bool gSupress = false;

int main(int argc, char* argv[])
{
    if (argc > 1) {
		if (!strcmp(argv[1], "-h")) {
			printt("\nnvcpltest version 1.0, Sundanse\n");
			printt("nvcpltest <-d> <-h>\n");
			printt("-d = just print active display number\n");
			printt("-h = this help template\n");
			return 0;
		}
		if (!strcmp(argv[1], "-d")) {
			gSupress = true;
		}
	}
    // Load NVCPL library
    HINSTANCE hLib = ::LoadLibrary("NVCPL.dll");
    if (hLib == 0) {
        printt("Unable to load NVCPL.dll\n");
        return -1;
    }
    // MMW: It's possible to load nvcpl even when current the graphics 
    // card is not Nvidia: therefore also need to check for Nvidia 
    // card to make sure the results of these queries reflect reality.

	//InitInfo(hLib);

	// Query desktop settings
    DesktopConfiguration(hLib);

    // Free NVCPL library
    ::FreeLibrary(hLib);

	return 0;
}

/*******************************************************************************

    printf replacement	

*******************************************************************************/
void printt(const char *fmt, ...)
{
	if (gSupress == true)
		return;
	int size = 0;
	const int BufSz = 4096;
	char FinalBuffer[BufSz] = {0};
	va_list ap;

	va_start( ap, fmt );
	size = vsprintf( FinalBuffer, fmt, ap );
	va_end( ap );
	if (size > BufSz)
		FinalBuffer[BufSz -1] = 0;
  
	printf("%s", FinalBuffer);
}
/*******************************************************************************

    Initial Info Display

*******************************************************************************/

static void InitInfo(HINSTANCE hLib)
{	
	// Get the NvGetDisplayInfo function pointer from the library
	NVDISPLAYINFO displayInfo = {0};
    fNvGetDisplayInfo pfNvGetDisplayInfo = (fNvGetDisplayInfo)::GetProcAddress(hLib, "NvGetDisplayInfo");
    if (pfNvGetDisplayInfo == NULL)
        printt("Unable to get a pointer to NvGetDisplayInfo\n");
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

static void DesktopConfiguration(HINSTANCE hLib)
{
    printt("\nDESKTOP CONFIGURATION:\n");

    // Get the NvCplGetRealConnectedDevicesString function pointer from the library
    fNvCplGetRealConnectedDevicesString pfNvCplGetRealConnectedDevicesString = (fNvCplGetRealConnectedDevicesString)::GetProcAddress(hLib, "NvCplGetRealConnectedDevicesString");
    if (pfNvCplGetRealConnectedDevicesString == NULL)
        printt("- Unable to get a pointer to NvCplGetRealConnectedDevicesString\n");
    else {
        char connectedDevices[1024];
        pfNvCplGetRealConnectedDevicesString(connectedDevices, 1024, FALSE);
        printt("- Connected devices: %s\n", connectedDevices);
        pfNvCplGetRealConnectedDevicesString(connectedDevices, 1024, TRUE);
        printt("- Active connected devices: ", connectedDevices);
        printf("%s\n", connectedDevices);
    }

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