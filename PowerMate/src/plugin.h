/* Global interfaces 
$Header$
*/

#define PLUGINNUM 44
#define PLUGINNAME "PowerMate"
#define PLUGINVERSION "3.1"

extern HINSTANCE g_hInstance;
extern sFunctions3 SF;

enum ActionType_t { ACTION_REQUEST = 1, ACTION_REPEAT };
enum RequestType_t { LED_BRIGHTNESS, 
                     PULSE_SLEEP, PULSE_ALWAYS, PULSE_SPEED };

extern BOOL DeviceStart();
extern BOOL DeviceStop();
extern BOOL DeviceRequest(RequestType_t nReq, USHORT nVal);

extern BOOL DUIOpen();
extern void DUIClose();
extern void DUIOpenCommand(PFTree tree);
extern void DUICloseCommand(PFTree tree);
extern void DUIOpenConfig(PFTree tree);
extern void DUICloseConfig(PFTree tree);
