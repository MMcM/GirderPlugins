/* Global interfaces 
$Header$
*/

#define PLUGINNUM 49
#define PLUGINNAME "Winamp3"
#define PLUGINVERSION "1.3"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern BOOL StartMonitor(BOOL events);
extern void StopMonitor(BOOL events);
extern void SendCommand(p_command command, char *status, int statuslen);

extern void OpenCommandUI();
extern void CloseCommandUI();
extern void UpdateCommandUI(p_command command);
extern void OpenLearnUI(PCHAR old);
extern void CloseLearnUI();
