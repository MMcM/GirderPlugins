/* Global interfaces 
$Header$
*/

#define PLUGINNUM 45
#define PLUGINNAME "ID3"
#define PLUGINVERSION "1.1"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern void OpenCommandUI();
extern void CloseCommandUI();
extern void UpdateCommandUI(p_command command);
