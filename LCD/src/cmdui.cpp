/* Command (action settings) user interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "display.h"

#define countof(x) sizeof(x)/sizeof(x[0])

enum {
  valNONE, valSTR, valINT, valBOOL, valVAR, valLIST, valSTR2, valLIST2
};

DisplayAction DisplayActions[] = {
  { "s", "String", valSTR, DisplayString },
  { "v", "Variable", valVAR, DisplayVariable },
  { "t", "Current Date/Time", valSTR, DisplayCurrentTime },
  { "f", "Filename Variable", valVAR, DisplayFilename },
  { "x", "Close Display", valNONE, DisplayClose },
  { "c", "Clear Display", valNONE, DisplayClear },
  { "#", "Screen", valNONE, DisplayScreen },
  { "*", "Character (numerical code)", valSTR, DisplayCharacter },
  { "$", "Custom Character", valSTR, DisplayCustomCharacter },
  { "k", "Keypad Legend", valLIST2, DisplayKeypadLegend },
  { "o", "General Purpose Output", valBOOL, DisplayGPO },
  { "p", "Fan Power", valSTR, DisplayFanPower },
};

BOOL FindDisplayAction(DisplayDeviceList& devices, p_command command,
                       DisplayActionDeviceType& devtype, DisplayDevice*& device,
                       DisplayAction*& action)
{
  devtype = devDEFAULT;
  device = devices.GetDefault();

  PCHAR key = command->svalue2;

#if 0
  if ((NULL == key) || ('\0' == *key)) {
    // An older GML file.  Convert the command to the new format.
    PCHAR val = NULL;
    char buf[128];

    switch (command->ivalue1) {
    case 0:                       // String
      key = "s";
      break;
    case 1:                       // String Register
      key = "v";
      sprintf(buf, "treg%d", command->ivalue2);
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
      sprintf(buf, "pld%d", command->ivalue2);
      val = buf;
      break;
    case 6:                       // Filename Payload
      key = "f";
      sprintf(buf, "pld%d", command->ivalue2);
      val = buf;
      break;
    default:
      return FALSE;
    }
    // Position information.
    command->ivalue1 = command->lvalue1; // Row
    command->ivalue2 = command->lvalue2; // Column
    command->ivalue3 = command->lvalue3; // Width
    if (command->ivalue3 <= 0)
      command->ivalue3 = -1;    // Standardize rest value.
    command->lvalue1 = 0;       // No links.
    command->lvalue2 = 0;
    command->lvalue3 = 0;
    if (NULL != val)
      SF.realloc_pchar(&command->svalue1, val);
    SF.realloc_pchar(&command->svalue2, key);
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

/* Local variables */
static DisplayDeviceList g_devices;
static BOOL g_bMultipleDevices = FALSE;
static p_command g_editCommand = NULL;
static HWND g_commandDialog = NULL;
static HANDLE g_commandThread = NULL;

const LPARAM ALL_DEVICE = (LPARAM)-1;
const LPARAM DEFAULT_DEVICE = (LPARAM)0;

