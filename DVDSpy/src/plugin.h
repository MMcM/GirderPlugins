#define PLUGINNUM 215
#define PLUGINNAME "DVDSpy"
#define PLUGINVERSION "1.32"

#include <girder.h>

#ifdef GIRDER_CPP

HINSTANCE hInstance;

t_functions  SF;

#else

extern HINSTANCE hInstance;

extern t_functions  SF;

#endif

#ifdef UI_CPP

HWND hConfigDialog;
HWND hLearnDialog;

#else

extern HWND hConfigDialog;
extern HWND hLearnDialog;

#endif

#define VTSTT_VOBS_ATTRIB_OFFSET        0x200

#define VIDEO_ATTRIB_STANDARD_PAL       0x10

#define VIDEO_ATTRIB_ASPECT_MASK        0x0c
#define VIDEO_ATTRIB_ASPECT_4X3         0x00
#define VIDEO_ATTRIB_ASPECT_16X9        0x0c

#define VIDEO_ATTRIB_DISPLAY_PANSCAN    0x02
#define VIDEO_ATTRIB_DISPLAY_LETTERBOX  0x01

extern void GirderEvent(PCHAR event, PCHAR payload = NULL, size_t pllen = 0);
extern void GetDVDTitle(LPCSTR disc, LPSTR title, size_t tsize, LPDWORD volser);
extern BOOL GetDVDVideoAttribs(LPCSTR disc, 
                               LPCSTR *aspect, LPCSTR *display, LPCSTR *standard);

extern void ZoomPlayerInit(LPCSTR data, HWND hMonitorWindow);
extern void ZoomPlayerLCD(WPARAM wParam, LPARAM lParam);
