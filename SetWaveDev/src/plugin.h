/* Global interfaces 
$Header$
*/

#define PLUGINNUM 123
#define PLUGINNAME "SetWaveDev"
#define PLUGINVERSION "1.5"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern void OpenCommandUI();
extern void CloseCommandUI();
extern void UpdateCommandUI(p_command command);