// Show input controls appropriate for this value type and optionally
// load from edited command.
static void ShowValueInputs(HWND hwnd, int valueType, BOOL reload)
{
  ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), 
             (valNONE != valueType) ? SW_SHOW : SW_HIDE);
  char legend[256];
  if (LoadString(g_hInstance,
                 (valVAR == valueType) ? IDS_VARIABLE : IDS_VALUE,
                 legend, sizeof(legend)))
    Static_SetText(GetDlgItem(hwnd, IDC_VALUEL), legend);

  ShowWindow(GetDlgItem(hwnd, IDC_VALSTR), 
             ((valSTR == valueType) || (valVAR == valueType) || (valSTR2 == valueType)) ?
             SW_SHOW : SW_HIDE);

  ShowWindow(GetDlgItem(hwnd, IDC_VALINT), 
             (valINT == valueType) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VAL_SPIN), 
             (valINT == valueType) ? SW_SHOW : SW_HIDE);

  ShowWindow(GetDlgItem(hwnd, IDC_VALBOOL), 
             (valBOOL == valueType) ? SW_SHOW : SW_HIDE);

  ShowWindow(GetDlgItem(hwnd, IDC_VALLIST),
             ((valLIST == valueType) || (valLIST2 == valueType)) ? SW_SHOW : SW_HIDE);

  ShowWindow(GetDlgItem(hwnd, IDC_VALUE2L), 
             ((valSTR2 == valueType) || (valLIST2 == valueType)) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VALSTR2),
             (valSTR2 == valueType) ? SW_SHOW : SW_HIDE);

  ShowWindow(GetDlgItem(hwnd, IDC_VALLIST2),
             (valLIST2 == valueType) ? SW_SHOW : SW_HIDE);

  if (reload && (NULL != g_editCommand)) {
    Button_SetCheck(GetDlgItem(hwnd, IDC_VALBOOL), g_editCommand->bvalue1);
    if (valINT == valueType)
      Edit_SetText(GetDlgItem(hwnd, IDC_VALINT), g_editCommand->svalue1);
    else if ((valLIST == valueType) || (valLIST2 == valueType))
      ComboBox_SetText(GetDlgItem(hwnd, IDC_VALLIST), g_editCommand->svalue1);
    else
      Edit_SetText(GetDlgItem(hwnd, IDC_VALSTR), g_editCommand->svalue1);
    if (valLIST2 == valueType)
      ComboBox_SetText(GetDlgItem(hwnd, IDC_VALLIST2), g_editCommand->svalue3);
    else if (valSTR2 == valueType)
      Edit_SetText(GetDlgItem(hwnd, IDC_VALSTR2), g_editCommand->svalue3);
  }
}

static void FillValueListChoices(HWND hwnd, UINT ctlid, UINT strid, LPCSTR *choices)
{
  char legend[256];
  if (LoadString(g_hInstance, strid, legend, sizeof(legend)))
    Static_SetText(GetDlgItem(hwnd, (IDC_VALLIST == ctlid) ? IDC_VALUEL : IDC_VALUE2L),
                   legend);
  HWND combo = GetDlgItem(hwnd, ctlid);
  ComboBox_ResetContent(combo);
  if (NULL != choices) {
    while (NULL != *choices)
      ComboBox_AddString(combo, *choices++);
  }
}

// Show position inputs and optionally load from edited command.
static void ShowPositionInputs(HWND hwnd, UINT ctlid, BOOL reload)
{
  int sw = (IDC_ROW == ctlid) ? SW_SHOW : SW_HIDE;
  ShowWindow(GetDlgItem(hwnd, IDC_ROWL), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_ROW), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_ROW_SPIN), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_USE_WRAP), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_USE_COL), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_COL), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_COL_SPIN), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_USE_REST), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_WIDTH), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), sw);

  if (reload) {
    UpDown_SetPos(GetDlgItem(hwnd, IDC_ROW_SPIN),
                  (IDC_ROW == ctlid) ? g_editCommand->ivalue1 : 0);
    UpDown_SetPos(GetDlgItem(hwnd, IDC_COL_SPIN),
                  (IDC_ROW == ctlid) ? g_editCommand->ivalue2 : 0);
    BOOL wrap = (IDC_ROW == ctlid) && (g_editCommand->ivalue2 < 0);
    Button_SetCheck(GetDlgItem(hwnd, IDC_USE_WRAP), wrap);
    Button_SetCheck(GetDlgItem(hwnd, IDC_USE_COL), !wrap);
    UpDown_SetPos(GetDlgItem(hwnd, IDC_WIDTH_SPIN), g_editCommand->ivalue3);
    EnableWindow(GetDlgItem(hwnd, IDC_COL), !wrap);
    EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), !wrap);
    EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), !wrap);
    EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), !wrap);
    BOOL rest = (wrap || (IDC_ROW != ctlid) || (g_editCommand->ivalue3 <= 0));
    Button_SetCheck(GetDlgItem(hwnd, IDC_USE_REST), rest);
    Button_SetCheck(GetDlgItem(hwnd, IDC_USE_WIDTH), !rest);
    EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), !rest);
    EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), !rest);
  }

  sw = (IDC_FANGPO == ctlid) ? SW_SHOW : SW_HIDE;
  ShowWindow(GetDlgItem(hwnd, IDC_VALUE2L), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_FANGPO), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_FANGPO_SPIN), sw);
  if (reload)
    UpDown_SetPos(GetDlgItem(hwnd, IDC_FANGPO_SPIN),
                  (IDC_FANGPO == ctlid) ? g_editCommand->ivalue1 : 0);
}

