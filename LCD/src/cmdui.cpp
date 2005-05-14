/* Command (action settings) user interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "display.h"

#define LCD_GROUP_NAME "LCD"
#define LCD_GROUP_GUID "{6F829806-2544-40e9-B150-A7655F93C906}"
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

#define SCREEN_NLINES 4

BOOL FindDisplayAction(DisplayDeviceList& devices, PCommand command,
                       DisplayActionDeviceType& devtype, DisplayDevice*& device,
                       DisplayAction*& action)
{
  devtype = devDEFAULT;
  device = devices.GetDefault();

  PCHAR key = command->Action.sValue2;
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

  if (NULL != g_deviceComboContents) {
    delete [] g_deviceComboContents;
    g_deviceComboContents = NULL;
  }
  if (g_bMultipleDevices)
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

  if (duOnGetDefaults == val1) {
    // Default device and action.
    EnsureDevices();
    deviceType = devDEFAULT;
    commandDevice = g_devices.GetDefault();
    
    for (int i = 0; i < countof(DisplayActions); i++) {
      if (DisplayActions[i].editorType == item->ActionSubType) {
        commandAction = DisplayActions+i;
        break;
      }
    }
  }
  else if ((duOnGetPage == val1) || (duOnGetValues == val1)) {
    // Decode device and action from command.
    EnsureDevices();
    if (!FindDisplayAction(g_devices, command, deviceType, commandDevice, commandAction))
      commandAction = DisplayActions;  // String
  }
  else {
    // Decode device and action from controls.
    if ((duOnApply == val1) ||
        ((duOnEvent == val1) && (0 == val2))) {
      if (g_bMultipleDevices) {
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
      else {
        deviceType = devDEFAULT;
        commandDevice = g_devices.GetDefault();
      }
    }
    if ((duOnApply == val1) ||
        ((duOnEvent == val1) && FALSE)) {
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

  if ((duOnGetDefaults == val1) ||
      (duOnGetValues == val1) ||
      ((duOnEvent == val1) && (0 == val2))) {
    // Handle device ranges and choices.
    switch (item->ActionSubType) {
    case editDisplay:
      ControlSetNumber(L, "row", "Max", 
                       (NULL == commandDevice) ? 0 : commandDevice->GetHeight() - 1);
      ControlSetNumber(L, "column", "Max", 
                       (NULL == commandDevice) ? 0 : commandDevice->GetWidth() - 1);
      ControlSetNumber(L, "width", "Max", 
                       (NULL == commandDevice) ? 0 : commandDevice->GetWidth());
      break;
    case editScreen:
      {
        int nrows = 0;
        if (NULL != commandDevice)
          nrows = commandDevice->GetHeight();
        char buf[18];
        const char *cnames[] = { "enabled", "line", "marquee" };
        for (int i = 0; i < SCREEN_NLINES; i++) {
          BOOL visible = (i < nrows);
          for (int j = 0; j < countof(cnames); j++) {
            _snprintf(buf, sizeof(buf), "%s%d", cnames[j], i+1);
            ControlSetBool(L, buf, "Visible", visible);
          }
        }    
      }
      break;
    case editKeypad:
      {
        // TODO: fill combos.
        char strings[2048];
        strings[0] = '\0';
        LPCSTR *choices = commandDevice->GetKeypadButtonChoices();
        if (NULL != choices) {
          while (NULL != *choices) {
            strncat(strings, *choices++, sizeof(strings));
            strncat(strings, "\n", sizeof(strings));
          }
        }
        ControlSetString(L, "key", "Strings", strings);
        strings[0] = '\0';
        choices = commandDevice->GetKeypadLegendChoices();
        if (NULL != choices) {
          while (NULL != *choices) {
            strncat(strings, *choices++, sizeof(strings));
            strncat(strings, "\n", sizeof(strings));
          }
        }
        ControlSetString(L, "legend", "Strings", strings);
      }
      break;
    case editGPO:
      {
        int ngpos = 0;
        if (NULL != commandDevice)
          ngpos = commandDevice->GetNGPOs();
        if (ngpos > 0) {
          ControlSetNumber(L, "gpo", "Min", 1);
          ControlSetNumber(L, "gpo", "Max", ngpos);
          ControlSetBool(L, "gpo", "Enabled", TRUE);
        }
        else
          ControlSetBool(L, "gpo", "Enabled", FALSE);
      }
      break;
    case editFan:
      {
        int nfans = 0;
        if (NULL != commandDevice)
          nfans = commandDevice->GetNFans();
        if (nfans > 0) {
          ControlSetNumber(L, "fan", "Min", 1);
          ControlSetNumber(L, "fan", "Max", nfans);
          ControlSetBool(L, "fan", "Enabled", TRUE);
        }
        else
          ControlSetBool(L, "fan", "Enabled", FALSE);
      }
      break;
    }
  }

  switch (val1) {
  case duOnGetPage:
    {
      // For compatibility with old GML files, adjust command to point
      // to correct subtype and switch to proper editor.

      if (command->ActionSubType != commandAction->editorType) {
        command->ActionSubType = commandAction->editorType;
      }

      if (pageNode != g_activePages[commandAction->editorType])
        newPage = g_activePages[commandAction->editorType];
    }
    break;
				
  case duOnGetDefaults:
    {
      if (g_bMultipleDevices) {
        // Minimal device choice combo.
        ControlSetString(L, "display", "Strings", "Default\n");
        ControlSetNumber(L, "display", "ItemIndex", 0);
        g_deviceComboContents[0] = commandDevice;
      }

      if ((editDisplay == item->ActionSubType) ||
          (editControl == item->ActionSubType)) {
        // Minimal action choice combo.
        ControlSetString(L, "type", "Strings", commandAction->name);
        ControlSetNumber(L, "type", "ItemIndex", 0);
      }

      switch (item->ActionSubType) {
      case editDisplay:
        {
          ControlSetNumber(L, "row", "Position", 0);
          ControlSetBool(L, "marquee", "Checked", FALSE);
          ControlSetNumber(L, "column", "Position", 0);
          ControlSetBool(L, "rest", "Checked", TRUE);
        }
        break;
      case editScreen:
        {
          char buf[128];
          for (int i = 0; i < SCREEN_NLINES; i++) {
            _snprintf(buf, sizeof(buf), "enabled%d", i+1);
            ControlSetBool(L, buf, "Checked", FALSE);
            _snprintf(buf, sizeof(buf), "marquee%d", i+1);
            ControlSetBool(L, buf, "Checked", FALSE);
          }
        }
        break;
      case editGPO:
        {
          ControlSetNumber(L, "gpo", "Position", 1); 
        }
        break;
      case editFan:
        {
          ControlSetNumber(L, "fan", "Position", 1); 
        }
        break;
      }
    }
    break;
				
  case duOnGetValues:
    {
      if (g_bMultipleDevices) {
        // Populate device choice combo.
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
        // Populate action choice combo.
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
          int row = strtol(command->Action.iValue1, NULL, 10);
          ControlSetNumber(L, "row", "Position", row);
          ControlSetString(L, "marquee", "Caption", "Marquee");
          int column = strtol(command->Action.iValue2, NULL, 10);
          BOOL marquee = (column < 0);
          ControlSetBool(L, "marquee", "Checked", marquee);
          ControlSetString(L, "columnCheck", "Caption", "Column:");
          ControlSetBool(L, "columnCheck", "Checked", !marquee);
          ControlSetBool(L, "column", "Enabled", !marquee);
          if (!marquee)
            ControlSetNumber(L, "column", "Position", column);
          int width = strtol(command->Action.iValue3, NULL, 10);
          BOOL rest = (width < 0);
          ControlSetString(L, "rest", "Caption", "Rest of line");
          ControlSetBool(L, "rest", "Checked", rest);
          ControlSetString(L, "widthCheck", "Caption", "Width:");
          ControlSetBool(L, "widthCheck", "Checked", !rest);
          ControlSetBool(L, "width", "Enabled", !rest);
          if (!rest)
            ControlSetNumber(L, "width", "Position", width);
        }
        break;
      case editScreen:
        {
          char lines[1024], buf[128];
          BOOL checked;
          strncpy(lines, command->Action.sValue1, sizeof(lines));
          int enabled = strtol(command->Action.iValue1, NULL, 10);
          int marquee = strtol(command->Action.iValue2, NULL, 10);
          PCHAR pval = lines;
          for (int i = 0; i < SCREEN_NLINES; i++) {
            _snprintf(buf, sizeof(buf), "enabled%d", i+1);
            checked = !(enabled & (1 << i));
            ControlSetBool(L, buf, "Checked", checked);
            _snprintf(buf, sizeof(buf), "marquee%d", i+1);
            ControlSetBool(L, buf, "Enabled", checked);
            checked = !!(marquee & (1 << i));
            ControlSetBool(L, buf, "Checked", checked);

            PCHAR next = strchr(pval, '\n');
            if (NULL != next) {
              if ((next > pval) && (*(next-1) == '\r'))
                *(next-1) = '\0';
              *next++ = '\0';
            }
            else
              next = pval + strlen(pval);
            _snprintf(buf, sizeof(buf), "line%d", i+1);
            ControlSetString(L, buf, "Text", pval);
            pval = next;
          }
        }
        break;
      case editKeypad:
        {
          ControlSetString(L, "key", "Text", command->Action.sValue1); 
          ControlSetString(L, "legend", "Text", command->Action.sValue3);
        }
        break;
      case editGPO:
        {
          int gpo = strtol(command->Action.iValue1, NULL, 10);
          ControlSetNumber(L, "gpo", "Position", gpo); 
        }
        break;
      case editFan:
        {
          int fan = strtol(command->Action.iValue1, NULL, 10);
          ControlSetNumber(L, "fan", "Position", fan); 
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
          const char *str;
          if (ControlGetString(L, "row", "Position", str))
            command->Action.iValue1 = GStrDup(str);
          BOOL marquee;
          if (ControlGetBool(L, "marquee", "Checked", marquee)) {
            if (marquee)
              command->Action.iValue2 = GStrDup("-1");
            else {
              if (ControlGetString(L, "column", "Position", str))
                command->Action.iValue2 = GStrDup(str);
            }
          }
          BOOL rest;
          if (ControlGetBool(L, "rest", "Checked", rest)) {
            if (rest)
              command->Action.iValue3 = GStrDup("-1");
            else {
              if (ControlGetString(L, "width", "Position", str))
                command->Action.iValue3 = GStrDup(str);
            }
          }
        }
        break;
      case editScreen:
        {
          char lines[1024], buf[128];
          int enabled = 0, marquee = 0;
          int nrows = 0;
          if (NULL != commandDevice)
            nrows = commandDevice->GetHeight();
          if (nrows > SCREEN_NLINES) nrows = SCREEN_NLINES;
          for (int i = 0; i < nrows; i++) {
            BOOL checked;
            _snprintf(buf, sizeof(buf), "enabled%d", i+1);
            if (ControlGetBool(L, buf, "Checked", checked) && !checked)
              enabled |= (1 << i);
            _snprintf(buf, sizeof(buf), "marquee%d", i+1);
            if (ControlGetBool(L, buf, "Checked", checked) && checked)
              marquee |= (1 << i);

            const char *str;
            _snprintf(buf, sizeof(buf), "line%d", i+1);
            if (ControlGetString(L, buf, "Text", str)) {
              if (i == 0)
                strncpy(lines, str, sizeof(lines));
              else {
                strncat(lines, "\n", sizeof(lines));
                strncat(lines, str, sizeof(lines));
              }
            }
          }
          _snprintf(buf, sizeof(buf), "%d", enabled);
          command->Action.iValue1 = GStrDup(buf);
          _snprintf(buf, sizeof(buf), "%d", marquee);
          command->Action.iValue2 = GStrDup(buf);
          command->Action.sValue1 = GStrDup(lines);
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
      case editGPO:
        {
          const char *str;
          if (ControlGetString(L, "gpo", "Position", str))
            command->Action.iValue1 = GStrDup(str);
        }
        break;
      case editFan:
        {
          const char *str;
          if (ControlGetString(L, "fan", "Position", str))
            command->Action.iValue1 = GStrDup(str);
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
      case editScreen:
        if ((val2 >= 1) && (val2 <= 8)) {
          char buf[128];
          BOOL checked;
          int line = (val2 + 1) / 2;
          if ((val2 & 1) != 0) {
            // Enabled affects marquee.
            _snprintf(buf, sizeof(buf), "enabled%d", line);
            if (!ControlGetBool(L, buf, "Checked", checked)) checked = FALSE;
            _snprintf(buf, sizeof(buf), "marquee%d", line);
            if (!checked)
              ControlSetBool(L, buf, "Checked", FALSE);
            ControlSetBool(L, buf, "Enabled", checked);
          }
          else {
            // Marquee turns off all others.
            _snprintf(buf, sizeof(buf), "marquee%d", line);
            if (!ControlGetBool(L, buf, "Checked", checked)) checked = FALSE;
            if (checked) {
              for (int i = 0; i < SCREEN_NLINES; i++) {
                if (line == i+1) continue;
                _snprintf(buf, sizeof(buf), "marquee%d", i+1);
                ControlSetBool(L, buf, "Checked", FALSE);
              }
            }            
          }
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
    g_activePages[i] = InsertDUIPageExS(tree, g_DUI, g_pages[i], 
                                        LCD_GROUP_GUID, LCD_GROUP_NAME);
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
