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
static DisplayDevice **g_deviceComboContents = NULL;
static BOOL g_bMultipleDevices = FALSE, g_bDevicesValid = FALSE;
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

void EnsureDevices()
{
  if (g_bDevicesValid) return;

  g_devices.Clear();
  g_devices.LoadFromRegistry();
  g_bMultipleDevices = ((NULL == g_devices.GetFirst()) ||
                        (NULL != g_devices.GetFirst()->GetName()));

  if (NULL != g_deviceComboContents)
    delete [] g_deviceComboContents;
  g_deviceComboContents = new DisplayDevice*[g_devices.Size() + 2];

  g_bDevicesValid = TRUE;
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

  DisplayActionDeviceType deviceType;  
  DisplayDevice *commandDevice;
  DisplayAction *commandAction;

  if ((val1 == duOnGetPage) || (val1 == duOnGetValues)) {
    EnsureDevices();

    if (!FindDisplayAction(g_devices, command, deviceType, commandDevice, commandAction))
      commandAction = DisplayActions;  // String
  }
  else {
    if ((val1 == duOnApply) ||
        ((val1 == duOnEvent) && (val2 == 0))) {
      int itemIndex = 0;
      double d;
      if (ControlGetNumber(L, "display", "ItemIndex", d))
        itemIndex = (int)d;
      commandDevice = g_deviceComboContents[itemIndex];
      if (itemIndex == 0)
        deviceType = devDEFAULT;
      else if (NULL == commandDevice)
        deviceType = devALL;
      else
        deviceType = devNAMED;
    }
    if ((val1 == duOnApply) ||
        ((val1 == duOnEvent) && FALSE)) {
      int itemIndex = 0;
      if ((editDisplay == item->ActionSubType) ||
          (editControl == item->ActionSubType)) {
        double d;
        if (ControlGetNumber(L, "type", "ItemIndex", d))
          itemIndex = (int)d;
      }
      for (int i = 0; i < countof(DisplayActions); i++) {
        if (DisplayActions[i].editorType == item->ActionSubType) {
          commandAction = DisplayActions+i;
          if (itemIndex-- <= 0)
            break;
        }
      }
    }
  }

  switch (val1) {
  case duOnGetPage:
    {
      if (command->ActionSubType != commandAction->editorType) {
        command->ActionSubType = commandAction->editorType;
      }

      if (pageNode != g_activePages[commandAction->editorType])
        newPage = g_activePages[commandAction->editorType]; // Switch to proper editor.
    }
    break;
				
  case duOnGetValues:
    {
      if (g_bMultipleDevices) {
        ControlSetString(L, "displayLabel", "Caption", "Display:");
        ControlSetBool(L, "displayLabel", "Visible", TRUE);

        char strings[2048];
        int itemIndex = 0, i = 0;
        strncpy(strings, "Default\n", sizeof(strings));
        g_deviceComboContents[i++] = g_devices.GetDefault();
        if (editControl == item->ActionSubType) {
          strncat(strings, "All\n", sizeof(strings));
          if (devALL == deviceType)
            itemIndex = i;
          g_deviceComboContents[i++] = NULL;
        }
        for (DisplayDevice *dev = g_devices.GetFirst(); NULL != dev;
             dev = dev->GetNext()) {
          switch (item->ActionSubType) {
          case editKeypad:
            if (!dev->HasKeypadLegends()) continue;
            break;
          case editGPO:
            if (!dev->HasGPOs()) continue;
            break;
          case editFan:
            if (!dev->HasFans()) continue;
            break;
          }
          strncat(strings, dev->GetName(), sizeof(strings));
          strncat(strings, "\n", sizeof(strings));
          if ((devNAMED == deviceType) && (commandDevice == dev))
            itemIndex = i;
          g_deviceComboContents[i++] = dev;
        }
        ControlSetString(L, "display", "Strings", strings);
        ControlSetNumber(L, "display", "ItemIndex", itemIndex);
        ControlSetBool(L, "display", "Visible", TRUE);
      }
      else {
        ControlSetBool(L, "displayLabel", "Visible", FALSE);
        ControlSetBool(L, "display", "Visible", FALSE);
      }

      if ((editDisplay == item->ActionSubType) ||
          (editControl == item->ActionSubType)) {
        char strings[2048];
        int itemIndex = 0, i = 0;
        for (int j = 0; j < countof(DisplayActions); j++) {
          if (DisplayActions[j].editorType == item->ActionSubType) {
            if (i == 0)
              strncpy(strings, DisplayActions[j].name, sizeof(strings));
            else
              strncat(strings, DisplayActions[j].name, sizeof(strings));
            strncat(strings, "\n", sizeof(strings));
            if (commandAction == DisplayActions+j)
              itemIndex = i;
            i++;
          }
        }
        ControlSetString(L, "type", "Strings", strings);
        ControlSetNumber(L, "type", "ItemIndex", itemIndex);
      }

      switch (item->ActionSubType) {
      case editDisplay:
        {
          ControlSetNumber(L, "row", "Max", 
                           (NULL == commandDevice) ? 0 : commandDevice->GetHeight() - 1);
          ControlSetString(L, "marquee", "Caption", "Marquee");
          int column = strtol(command->Action.iValue2, NULL, 10);
          BOOL marquee = (column < 0);
          ControlSetBool(L, "marquee", "Checked", marquee);
          ControlSetString(L, "columnCheck", "Caption", "Column:");
          ControlSetBool(L, "columnCheck", "Checked", !marquee);
          ControlSetBool(L, "column", "Enabled", !marquee);
          ControlSetNumber(L, "column", "Max", 
                           (NULL == commandDevice) ? 0 : commandDevice->GetWidth() - 1);
          if (!marquee)
            ControlSetNumber(L, "column", "Position", column);
          int width = strtol(command->Action.iValue3, NULL, 10);
          BOOL rest = (width < 0);
          ControlSetString(L, "rest", "Caption", "Rest of line");
          ControlSetBool(L, "rest", "Checked", rest);
          ControlSetString(L, "widthCheck", "Caption", "Width:");
          ControlSetBool(L, "widthCheck", "Checked", !rest);
          ControlSetBool(L, "width", "Enabled", !rest);
          ControlSetNumber(L, "width", "Max", 
                           (NULL == commandDevice) ? 0 : commandDevice->GetWidth());
          if (!rest)
            ControlSetNumber(L, "width", "Position", width);
        }
        break;
      case editScreen:
        {
          // TODO: ...
        }
        break;
      case editKeypad:
        {
          // TODO: fill combos.
          ControlSetString(L, "key", "Text", command->Action.sValue1); 
          ControlSetString(L, "legend", "Text", command->Action.sValue3);
        }
        break;
      }
    }
    break;
				
  case duOnApply:
    {
      if (devDEFAULT == deviceType)
        command->Action.sValue2 = GStrDup(commandAction->key);
      else {
        char key[1024];
        strncpy(key, 
                (devALL == deviceType) ? "*" : commandDevice->GetName(),
                sizeof(key));
        strncat(key, ":", sizeof(key));
        strncat(key, commandAction->key, sizeof(key));
        command->Action.sValue2 = GStrDup(key);
      }

      switch (item->ActionSubType) {
      case editDisplay:
        {
          BOOL marquee;
          if (ControlGetBool(L, "marquee", "Checked", marquee)) {
            if (marquee)
              command->Action.iValue2 = GStrDup("-1");
            else {
              const char *str;
              if (ControlGetString(L, "column", "Position", str))
                command->Action.iValue2 = GStrDup(str);
            }
          }
          BOOL rest;
          if (ControlGetBool(L, "rest", "Checked", rest)) {
            if (rest)
              command->Action.iValue3 = GStrDup("-1");
            else {
              const char *str;
              if (ControlGetString(L, "width", "Position", str))
                command->Action.iValue3 = GStrDup(str);
            }
          }
        }
        break;
      case editScreen:
        {
          // TODO: ...
        }
        break;
      case editKeypad:
        {
          const char *str;
          if (ControlGetString(L, "key", "Text", str))
            command->Action.sValue1 = GStrDup(str);
          if (ControlGetString(L, "legend", "Text", str))
            command->Action.sValue3 = GStrDup(str);
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
        case 0:
          {
            ControlSetNumber(L, "row", "Max", 
                             (NULL == commandDevice) ? 0 : commandDevice->GetHeight() - 1);
            ControlSetNumber(L, "column", "Max", 
                             (NULL == commandDevice) ? 0 : commandDevice->GetWidth() - 1);
            ControlSetNumber(L, "width", "Max", 
                             (NULL == commandDevice) ? 0 : commandDevice->GetWidth());
          }
          break;
        case 1:
          {
            BOOL marquee;
            if (ControlGetBool(L, "marquee", "Checked", marquee)) {
              ControlSetBool(L, "columnCheck", "Checked", !marquee);
              ControlSetBool(L, "column", "Enabled", !marquee);
            }
          }
          break;
        case 2:
          {
            BOOL notMarquee;
            if (ControlGetBool(L, "columnCheck", "Checked", notMarquee)) {
              ControlSetBool(L, "marquee", "Checked", !notMarquee);
              ControlSetBool(L, "column", "Enabled", notMarquee);
            }
          }
          break;
        case 3:
          {
            BOOL rest;
            if (ControlGetBool(L, "rest", "Checked", rest)) {
              ControlSetBool(L, "widthCheck", "Checked", !rest);
              ControlSetBool(L, "width", "Enabled", !rest);
            }
          }
          break;
        case 4:
          {
            BOOL notRest;
            if (ControlGetBool(L, "widthCheck", "Checked", notRest)) {
              ControlSetBool(L, "rest", "Checked", !notRest);
              ControlSetBool(L, "width", "Enabled", notRest);
            }
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

  g_devices.Clear();
  if (NULL != g_deviceComboContents) {
    delete [] g_deviceComboContents;
    g_deviceComboContents = NULL;
  }
  g_bDevicesValid = FALSE;
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

void DUIDevicesChanged()
{
  g_bDevicesValid = FALSE;
}