// Show inputs for screen command and optionally load from edited command.
static void ShowScreenInputs(HWND hwnd, BOOL show, BOOL reload)
{
  ShowWindow(GetDlgItem(hwnd, IDC_SCREENL), (show) ? SW_SHOW : SW_HIDE);
  int nrows = 0;
  if (show) {
    nrows = DisplayHeight();
    if (nrows > 4) nrows = 4;
  }
  for (int i = 0; i < nrows; i++) {
    ShowWindow(GetDlgItem(hwnd, IDC_ENABLE_LINE1 + i), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_LINE1 + i), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i), SW_SHOW);
  }    
  for (i = nrows; i < 4; i++) {
    ShowWindow(GetDlgItem(hwnd, IDC_ENABLE_LINE1 + i), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_LINE1 + i), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i), SW_HIDE);
  }

  if (reload) {
    char buf[1024];
    PCHAR pval = buf;
    if (show)
      strncpy(buf, g_editCommand->svalue1, sizeof(buf));
    else
      buf[0] = '\0';

    for (i = 0; i < 4; i++) {
      Button_SetCheck(GetDlgItem(hwnd, IDC_ENABLE_LINE1 + i),
                      !show || !(g_editCommand->ivalue1 & (1 << i)));
      Button_SetCheck(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i),
                      show && !!(g_editCommand->ivalue2 & (1 << i)));

      PCHAR next = strchr(pval, '\n');
      if (NULL != next) {
        if ((next > pval) && (*(next-1) == '\r'))
          *(next-1) = '\0';
        *next++ = '\0';
      }
      else
        next = pval + strlen(pval);
      Edit_SetText(GetDlgItem(hwnd, IDC_LINE1 + i), pval);
      pval = next;
    }
  }
}
                            
