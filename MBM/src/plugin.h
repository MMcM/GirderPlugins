/* Global interfaces */

#define PLUGINNUM 46
#define PLUGINNAME "Motherboard Monitor"
#define PLUGINVERSION "1.1"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern void OpenLearnUI(PCHAR old);
extern void CloseLearnUI();

const size_t NSENSORS = 100;
extern BOOL OpenSharedMemory();
extern void CloseSharedMemory();
extern BOOL StartMonitor();
extern void StopMonitor();
extern LPCSTR GetEventName(size_t i);
