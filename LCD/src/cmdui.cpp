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

static PCHAR PAGE_GUIDS[] = {
  NULL,
  "{6F829809-2544-40e9-B150-A7655F93C906}", // Display
  "{6F82980A-2544-40e9-B150-A7655F93C906}", // Control
  "{6F82980B-2544-40e9-B150-A7655F93C906}", // Screen
  "{6F82980C-2544-40e9-B150-A7655F93C906}", // Keypad
  "{6F82980D-2544-40e9-B150-A7655F93C906}", // GPO
  "{6F82980E-2544-40e9-B150-A7655F93C906}", // Fan
};

static DisplayDeviceList g_devices;
static BOOL g_bMultipleDevices = FALSE;
PFTree g_DUI;
PFTreeNode g_pages[countof(PAGE_GUIDS)], g_activePages[countof(PAGE_GUIDS)];

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
PageCallback(pLuaRec lua, PFTree tree, PFTreeNode pageNode, PBaseNode baseNode,
             int val1, int val2, void *userdata)
{
  if (ntCommand != baseNode->NodeType)
    return NULL;
  PCommand command = (PCommand)baseNode;

  PDUIControl control = NULL;
  PDUIItem item = (PDUIItem)pageNode->Data;
  if (duControl == item->NodeType) {
    control = (PDUIControl)item;
    pageNode = pageNode->Parent;
    item = (PDUIItem)pageNode->Data;
  }
  if (duItem != item->NodeType)
    return NULL;

  lua_State *L = (lua_State *)lua->L;

  PFTreeNode newPage = NULL;

  EnterCriticalSection(&lua->CS);

  switch (val1) {
  case duOnGetPage:
    {
      DisplayActionDeviceType deviceType;  
      DisplayDevice *commandDevice;
      DisplayAction *action;
      if (!FindDisplayAction(g_devices, command, deviceType, commandDevice, action))
        action = DisplayActions;    // String

      if (command->ActionSubType != action->editorType) {
        command->ActionSubType = action->editorType;
      }

      if (pageNode != g_activePages[action->editorType])
        newPage = g_activePages[action->editorType]; // Switch to proper editor.
    }
    break;
				
  case duOnGetValues:
    {
      ControlSetString(L, "displayLabel", "Caption", "Display:");

      switch (item->ActionSubType) {
      case editDisplay:
        {
          ControlSetString(L, "marquee", "Caption", "Marquee");
          int column = strtol(command->Action.iValue2, NULL, 10);
          BOOL marquee = (column < 0);
          ControlSetBool(L, "marquee", "Checked", marquee);
          ControlSetString(L, "columnCheck", "Caption", "Column:");
          ControlSetBool(L, "columnCheck", "Checked", !marquee);
          ControlSetBool(L, "column", "Enabled", !marquee);
          ControlSetNumber(L, "column", "Max", 2);
          if (!marquee)
            ControlSetNumber(L, "column", "Position", column);
          int width = strtol(command->Action.iValue3, NULL, 10);
          BOOL rest = (width < 0);
          ControlSetString(L, "rest", "Caption", "Rest of line");
          ControlSetBool(L, "rest", "Checked", rest);
          ControlSetString(L, "widthCheck", "Caption", "Width:");
          ControlSetBool(L, "widthCheck", "Checked", !rest);
          ControlSetBool(L, "width", "Enabled", !rest);
          ControlSetNumber(L, "width", "Max", 10);
          if (!rest)
            ControlSetNumber(L, "width", "Position", width);
        }
        break;
      }
    }
    break;
				
  case duOnApply:
    {
      switch (item->ActionSubType) {
      case editDisplay:
        {
          BOOL marquee;
          ControlGetBool(L, "marquee", "Checked", marquee);
          if (marquee)
            command->Action.iValue2 = GStrDup("-1");
          else
            ControlGetString(L, "column", "Position", command->Action.iValue2);
          
          BOOL rest;
          ControlGetBool(L, "rest", "Checked", rest);
          if (rest)
            command->Action.iValue3 = GStrDup("-1");
          else
            ControlGetString(L, "width", "Position", command->Action.iValue3);
        }
        break;
      }
    }
    break;

  case duOnEvent:
    {
      switch (item->ActionSubType) {
      case editDisplay:
        switch (val2) {
        case 1:
          {
            BOOL marquee;
            ControlGetBool(L, "marquee", "Checked", marquee);
            ControlSetBool(L, "columnCheck", "Checked", !marquee);
            ControlSetBool(L, "column", "Enabled", !marquee);
          }
          break;
        case 2:
          {
            BOOL notMarquee;
            ControlGetBool(L, "columnCheck", "Checked", notMarquee);
            ControlSetBool(L, "marquee", "Checked", !notMarquee);
            ControlSetBool(L, "column", "Enabled", notMarquee);
          }
          break;
        case 3:
          {
            BOOL rest;
            ControlGetBool(L, "rest", "Checked", rest);
            ControlSetBool(L, "widthCheck", "Checked", !rest);
            ControlSetBool(L, "width", "Enabled", !rest);
          }
          break;
        case 4:
          {
            BOOL notRest;
            ControlGetBool(L, "widthCheck", "Checked", notRest);
            ControlSetBool(L, "rest", "Checked", !notRest);
            ControlSetBool(L, "width", "Enabled", notRest);
          }
          break;
        }
        break;
      }
    }
    break;
  }

  LeaveCriticalSection(&lua->CS);

  return newPage;
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
  
  for (int i = 1; i < countof(PAGE_GUIDS); i++) {
    g_pages[i] = FindNodeS(PAGE_GUIDS[i], g_DUI, NULL);
    if (NULL == g_pages[i]) {	
      GirderLogMessageEx(PLUGINNAME, "Could not find page in LCD.xml.",
                         GLM_ERROR_ICON);
      return FALSE;
    }
    SetCallbacks(g_DUI, g_pages[i], PageCallback, TRUE);
  }

  return TRUE;
}

void DUIClose()
{
  DeleteFTree(g_DUI);
  g_DUI = NULL;
  for (int i = 1; i < countof(PAGE_GUIDS); i++) {
    g_pages[i] = NULL;
  }
}

void DUIOpenCommand(PFTree tree)
{
  for (int i = 1; i < countof(PAGE_GUIDS); i++) {
    g_activePages[i] = InsertDUIPage(tree, g_DUI, g_pages[i],
                                     &GROUP.PageGUID, &GROUP);
  }
}

void DUICloseCommand(PFTree tree)
{
  for (int i = 1; i < countof(PAGE_GUIDS); i++) {
    RemoveDUIPageS(tree, PAGE_GUIDS[i]);
    g_activePages[i] = NULL;
  }
}
