/* Global interfaces */

#define PLUGINNUM 44
#define PLUGINNAME "PowerMate"
#define PLUGINVERSION "1.0"

extern HINSTANCE g_hInstance;
extern s_functions SF;

enum ActionType_t { ACTION_REQUEST = 1, ACTION_REPEAT };
enum RequestType_t { LED_BRIGHTNESS, 
                     PULSE_SLEEP, PULSE_ALWAYS, PULSE_SPEED };

extern BOOL DeviceStart();
extern BOOL DeviceStop();
extern BOOL DeviceRequest(RequestType_t nReq, USHORT nVal);

extern void OpenCommandUI();
extern void CloseCommandUI();
extern void UpdateCommandUI(p_command command);
