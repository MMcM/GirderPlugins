/* Command (action settings) user interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "display.h"

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
PageCallback(PFTree tree, PFTreeNode pageNode, PBaseNode baseNode, int duiIndex,
             int msg, WPARAM wParam, LPARAM lParam, void *userData)
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

  if (WaitForSingleObject(SF.CoreVars->LuaMutex, 1000) != WAIT_OBJECT_0)
    return NULL;

  lua_State *L = (lua_State *)SF.CoreVars->LuaState;

  PFTreeNode newPage = NULL;

  DisplayActionDeviceType deviceType;  
  DisplayDevice *commandDevice;
  DisplayAction *commandAction;

  if (duOnGetDefaults == msg) {
    // Default device and action (without command or controls).
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
  else if (duOnShow == msg) {
    // Decode device and action from command.
    EnsureDevices();
    if (!FindDisplayAction(g_devices, command, deviceType, commandDevice, commandAction))
      commandAction = DisplayActions;  // String
  }
  else {
    // Decode device and action from controls.
    if ((duOnApply == msg) ||
        ((duOnEvent == msg) && (0 == wParam))) {
      if (g_bMultipleDevices) {
        int itemIndex = 0;
        double d;
        if (ControlGetNumber(L, duiIndex, "display", "ItemIndex", d))
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
    if ((duOnApply == msg) ||
        ((duOnEvent == msg) && FALSE)) {
      int itemIndex = 0;
      if ((editDisplay == item->ActionSubType) ||
          (editControl == item->ActionSubType)) {
        double d;
        if (ControlGetNumber(L, duiIndex, "type", "ItemIndex", d))
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

  if ((duOnShow == msg) ||
      ((duOnEvent == msg) && (0 == wParam))) {
    // Handle device ranges and choices.
    switch (item->ActionSubType) {
    case editDisplay:
      ControlSetNumber(L, duiIndex, "row", "Max", 
                       (NULL == commandDevice) ? 0 : commandDevice->GetHeight() - 1);
      ControlSetNumber(L, duiIndex, "column", "Max", 
                       (NULL == commandDevice) ? 0 : commandDevice->GetWidth() - 1);
      ControlSetNumber(L, duiIndex, "width", "Max", 
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
            ControlSetBool(L, duiIndex, buf, "Visible", visible);
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
        ControlSetString(L, duiIndex, "key", "Strings", strings);
        strings[0] = '\0';
        choices = commandDevice->GetKeypadLegendChoices();
        if (NULL != choices) {
          while (NULL != *choices) {
            strncat(strings, *choices++, sizeof(strings));
            strncat(strings, "\n", sizeof(strings));
          }
        }
        ControlSetString(L, duiIndex, "legend", "Strings", strings);
      }
      break;
    case editGPO:
      {
        int ngpos = 0;
        if (NULL != commandDevice)
          ngpos = commandDevice->GetNGPOs();
        if (ngpos > 0) {
          ControlSetNumber(L, duiIndex, "gpo", "Min", 1);
          ControlSetNumber(L, duiIndex, "gpo", "Max", ngpos);
          ControlSetBool(L, duiIndex, "gpo", "Enabled", TRUE);
        }
        else
          ControlSetBool(L, duiIndex, "gpo", "Enabled", FALSE);
      }
      break;
    case editFan:
      {
        int nfans = 0;
        if (NULL != commandDevice)
          nfans = commandDevice->GetNFans();
        if (nfans > 0) {
          ControlSetNumber(L, duiIndex, "fan", "Min", 1);
          ControlSetNumber(L, duiIndex, "fan", "Max", nfans);
          ControlSetBool(L, duiIndex, "fan", "Enabled", TRUE);
        }
        else
          ControlSetBool(L, duiIndex, "fan", "Enabled", FALSE);
      }
      break;
    }
  }

  switch (msg) {
  case duOnGetDefaults:
    {
      command->Action.sValue2 = GStrDup(commandAction->key);

      switch (item->ActionSubType) {
      case editDisplay:
        {
          command->Action.iValue1 = GStrDup("0"); // Row
          command->Action.iValue2 = GStrDup("0"); // Column
          command->Action.iValue3 = GStrDup("-1"); // Rest
        }
        break;
      case editScreen:
        {
          command->Action.iValue1 = GStrDup("0"); // Enabled (all)
          command->Action.iValue2 = GStrDup("0"); // Marquee (none)
          command->Action.sValue1 = GStrDup("");
        }
        break;
      case editKeypad:
        {
          LPCSTR *choices = commandDevice->GetKeypadButtonChoices();
          command->Action.sValue1 = GStrDup((NULL != choices) ? *choices : "");
          choices = commandDevice->GetKeypadLegendChoices();
          command->Action.sValue3 = GStrDup((NULL != choices) ? *choices : "");
        }
        break;
      case editGPO:
      case editFan:
        {
          command->Action.iValue1 = GStrDup("1"); // GPO or Fan #
        }
        break;
      }
    }
    break;

  case duOnShow:
    {
      // For compatibility with old GML files, adjust command to point
      // to correct subtype and switch to proper editor.

      if (command->ActionSubType != commandAction->editorType) {
        command->ActionSubType = commandAction->editorType;
      }

      PFTreeNode editorPage = FindNodeS(PAGE_GUIDS[commandAction->editorType], 
                                        tree, NULL);
      if (pageNode != editorPage) {
        newPage = editorPage;
        break;
      }
    }
    {
      if (g_bMultipleDevices) {
        // Populate device choice combo.
        ControlSetString(L, duiIndex, "displayLabel", "Caption", "Display:");
        ControlSetBool(L, duiIndex, "displayLabel", "Visible", TRUE);

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
        ControlSetString(L, duiIndex, "display", "Strings", strings);
        ControlSetNumber(L, duiIndex, "display", "ItemIndex", itemIndex);
        ControlSetBool(L, duiIndex, "display", "Visible", TRUE);
      }
      else {
        ControlSetBool(L, duiIndex, "displayLabel", "Visible", FALSE);
        ControlSetBool(L, duiIndex, "display", "Visible", FALSE);
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
        ControlSetString(L, duiIndex, "type", "Strings", strings);
        ControlSetNumber(L, duiIndex, "type", "ItemIndex", itemIndex);
      }

      switch (item->ActionSubType) {
      case editDisplay:
        {
          int row = strtol(command->Action.iValue1, NULL, 10);
          ControlSetNumber(L, duiIndex, "row", "Position", row);
          ControlSetString(L, duiIndex, "marquee", "Caption", "Marquee");
          int column = strtol(command->Action.iValue2, NULL, 10);
          BOOL marquee = (column < 0);
          ControlSetBool(L, duiIndex, "marquee", "Checked", marquee);
          ControlSetString(L, duiIndex, "columnCheck", "Caption", "Column:");
          ControlSetBool(L, duiIndex, "columnCheck", "Checked", !marquee);
          ControlSetBool(L, duiIndex, "column", "Enabled", !marquee);
          if (!marquee)
            ControlSetNumber(L, duiIndex, "column", "Position", column);
          int width = strtol(command->Action.iValue3, NULL, 10);
          BOOL rest = (width < 0);
          ControlSetString(L, duiIndex, "rest", "Caption", "Rest of line");
          ControlSetBool(L, duiIndex, "rest", "Checked", rest);
          ControlSetString(L, duiIndex, "widthCheck", "Caption", "Width:");
          ControlSetBool(L, duiIndex, "widthCheck", "Checked", !rest);
          ControlSetBool(L, duiIndex, "width", "Enabled", !rest);
          if (!rest)
            ControlSetNumber(L, duiIndex, "width", "Position", width);
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
            ControlSetBool(L, duiIndex, buf, "Checked", checked);
            _snprintf(buf, sizeof(buf), "marquee%d", i+1);
            ControlSetBool(L, duiIndex, buf, "Enabled", checked);
            checked = !!(marquee & (1 << i));
            ControlSetBool(L, duiIndex, buf, "Checked", checked);

            PCHAR next = strchr(pval, '\n');
            if (NULL != next) {
              if ((next > pval) && (*(next-1) == '\r'))
                *(next-1) = '\0';
              *next++ = '\0';
            }
            else
              next = pval + strlen(pval);
            _snprintf(buf, sizeof(buf), "line%d", i+1);
            ControlSetString(L, duiIndex, buf, "Text", pval);
            pval = next;
          }
        }
        break;
      case editKeypad:
        {
          ControlSetString(L, duiIndex, "key", "Text", command->Action.sValue1); 
          ControlSetString(L, duiIndex, "legend", "Text", command->Action.sValue3);
        }
        break;
      case editGPO:
        {
          int gpo = strtol(command->Action.iValue1, NULL, 10);
          ControlSetNumber(L, duiIndex, "gpo", "Position", gpo); 
        }
        break;
      case editFan:
        {
          int fan = strtol(command->Action.iValue1, NULL, 10);
          ControlSetNumber(L, duiIndex, "fan", "Position", fan); 
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
          char *str;
          if (ControlGetString(L, duiIndex, "row", "Position", str))
            command->Action.iValue1 = str;
          BOOL marquee;
          if (ControlGetBool(L, duiIndex, "marquee", "Checked", marquee)) {
            if (marquee)
              command->Action.iValue2 = GStrDup("-1");
            else {
              if (ControlGetString(L, duiIndex, "column", "Position", str))
                command->Action.iValue2 = str;
            }
          }
          BOOL rest;
          if (ControlGetBool(L, duiIndex, "rest", "Checked", rest)) {
            if (rest)
              command->Action.iValue3 = GStrDup("-1");
            else {
              if (ControlGetString(L, duiIndex, "width", "Position", str))
                command->Action.iValue3 = str;
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
            if (ControlGetBool(L, duiIndex, buf, "Checked", checked) && !checked)
              enabled |= (1 << i);
            _snprintf(buf, sizeof(buf), "marquee%d", i+1);
            if (ControlGetBool(L, duiIndex, buf, "Checked", checked) && checked)
              marquee |= (1 << i);

            char *str;
            _snprintf(buf, sizeof(buf), "line%d", i+1);
            if (ControlGetString(L, duiIndex, buf, "Text", str)) {
              if (i == 0)
                strncpy(lines, str, sizeof(lines));
              else {
                strncat(lines, "\n", sizeof(lines));
                strncat(lines, str, sizeof(lines));
              }
              SafeFree(str);
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
          char *str;
          if (ControlGetString(L, duiIndex, "key", "Text", str))
            command->Action.sValue1 = str;
          if (ControlGetString(L, duiIndex, "legend", "Text", str))
            command->Action.sValue3 = str;
        }
        break;
      case editGPO:
        {
          char *str;
          if (ControlGetString(L, duiIndex, "gpo", "Position", str))
            command->Action.iValue1 = str;
        }
        break;
      case editFan:
        {
          char *str;
          if (ControlGetString(L, duiIndex, "fan", "Position", str))
            command->Action.iValue1 = str;
        }
        break;
      }
    }
    break;

  case duOnEvent:
    {
      switch (item->ActionSubType) {
      case editDisplay:
        switch (wParam) {
        case 1:
          {
            BOOL marquee;
            if (ControlGetBool(L, duiIndex, "marquee", "Checked", marquee)) {
              ControlSetBool(L, duiIndex, "columnCheck", "Checked", !marquee);
              ControlSetBool(L, duiIndex, "column", "Enabled", !marquee);
            }
          }
          break;
        case 2:
          {
            BOOL notMarquee;
            if (ControlGetBool(L, duiIndex, "columnCheck", "Checked", notMarquee)) {
              ControlSetBool(L, duiIndex, "marquee", "Checked", !notMarquee);
              ControlSetBool(L, duiIndex, "column", "Enabled", notMarquee);
            }
          }
          break;
        case 3:
          {
            BOOL rest;
            if (ControlGetBool(L, duiIndex, "rest", "Checked", rest)) {
              ControlSetBool(L, duiIndex, "widthCheck", "Checked", !rest);
              ControlSetBool(L, duiIndex, "width", "Enabled", !rest);
            }
          }
          break;
        case 4:
          {
            BOOL notRest;
            if (ControlGetBool(L, duiIndex, "widthCheck", "Checked", notRest)) {
              ControlSetBool(L, duiIndex, "rest", "Checked", !notRest);
              ControlSetBool(L, duiIndex, "width", "Enabled", notRest);
            }
          }
          break;
        }
        break;
      case editScreen:
        if ((wParam >= 1) && (wParam <= 8)) {
          char buf[128];
          BOOL checked;
          int line = (wParam + 1) / 2;
          if ((wParam & 1) != 0) {
            // Enabled affects marquee.
            _snprintf(buf, sizeof(buf), "enabled%d", line);
            if (!ControlGetBool(L, duiIndex, buf, "Checked", checked)) checked = FALSE;
            _snprintf(buf, sizeof(buf), "marquee%d", line);
            if (!checked)
              ControlSetBool(L, duiIndex, buf, "Checked", FALSE);
            ControlSetBool(L, duiIndex, buf, "Enabled", checked);
          }
          else {
            // Marquee turns off all others.
            _snprintf(buf, sizeof(buf), "marquee%d", line);
            if (!ControlGetBool(L, duiIndex, buf, "Checked", checked)) checked = FALSE;
            if (checked) {
              for (int i = 0; i < SCREEN_NLINES; i++) {
                if (line == i+1) continue;
                _snprintf(buf, sizeof(buf), "marquee%d", i+1);
                ControlSetBool(L, duiIndex, buf, "Checked", FALSE);
              }
            }            
          }
        }
        break;
      }
    }
    break;
  }

  ReleaseMutex(SF.CoreVars->LuaMutex);

  return newPage;
}

BOOL DUIOpen()
{
  PCHAR buf = PathExpand("%GIRDER%Plugins\\UI\\LCD.xml");
  g_DUI = LoadDUI(buf);
  SafeFree(buf);
  if (NULL == g_DUI) {
    GirderLogMessageEx(PLUGINNAME, "Could not open DUI File (LCD.xml).",
                       GLM_ERROR_ICON);
    return FALSE;
  }
  
  for (int i = 1; i < countof(PAGE_GUIDS); i++) {
    PFTreeNode page = FindNodeS(PAGE_GUIDS[i], g_DUI, NULL);
    if (NULL == page) {	
      GirderLogMessageEx(PLUGINNAME, "Could not find page in LCD.xml.",
                         GLM_ERROR_ICON);
      return FALSE;
    }
    SetCallbacks(g_DUI, page, PageCallback, TRUE);
  }

  return TRUE;
}

void DUIClose()
{
  DeleteFTree(g_DUI);
  g_DUI = NULL;

  g_devices.Clear();
  if (NULL != g_deviceComboContents) {
    delete [] g_deviceComboContents;
    g_deviceComboContents = NULL;
  }
  g_bDevicesValid = FALSE;
}

void DUIOpenCommand(PFTree tree)
{
  LockTree(tree, TreeLockWrite);
  MergeDUITrees(tree, g_DUI, duOnHookCommand);
  UnlockTree(tree, TreeLockWrite);
}

void DUICloseCommand(PFTree tree)
{
  LockTree(tree, TreeLockWrite);
  UnmergeDUITrees(tree, g_DUI, duOnUnHookCommand);
  UnlockTree(tree, TreeLockWrite);
}

void DUIOpenConfig(PFTree tree)
{
  LockTree(tree, TreeLockWrite);
  MergeDUITrees(tree, g_DUI, duOnHookConfig);
  UnlockTree(tree, TreeLockWrite);
}

void DUICloseConfig(PFTree tree)
{
  LockTree(tree, TreeLockWrite);
  UnmergeDUITrees(tree, g_DUI, duOnUnHookConfig);
  UnlockTree(tree, TreeLockWrite);
}

void DUIDevicesChanged()
{
  g_bDevicesValid = FALSE;
}