// Show input controls appropriate for this command type and
// optionally load from edited command.
static void ShowCommandInputs(HWND hwnd, DisplayDevice *commandDevice, 
                              DisplayAction *action, BOOL reload)
{
  if ((NULL == action) ||
      (DisplayClear == action->function) ||
      (DisplayClose == action->function)) {
    ShowValueInputs(hwnd, valNONE, reload);
    ShowPositionInputs(hwnd, 0, reload);
    ShowScreenInputs(hwnd, FALSE, reload);
  }
  else if (DisplayScreen == action->function) {
    ShowValueInputs(hwnd, valNONE, reload);
    ShowPositionInputs(hwnd, 0, reload);
    ShowScreenInputs(hwnd, TRUE, reload);
  }
  else if (DisplayKeypadLegend == action->function) {
    FillValueListChoices(hwnd, IDC_VALLIST, 
                         IDS_BUTTON, commandDevice->GetKeypadButtonChoices());
    FillValueListChoices(hwnd, IDC_VALLIST2, 
                         IDS_LEGEND, commandDevice->GetKeypadLegendChoices());
    ShowValueInputs(hwnd, valLIST2, reload);
    ShowPositionInputs(hwnd, 0, reload);
    ShowScreenInputs(hwnd, FALSE, reload);
  }
  else if ((DisplayGPO == action->function) ||
           (DisplayFanPower == action->function)) {
    char legend[256];
    if (LoadString(g_hInstance,
                   (DisplayGPO == action->function) ? IDS_GPOL : IDS_FANL,
                   legend, sizeof(legend)))
      Static_SetText(GetDlgItem(hwnd, IDC_VALUE2L), legend);
    UpDown_SetRange(GetDlgItem(hwnd, IDC_FANGPO_SPIN),
                    (DisplayGPO == action->function) ? 
                    commandDevice->GetGPOs() : commandDevice->GetFans(),
                    1);
    ShowValueInputs(hwnd, action->valueType, reload);
    ShowPositionInputs(hwnd, IDC_FANGPO, reload);
    ShowScreenInputs(hwnd, FALSE, reload);
  }
  else if ((valSTR2 == action->valueType) ||
           (valLIST2 == action->valueType)) {
    ShowValueInputs(hwnd, action->valueType, reload);
    ShowPositionInputs(hwnd, 0, reload);
    ShowScreenInputs(hwnd, FALSE, reload);
  }
  else {
    UpDown_SetRange(GetDlgItem(hwnd, IDC_ROW_SPIN), commandDevice->GetHeight() - 1, 0);
    UpDown_SetRange(GetDlgItem(hwnd, IDC_COL_SPIN), commandDevice->GetWidth() - 1, 0);
    ShowValueInputs(hwnd, action->valueType, reload);
    ShowPositionInputs(hwnd, IDC_ROW, reload);
    ShowScreenInputs(hwnd, FALSE, reload);
  }
}

static void FillCommandChoices(HWND hwnd, 
                               DisplayDevice *commandDevice, DisplayAction *selact)
{
  HWND combo = GetDlgItem(hwnd, IDC_TYPE);
  ComboBox_ResetContent(combo);
  for (size_t i = 0; i < countof(DisplayActions); i++) {
    DisplayAction *action = DisplayActions + i;
    if (NULL == commandDevice) {
      if (DisplayClose !=  action->function) continue;
    }
    else if (DisplayKeypadLegend == action->function) {
      if (!commandDevice->HasKeypadLegends()) continue;
    }
    else if (DisplayGPO == action->function) {
      if (commandDevice->GetGPOs() <= 0) continue;
    }
    else if (DisplayFanPower == action->function) {
      if (commandDevice->GetFans() <= 0) continue;
    }
    int idx = ComboBox_AddString(combo, action->name);
    ComboBox_SetItemData(combo, idx, action);
    if (action == selact)
      ComboBox_SetCurSel(combo, idx);
  }
}

// Load state from edited command.
static void LoadCommandSettings(HWND hwnd)
{
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);

  if (NULL == g_editCommand) {
    ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_TYPE), CB_ERR);
    ShowCommandInputs(hwnd, NULL, NULL, TRUE);
    return;
  }

  EnterCriticalSection(&g_editCommand->critical_section);
  
  DisplayActionDeviceType deviceType;  
  DisplayDevice *commandDevice;
  DisplayAction *action;
  if (!FindDisplayAction(g_devices, g_editCommand, 
                         deviceType, commandDevice, 
                         action))
    action = DisplayActions;    // String

  if (g_bMultipleDevices) {
    LPARAM devdata;
    switch (deviceType) {
    case devDEFAULT:
      devdata = DEFAULT_DEVICE;
      break;
    case devALL:
      devdata = ALL_DEVICE;
      break;
    case devNAMED:
      devdata = (LPARAM)commandDevice;
      break;
    case devUNKNOWN:
      devdata = (LPARAM)-2;     // Something that won't match.
      break;
    }
    HWND combo = GetDlgItem(hwnd, IDC_DISPLAY);
    int nidx = ComboBox_GetCount(combo);
    for (int idx = 0; idx < nidx; idx++) {
      if (devdata == ComboBox_GetItemData(combo, idx)) {
        ComboBox_SetCurSel(combo, idx);
        break;
      }
    }
  }

  FillCommandChoices(hwnd, commandDevice, action);
  
  ShowCommandInputs(hwnd, commandDevice, action, TRUE);

  LeaveCriticalSection(&g_editCommand->critical_section);
}

