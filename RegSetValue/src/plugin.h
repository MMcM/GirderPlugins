/* Global interfaces 
$Header$
*/

#define PLUGINNUM 117
#define PLUGINNAME "RegSetValue"
#define PLUGINVERSION "1.7"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern void OpenCommandUI();
extern void CloseCommandUI();
extern void UpdateCommandUI(p_command command);

extern PCHAR DecodeKeyValue(PCHAR value, PHKEY phkey);
