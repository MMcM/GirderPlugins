/* Global interfaces */

#define PLUGINNUM 215
#define PLUGINNAME "DVDSpy"
#define PLUGINVERSION "1.34"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern void OpenConfigUI();
extern void CloseConfigUI();
extern void OpenLearnUI(PCHAR old);
extern void CloseLearnUI();

extern BOOL StartMonitor();
extern void StopMonitor();

extern void GirderEvent(PCHAR event, PCHAR payload = NULL, size_t pllen = 0);
extern void GetDVDTitle(LPCSTR disc, LPSTR title, size_t tsize, LPDWORD volser);
extern BOOL GetDVDVideoAttribs(LPCSTR disc, 
                               LPCSTR *aspect, LPCSTR *display, LPCSTR *standard);

extern void ZoomPlayerInit(LPCSTR data, HWND hMonitorWindow);
extern void ZoomPlayerLCD(WPARAM wParam, LPARAM lParam);
