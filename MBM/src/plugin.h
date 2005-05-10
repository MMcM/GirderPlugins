/* Global interfaces 
$Header$
*/

#define PLUGINNUM 46
#define PLUGINNAME "Motherboard Monitor"
#define PLUGINVERSION "2.0"

extern HINSTANCE g_hInstance;
extern sFunctions3 SF;

const size_t NSENSORS = 100;
extern BOOL OpenSharedMemory();
extern void CloseSharedMemory();
extern BOOL StartMonitor();
extern void StopMonitor();
extern LPCSTR GetEventName(size_t i);

extern BOOL DUIOpen();
extern void DUIClose();
extern void DUIOpenConfig(PFTree tree);
extern void DUICloseConfig(PFTree tree);
extern void DUIOpenCommand(PFTree tree);
extern void DUICloseCommand(PFTree tree);