// Save state back into edited command.
static BOOL SaveCommandSettings(HWND hwnd)
{
  char buf[1024];

  if (g_editCommand == NULL)
     return FALSE;

  EnterCriticalSection(&g_editCommand->critical_section);

  HWND combo = GetDlgItem(hwnd, IDC_TYPE);
  int selidx = ComboBox_GetCurSel(combo);
  DisplayAction *action = (DisplayAction *)
    ComboBox_GetItemData(combo, selidx);
  PCHAR key = (NULL != action) ? (PCHAR)action->key : "?";
  if (g_bMultipleDevices) {
    combo = GetDlgItem(hwnd, IDC_DISPLAY);
    selidx = ComboBox_GetCurSel(combo);
    if (CB_ERR != selidx) {
      LPARAM devdata = ComboBox_GetItemData(combo, selidx);
      if (DEFAULT_DEVICE != devdata) {
        if (ALL_DEVICE == devdata)
          strcpy(buf, "*");
        else
          strncpy(buf, ((DisplayDevice *)devdata)->GetName(), sizeof(buf));
        strncat(buf, ":", sizeof(buf));
        strncat(buf, key, sizeof(buf));
        key = buf;
      }
    }
  }
  SF.realloc_pchar(&g_editCommand->svalue2, key);

  if (DisplayScreen == action->function) {
    PCHAR pval = buf;
    g_editCommand->ivalue1 = g_editCommand->ivalue2 = 0;
    int nrows = DisplayHeight();
    if (nrows > 4) nrows = 4;
    for (int i = 0; i < nrows; i++) {
      if (i > 0)
        *pval++ = '\n';

      if (!Button_GetCheck(GetDlgItem(hwnd, IDC_ENABLE_LINE1 + i)))
        g_editCommand->ivalue1 |= (1 << i);
      if (Button_GetCheck(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i)))
        g_editCommand->ivalue2 |= (1 << i);

      Edit_GetText(GetDlgItem(hwnd, IDC_LINE1 + i), pval, sizeof(buf) - (pval - buf));
      pval += strlen(pval);
    }
    SF.realloc_pchar(&g_editCommand->svalue1, buf);
  }
  else {
    if (NULL != action) {
      switch (action->valueType) {
      case valNONE:
      case valBOOL:
        break;
      case valINT:
        Edit_GetText(GetDlgItem(hwnd, IDC_VALINT), buf, sizeof(buf));
        SF.realloc_pchar(&g_editCommand->svalue1, buf);
        break;
      case valLIST:
      case valLIST2:
        ComboBox_GetText(GetDlgItem(hwnd, IDC_VALLIST), buf, sizeof(buf));
        SF.realloc_pchar(&g_editCommand->svalue1, buf);
        break;
      default:
        Edit_GetText(GetDlgItem(hwnd, IDC_VALSTR), buf, sizeof(buf));
        SF.realloc_pchar(&g_editCommand->svalue1, buf);
      }
      switch (action->valueType) {
      case valLIST2:
        ComboBox_GetText(GetDlgItem(hwnd, IDC_VALLIST2), buf, sizeof(buf));
        SF.realloc_pchar(&g_editCommand->svalue3, buf);
        break;
      case valSTR2:
        Edit_GetText(GetDlgItem(hwnd, IDC_VALSTR2), buf, sizeof(buf));
        SF.realloc_pchar(&g_editCommand->svalue3, buf);
      }
    }

    g_editCommand->bvalue1 = Button_GetCheck(GetDlgItem(hwnd, IDC_VALBOOL));

    if ((DisplayGPO == action->function) ||
        (DisplayFanPower == action->function)) {
      g_editCommand->ivalue1 = UpDown_GetPos(GetDlgItem(hwnd, IDC_FANGPO_SPIN));
    }
    else {
      g_editCommand->ivalue1 = UpDown_GetPos(GetDlgItem(hwnd, IDC_ROW_SPIN));
      if (Button_GetCheck(GetDlgItem(hwnd, IDC_USE_WRAP)))
        g_editCommand->ivalue2 = -1;
      else
        g_editCommand->ivalue2 = UpDown_GetPos(GetDlgItem(hwnd, IDC_COL_SPIN));
      if (Button_GetCheck(GetDlgItem(hwnd, IDC_USE_REST)))
        g_editCommand->ivalue3 = -1;
      else
        g_editCommand->ivalue3 = UpDown_GetPos(GetDlgItem(hwnd, IDC_WIDTH_SPIN));
    }
  }
  
  g_editCommand->actiontype = PLUGINNUM;
  SF.set_command(g_editCommand);

  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);

  LeaveCriticalSection(&g_editCommand->critical_section);

  return TRUE;
}
     
