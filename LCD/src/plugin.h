/* Global interfaces */

#define PLUGINNUM 118
#define PLUGINNAME "LCD"
#define PLUGINVERSION "2.4"

extern HINSTANCE g_hInstance;
extern s_functions SF;

extern void OpenConfigUI();
extern void CloseConfigUI();
extern void OpenCommandUI();
extern void CloseCommandUI();
extern void UpdateCommandUI(p_command command);

class DisplayDevice;

extern int DisplayWidth();
extern int DisplayHeight();
extern int DisplayGPOs();
extern void DisplayClose();
extern void DisplayReopen(DisplayDevice *device);
extern BOOL DisplayEnableInput();
extern void DisplayDisableInput();
extern void DisplaySendEvent(BYTE b);

class DisplayCommandState;

enum DisplayValueType {
  valNONE, valSTR, valINT, valBOOL, valVAR
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
extern void DisplayCharacter(DisplayCommandState& state);
extern void DisplayCustomCharacter(DisplayCommandState& state);
extern void DisplayGPO(DisplayCommandState& state);

extern void DisplayCommand(p_command command,
                           PCHAR status, int statuslen);
