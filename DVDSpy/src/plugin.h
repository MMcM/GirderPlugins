/* Global interfaces 
$Header$
*/

#define PLUGINNUM 215
#define PLUGINNAME "DVDSpy"
#define PLUGINVERSION "1.59"

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
extern BOOL GetDVDAttribs(LPCSTR disc, 
                          LPCSTR& aspect, LPCSTR& display, LPCSTR& standard,
                          LPSTR regions);
extern void GetDVDDiscID(LPCSTR disc, ULONGLONG *discid);

extern void ZoomPlayerInit(LPCSTR data, HWND hMonitorWindow);
extern void ZoomPlayerLCD(WPARAM wParam, LPARAM lParam);
extern void EugenesInit(LPCSTR data, HWND hwndMonitor);
extern void EugenesClose();
