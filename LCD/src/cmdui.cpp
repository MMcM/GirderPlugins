/* Command (action settings) user interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "display.h"

#include <dui.h>

static sGroups GROUP = 
  // {6F829806-2544-40e9-B150-A7655F93C906}
  { { 0x6f829806, 0x2544, 0x40e9, { 0xb1, 0x50, 0xa7, 0x65, 0x5f, 0x93, 0xc9, 0x6 } },
    "LCD" };

#define LCD_DUI_GUID  "{6F829807-2544-40e9-B150-A7655F93C906}"
#define DISPLAY_PAGE_GUID  "{6F829809-2544-40e9-B150-A7655F93C906}"
#define CONTROL_PAGE_GUID  "{6F82980A-2544-40e9-B150-A7655F93C906}"
#define SCREEN_PAGE_GUID  "{6F82980B-2544-40e9-B150-A7655F93C906}"
#define KEYPAD_PAGE_GUID  "{6F82980C-2544-40e9-B150-A7655F93C906}"
#define GPO_PAGE_GUID  "{6F82980D-2544-40e9-B150-A7655F93C906}"
#define FAN_PAGE_GUID  "{6F82980E-2544-40e9-B150-A7655F93C906}"

static DisplayDeviceList g_devices;
static BOOL g_bMultipleDevices = FALSE;
PFTree g_DUI;
PFTreeNode g_DisplayPage, g_ControlPage, g_ScreenPage, 
  g_KeypadPage, g_GPOPage, g_FanPage;
PFTreeNode g_DisplayPageActive, g_ControlPageActive, g_ScreenPageActive, 
  g_KeypadPageActive, g_GPOPageActive, g_FanPageActive;

enum {
  valNONE, valSTR, valINT, valBOOL, valVAR, valLIST, valSTR2, valLIST2
};

enum {
  editDisplay = 1, editControl, editScreen, editKeypad, editGPO, editFan
};

DisplayAction DisplayActions[] = {
  { "s", "String", valSTR, editDisplay, DisplayString },
  { "v", "Variable", valVAR, editDisplay, DisplayVariable },
  { "t", "Current Date/Time", valSTR, editDisplay, DisplayCurrentTime },
  { "f", "Filename Variable", valVAR, editDisplay, DisplayFilename },
  { "*", "Character (numerical code)", valSTR, editDisplay, DisplayCharacter },
  { "$", "Custom Character", valSTR, editDisplay, DisplayCustomCharacter },

  { "x", "Close Display", valNONE, editControl, DisplayClose },
  { "c", "Clear Display", valNONE, editControl, DisplayClear },

  { "#", "Screen", valNONE, editScreen, DisplayScreen },

  { "k", "Keypad Legend", valLIST2, editKeypad, DisplayKeypadLegend },

  { "o", "General Purpose Output", valBOOL, editGPO, DisplayGPO },

  { "p", "Fan Power", valSTR, editFan, DisplayFanPower },
};

BOOL FindDisplayAction(DisplayDeviceList& devices, PCommand command,
                       DisplayActionDeviceType& devtype, DisplayDevice*& device,
                       DisplayAction*& action)
{
  devtype = devDEFAULT;
  device = devices.GetDefault();

  PCHAR key = command->Action.sValue2;

#if 0
  if ((NULL == key) || ('\0' == *key)) {
    // An older GML file.  Convert the command to the new format.
    PCHAR val = NULL;
    char buf[128];

    switch (strtol(command->Action.iValue1, NULL, 10)) {
    case 0:                       // String
      key = "s";
      break;
    case 1:                       // String Register
      key = "v";
      sprintf(buf, "treg%s", command->Action.iValue2);
      val = buf;
      break;
    case 2:                       // Current Date/Time
      key = "t";
      break;
    case 3:                       // Clear Display
      key = "c";
      break;
    case 4:                       // Close Display
      key = "x";
      break;
    case 5:                       // Payload
      key = "v";
      sprintf(buf, "pld%s", command->Action.iValue2);
      val = buf;
      break;
    case 6:                       // Filename Payload
      key = "f";
      sprintf(buf, "pld%s", command->Action.iValue2);
      val = buf;
      break;
    default:
      return FALSE;
    }
    if (NULL != val) {
      SafeFree(command->Action.sValue1);
      command->Action.sValue1 = GStrDup(val);
    }
    SafeFree(command->Action.sValue2);
    command->Action.sValue2 = GStrDup(key);
    // Position information.
    SafeFree(command->Action.iValue1);
    sprintf(buf, "%d", command->lvalue1); // Row
    command->Action.iValue1 = GStrDup(buf);
    SafeFree(command->Action.iValue2);
    sprintf(buf, "%d", command->lvalue2); // Column
    command->Action.iValue2 = GStrDup(buf);
    SafeFree(command->Action.iValue3);
    sprintf(buf, "%d", (command->lvalue3 <= 0) ? -1 : command->lvalue3); // Width
    command->Action.iValue3 = GStrDup(buf);
    command->lvalue1 = 0;       // No links.
    command->lvalue2 = 0;
    command->lvalue3 = 0;
  }
#endif

  LPSTR cpos = strchr(key, ':');
  if (NULL != cpos) {
    char name[128];
    size_t nlen = cpos - key;
    if (nlen < sizeof(name)) {
      memcpy(name, key, nlen);
      name[nlen] = '\0';
      if (!strcmp(name, "*")) {
        devtype = devALL;
        device = devices.GetFirst();
      }
      else {
        device = devices.Get(name);
        if (NULL != device)
          devtype = devNAMED;
        else
          devtype = devUNKNOWN;
      }
    }
    key = cpos + 1;
  }

  for (size_t i = 0; i < countof(DisplayActions); i++) {
    if (!strcmp(DisplayActions[i].key, key)) {
      action = DisplayActions + i;
      return TRUE;
    }
  }
  action = NULL;
  return FALSE;
}

void * WINAPI
DisplayPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                    int val1, int val2, void *userdata)
{
  PCommand command;
  if (ntCommand == baseNode->NodeType) {
    command = (PCommand)baseNode;
  }
  else {
    return NULL;
  }

  switch (val1) {
  case duOnGetValues:
    {
      EnterCriticalSection(&lua->CS);
  
      DisplayActionDeviceType deviceType;  
      DisplayDevice *commandDevice;
      DisplayAction *action;
      if (!FindDisplayAction(g_devices, command, deviceType, commandDevice, action))
        action = DisplayActions;    // String

      if (command->ActionSubType != action->editorType) {
        LeaveCriticalSection(&lua->CS);
        return g_ControlPageActive;
      }

      LeaveCriticalSection(&lua->CS);
    }
    break;
				
  case duOnApply:
    break;

  case duOnEvent:
    break;
  }

  return NULL;
}

void * WINAPI
ControlPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                   int val1, int val2, void *userdata)
{
  return NULL;
}

void * WINAPI
ScreenPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                   int val1, int val2, void *userdata)
{
  return NULL;
}

void * WINAPI
KeypadPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                   int val1, int val2, void *userdata)
{
  return NULL;
}

void * WINAPI
GPOPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                int val1, int val2, void *userdata)
{
  return NULL;
}

void * WINAPI
FanPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                int val1, int val2, void *userdata)
{
  return NULL;
}

BOOL DUIOpen()
{
  char path[MAX_PATH];
  strcpy(path, SF.CoreVars->ExePath);
  strcat(path, "plugins\\UI\\LCD.xml");
  g_DUI = LoadDUI(path);
  if (NULL == g_DUI) {
    GirderLogMessageEx(PLUGINNAME, "Could not open DUI File (LCD.xml).",
                       GLM_ERROR_ICON);
    return FALSE;
  }
  
  g_DisplayPage = FindNodeS(DISPLAY_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_DisplayPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the Display Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_DisplayPage, DisplayPageCallback, TRUE);

  g_ControlPage = FindNodeS(CONTROL_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_ControlPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the Control Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_ControlPage, ControlPageCallback, TRUE);

  g_ScreenPage = FindNodeS(SCREEN_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_ScreenPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the Screen Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_ScreenPage, ScreenPageCallback, TRUE);

  g_KeypadPage = FindNodeS(KEYPAD_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_KeypadPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the Keypad Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_KeypadPage, KeypadPageCallback, TRUE);

  g_GPOPage = FindNodeS(GPO_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_GPOPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the GPO Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_GPOPage, GPOPageCallback, TRUE);

  g_FanPage = FindNodeS(FAN_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_FanPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the Fan Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_FanPage, FanPageCallback, TRUE);

  return TRUE;
}

void DUIClose()
{
  DeleteFTree(g_DUI);
  g_DUI = NULL;
  g_DisplayPage =  g_ControlPage = g_ScreenPage = 
    g_KeypadPage = g_GPOPage = g_FanPage = NULL;
}

void DUIOpenCommand(PFTree tree)
{
  g_DisplayPageActive = InsertDUIPage(tree, g_DUI, g_DisplayPage,
                                      &GROUP.PageGUID, &GROUP);
  g_ControlPageActive = InsertDUIPage(tree, g_DUI, g_ControlPage,
                                      &GROUP.PageGUID, &GROUP);
  g_ScreenPageActive = InsertDUIPage(tree, g_DUI, g_ScreenPage,
                                     &GROUP.PageGUID, &GROUP);
  g_KeypadPageActive = InsertDUIPage(tree, g_DUI, g_KeypadPage,
                                     &GROUP.PageGUID, &GROUP);
  g_GPOPageActive = InsertDUIPage(tree, g_DUI, g_GPOPage,
                                  &GROUP.PageGUID, &GROUP);
  g_FanPageActive = InsertDUIPage(tree, g_DUI, g_FanPage,
                                  &GROUP.PageGUID, &GROUP);
}

void DUICloseCommand(PFTree tree)
{
  RemoveDUIPageS(tree, DISPLAY_PAGE_GUID);
  g_DisplayPageActive = NULL;
  RemoveDUIPageS(tree, CONTROL_PAGE_GUID);
  g_ControlPageActive = NULL;
  RemoveDUIPageS(tree, SCREEN_PAGE_GUID);
  g_ScreenPageActive = NULL;
  RemoveDUIPageS(tree, KEYPAD_PAGE_GUID);
  g_KeypadPageActive = NULL;
  RemoveDUIPageS(tree, GPO_PAGE_GUID);
  g_GPOPageActive = NULL;
  RemoveDUIPageS(tree, FAN_PAGE_GUID);
  g_FanPageActive = NULL;
}