static BOOL CALLBACK CommandDialogProc(HWND hwnd, UINT uMsg, 
                                       WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      g_commandDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PLUGIN)));

      SetWindowText(hwnd, PLUGINNAME);
			
      if (g_bMultipleDevices) {
        HWND combo = GetDlgItem(hwnd, IDC_DISPLAY);
        int idx = ComboBox_AddString(combo, " All");
        ComboBox_SetItemData(combo, idx, ALL_DEVICE);
        idx = ComboBox_AddString(combo, " Default");
        ComboBox_SetItemData(combo, idx, DEFAULT_DEVICE);
        for (DisplayDevice *dev = g_devices.GetFirst(); NULL != dev;
             dev = dev->GetNext()) {
          idx = ComboBox_AddString(combo, dev->GetName());
          ComboBox_SetItemData(combo, idx, dev);
        }
        ShowWindow(GetDlgItem(hwnd, IDC_DISPLAYL), SW_SHOW);
        ShowWindow(combo, SW_SHOW);
      }

      LoadCommandSettings(hwnd);

      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      return TRUE;
    }

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (SaveCommandSettings(hwnd))
        EndDialog(hwnd, TRUE);
      return TRUE;

    case IDC_APPLY:
      SaveCommandSettings(hwnd);
      return TRUE;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return TRUE;

    case IDC_DISPLAY:
      if (HIWORD(wParam) == CBN_SELENDOK) {
        HWND combo = GetDlgItem(hwnd, IDC_DISPLAY);
        int selidx = ComboBox_GetCurSel(combo);
        LPARAM devdata = ComboBox_GetItemData(combo, selidx);
        DisplayDevice *commandDevice;
        if (DEFAULT_DEVICE == devdata)
          commandDevice = g_devices.GetDefault();
        else if (ALL_DEVICE == devdata)
          commandDevice = NULL;
        else
          commandDevice = (DisplayDevice *)devdata;
        DisplayAction *selact = NULL;
        combo = GetDlgItem(hwnd, IDC_TYPE);
        selidx = ComboBox_GetCurSel(combo);
        if (CB_ERR != selidx)
          // Maintain existing action type selection if possible.
          selact = (DisplayAction *)ComboBox_GetItemData(combo, selidx);
        FillCommandChoices(hwnd, commandDevice, selact);
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_TYPE:
      if (HIWORD(wParam) == CBN_SELENDOK) {
        HWND combo = GetDlgItem(hwnd, IDC_TYPE);
        int selidx = ComboBox_GetCurSel(combo);
        DisplayAction *action = (DisplayAction *)
          ComboBox_GetItemData(combo, selidx);
        combo = GetDlgItem(hwnd, IDC_DISPLAY);
        selidx = ComboBox_GetCurSel(combo);
        LPARAM devdata = ComboBox_GetItemData(combo, selidx);
        DisplayDevice *commandDevice;
        if (DEFAULT_DEVICE == devdata)
          commandDevice = g_devices.GetDefault();
        else if (ALL_DEVICE == devdata)
          commandDevice = NULL;
        else
          commandDevice = (DisplayDevice *)devdata;
        ShowCommandInputs(hwnd, commandDevice, action, FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), (NULL != action));
      }
      break;

    case IDC_VALSTR:
    case IDC_VALINT:
    case IDC_ROW:
    case IDC_COL:
    case IDC_WIDTH:
      if (HIWORD(wParam) == EN_CHANGE) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_VALBOOL:
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      break;

    case IDC_USE_WRAP:
      EnableWindow(GetDlgItem(hwnd, IDC_COL), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), FALSE);
      Button_SetCheck(GetDlgItem(hwnd, IDC_USE_REST), TRUE);
      Button_SetCheck(GetDlgItem(hwnd, IDC_USE_WIDTH), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), FALSE);
      /* falls through */
    case IDC_USE_REST:
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      break;

    case IDC_USE_COL:
      EnableWindow(GetDlgItem(hwnd, IDC_COL), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      break;

    case IDC_USE_WIDTH:
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      break;

    case IDC_ENABLE_LINE1:
    case IDC_ENABLE_LINE2:
    case IDC_ENABLE_LINE3:
    case IDC_ENABLE_LINE4:
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      break;

    case IDC_MARQUEE_LINE1:
    case IDC_MARQUEE_LINE2:
    case IDC_MARQUEE_LINE3:
    case IDC_MARQUEE_LINE4:
      {
        int off = LOWORD(wParam) - IDC_MARQUEE_LINE1;
        BOOL check = !Button_GetCheck(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + off));
        for (int i = 0; i < 4; i++) {
          Button_SetCheck(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i), 
                          ((i == off) & check));
        }
      }
      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      break;
    }
    break;

  case WM_NOTIFY:
    {
      LPNMHDR phdr = (LPNMHDR)lParam;
      switch (phdr->code) {
      case UDN_DELTAPOS:
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
        break;
      }
    }
    break;

  case WM_USER+100:
    LoadCommandSettings(hwnd);
    return TRUE;
  }
  return FALSE;
}

