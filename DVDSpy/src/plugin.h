#define PLUGINNUM 215
#define PLUGINNAME "DVDSpy"
#define PLUGINVERSION "1.25"

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

extern void GirderEvent(PCHAR event, PCHAR payload = NULL, size_t pllen = 0);
extern void GetDVDTitle(LPCSTR disc, LPSTR title, size_t tsize, LPDWORD volser);

extern void ZoomPlayerInit(LPCSTR data, HWND hMonitorWindow);
extern void ZoomPlayerLCD(WPARAM wParam, LPARAM lParam);
