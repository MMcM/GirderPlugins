/* Global interfaces */

#define PLUGINNUM 118
#define PLUGINNAME "LCD"
#define PLUGINVERSION "1.14"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern void OpenCommandUI();
extern void CloseCommandUI();
extern void UpdateCommandUI(p_command command);

extern int nDisplayCols, nDisplayRows;
extern LPSTR pDisplayBuf;

class DisplayCommandState;

enum DisplayValueType {
  valNONE, valSTR, valINT, valVAR, valSCREEN
};

struct DisplayAction
{
  const char *key;
  const char *name;
  DisplayValueType valueType;
  void (*function)(DisplayCommandState& state);
};

extern DisplayAction *FindDisplayAction(p_command command);

extern void DisplayString(DisplayCommandState& state);
extern void DisplayVariable(DisplayCommandState& state);
extern void DisplayCurrentTime(DisplayCommandState& state);
extern void DisplayFilename(DisplayCommandState& state);
extern void DisplayClose(DisplayCommandState& state);
extern void DisplayClear(DisplayCommandState& state);
extern void DisplayScreen(DisplayCommandState& state);

extern void DisplayCommand(p_command command,
                           PCHAR status, int statuslen);
extern void DisplayClose();