static DWORD WINAPI CommandThread(LPVOID lpParam)
{
  g_devices.LoadFromRegistry();
  g_bMultipleDevices = ((NULL == g_devices.GetFirst()) ||
                        (NULL != g_devices.GetFirst()->GetName()));

  DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_COMMAND), DisplayWindowParent(), 
                 CommandDialogProc, (LPARAM)lpParam);
  g_commandDialog = NULL;
   
  g_devices.Clear();
  return 0;
}

// Plugin request for Settings dialog.
void OpenCommandUI()
{
  // TODO: I think there is a race condition here when thread is
  // starting or finishing
  if (NULL != g_commandDialog) {
    SetForegroundWindow(g_commandDialog);
  }
  else {
    if (NULL != g_commandThread)
      CloseHandle(g_commandThread);
    DWORD dwThreadId;
    g_commandThread = CreateThread(NULL, 0, &CommandThread, NULL, 0, &dwThreadId);
    if (NULL == g_commandThread)
      MessageBox(NULL, "Cannot create dialog thread.", "Error", MB_OK);
  }
}

// Plugin request to edit another command.
void UpdateCommandUI(p_command command)
{
  g_editCommand = command;
  if (NULL != g_commandDialog)
    SendMessage(g_commandDialog, WM_USER+100, 0, 0);
}

// Plugin request to close any Settings dialog.
void CloseCommandUI()
{
  if (NULL != g_commandDialog) {
    SendMessage(g_commandDialog, WM_DESTROY, 0, 0);
    WaitForSingleObject(g_commandThread, 4000);
    CloseHandle(g_commandThread);
  }
}
