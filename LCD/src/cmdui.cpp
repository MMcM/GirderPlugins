/* Command (action settings) user interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "display.h"

#define countof(x) sizeof(x)/sizeof(x[0])

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
  { "o", "General Purpose Output", valBOOL, DisplayGPO },
};

DisplayAction *FindDisplayAction(p_command command)
{
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
      return NULL;
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
  for (size_t i = 0; i < countof(DisplayActions); i++) {
    if (!strcmp(DisplayActions[i].key, key))
      return DisplayActions + i;
  }
  return NULL;
}

/* Local variables */
p_command g_editCommand = NULL;
HWND g_commandDialog = NULL;
HANDLE g_commandThread = NULL;

// Show input controls appropriate for this value type and optionally
// load from edited command.
static void ShowValueInputs(HWND hwnd, DisplayValueType valueType, BOOL reload)
{
  ShowWindow(GetDlgItem(hwnd, IDC_VALUEL), 
             (valNONE != valueType) ? SW_SHOW : SW_HIDE);
  char trans[256];
  SF.i18n_translate((valVAR == valueType) ? "Variable:" : "Value:",
                    trans, sizeof(trans));
  SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

  ShowWindow(GetDlgItem(hwnd, IDC_VALSTR), 
             ((valSTR == valueType) || (valVAR == valueType)) ? SW_SHOW : SW_HIDE);

  ShowWindow(GetDlgItem(hwnd, IDC_VALINT), 
             (valINT == valueType) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_VAL_SPIN), 
             (valINT == valueType) ? SW_SHOW : SW_HIDE);

  ShowWindow(GetDlgItem(hwnd, IDC_VALBOOL), 
             (valBOOL == valueType) ? SW_SHOW : SW_HIDE);

  if (reload && (NULL != g_editCommand)) {
    Button_SetCheck(GetDlgItem(hwnd, IDC_VALBOOL), g_editCommand->bvalue1);
    if (valINT == valueType)
      SetWindowText(GetDlgItem(hwnd, IDC_VALINT), g_editCommand->svalue1);
    else
      SetWindowText(GetDlgItem(hwnd, IDC_VALSTR), g_editCommand->svalue1);
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

  sw = (IDC_GPO == ctlid) ? SW_SHOW : SW_HIDE;
  ShowWindow(GetDlgItem(hwnd, IDC_GPOL), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_GPO), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_GPO_SPIN), sw);
  if (reload)
    UpDown_SetPos(GetDlgItem(hwnd, IDC_GPO_SPIN),
                  (IDC_GPO == ctlid) ? g_editCommand->ivalue1 : 0);
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
      SetWindowText(GetDlgItem(hwnd, IDC_LINE1 + i), pval);
      pval = next;
    }
  }
}

// Show input controls appropriate for this command type and
// optionally load from edited command.
static void ShowCommandInputs(HWND hwnd, DisplayAction *action, BOOL reload)
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
  else if (DisplayGPO == action->function) {
    ShowValueInputs(hwnd, valBOOL, reload);
    ShowPositionInputs(hwnd, IDC_GPO, reload);
    ShowScreenInputs(hwnd, FALSE, reload);
  }
  else {
    ShowValueInputs(hwnd, action->valueType, reload);
    ShowPositionInputs(hwnd, IDC_ROW, reload);
    ShowScreenInputs(hwnd, FALSE, reload);
  }
}

// Load state from edited command.
static void LoadCommandSettings(HWND hwnd)
{
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);

  if (NULL == g_editCommand) {
    ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_TYPE), CB_ERR);
    ShowCommandInputs(hwnd, NULL, TRUE);
    return;
  }

  EnterCriticalSection(&g_editCommand->critical_section);

  DisplayAction *action = FindDisplayAction(g_editCommand);
  if (NULL == action)
    action = DisplayActions;    // String

  for (int idx = 0; idx < countof(DisplayActions); idx++) {
    DisplayAction *itemAction = (DisplayAction *)
      ComboBox_GetItemData(GetDlgItem(hwnd, IDC_TYPE), idx);
    if (itemAction == action) {
      ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_TYPE), idx);      
      break;
    }
  }
  
  ShowCommandInputs(hwnd, action, TRUE);

  LeaveCriticalSection(&g_editCommand->critical_section);
}

// Save state back into edited command.
static BOOL SaveCommandSettings(HWND hwnd)
{
  char buf[1024];

  if (g_editCommand == NULL)
     return FALSE;

  EnterCriticalSection(&g_editCommand->critical_section);

  int idx = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_TYPE));
  DisplayAction *action = (DisplayAction *)
    ComboBox_GetItemData(GetDlgItem(hwnd, IDC_TYPE), idx);
  if (NULL != action)
    SF.realloc_pchar(&g_editCommand->svalue2, (PCHAR)action->key);
  else
    SF.realloc_pchar(&g_editCommand->svalue2, "?");

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

      GetWindowText(GetDlgItem(hwnd, IDC_LINE1 + i), pval, sizeof(buf) - (pval - buf));
      pval += strlen(pval);
    }
    SF.realloc_pchar(&g_editCommand->svalue1, buf);
  }
  else if (DisplayGPO == action->function) {
    g_editCommand->bvalue1 = Button_GetCheck(GetDlgItem(hwnd, IDC_VALBOOL));
    g_editCommand->ivalue1 = UpDown_GetPos(GetDlgItem(hwnd, IDC_GPO_SPIN));
  }
  else {
    if (NULL != action) {
      switch (action->valueType) {
      case valNONE:
      case valBOOL:
        break;
      case valINT:
        GetWindowText(GetDlgItem(hwnd, IDC_VALINT), buf, sizeof(buf));
        SF.realloc_pchar(&g_editCommand->svalue1, buf);
        break;
      default:
        GetWindowText(GetDlgItem(hwnd, IDC_VALSTR), buf, sizeof(buf));
        SF.realloc_pchar(&g_editCommand->svalue1, buf);
      }
    }

    g_editCommand->bvalue1 = Button_GetCheck(GetDlgItem(hwnd, IDC_VALBOOL));

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
      char trans[256];
      	
      g_commandDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PLUGIN)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.i18n_translate("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.i18n_translate("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      SF.i18n_translate("Apply", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      SF.i18n_translate("Type:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_TYPEL), trans);

      SF.i18n_translate("Value:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_VALUEL), trans);

      SF.i18n_translate("Row:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_ROWL), trans);

      SF.i18n_translate("Marquee", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_WRAP), trans);

      SF.i18n_translate("Column:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_COL), trans);

      SF.i18n_translate("Rest of line", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_REST), trans);

      SF.i18n_translate("Width:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_USE_WIDTH), trans);

      int ngpos = DisplayGPOs();

      HWND combo = GetDlgItem(hwnd, IDC_TYPE);
      for (size_t i = 0; i < countof(DisplayActions); i++) {
        DisplayAction *action = DisplayActions + i;
        if (DisplayGPO == action->function) {
          if (ngpos <= 0) continue;
        }
        int idx = ComboBox_AddString(combo, action->name);
        ComboBox_SetItemData(combo, idx, action);
      }

      int nrows = DisplayHeight();
      int ncols = DisplayWidth();

      UpDown_SetRange(GetDlgItem(hwnd, IDC_ROW_SPIN), nrows-1, 0);
      UpDown_SetRange(GetDlgItem(hwnd, IDC_COL_SPIN), ncols-1, 0);
      UpDown_SetRange(GetDlgItem(hwnd, IDC_WIDTH_SPIN), ncols, 1);
      UpDown_SetRange(GetDlgItem(hwnd, IDC_GPO_SPIN), ngpos, 1);

      LoadCommandSettings(hwnd);

      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      return 0;
    }

  case WM_DESTROY: 
    PostQuitMessage(0); 
    return 0;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (SaveCommandSettings(hwnd))
        EndDialog(hwnd, TRUE);
      return 1;

    case IDC_APPLY:
      SaveCommandSettings(hwnd);
      return 1;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return 1;

    case IDC_TYPE:
      if (HIWORD(wParam) == CBN_SELENDOK) {
        int idx = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_TYPE));
        DisplayAction *action = (DisplayAction *)
          ComboBox_GetItemData(GetDlgItem(hwnd, IDC_TYPE), idx);
        ShowCommandInputs(hwnd, action, FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), (NULL != action));
      }
      break;

    case IDC_VALSTR:
    case IDC_ROW:
    case IDC_COL:
    case IDC_WIDTH:
      if (HIWORD(wParam) == EN_CHANGE) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
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
      break;

    case IDC_USE_COL:
      EnableWindow(GetDlgItem(hwnd, IDC_COL), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), TRUE);
      break;

    case IDC_USE_WIDTH:
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), TRUE);
      EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), TRUE);
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
    return 1;
  }
  return 0;
}

static DWORD WINAPI CommandThread(LPVOID lpParam)
{
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_ACTION), NULL, 
                               CommandDialogProc, (LPARAM)lpParam);
  g_commandDialog = NULL;
  return 0;
}

// Plugin request for Settings dialog.
void OpenCommandUI()
{
   DWORD dwThreadId;
   if (NULL != g_commandDialog) {
     SetForegroundWindow(g_commandDialog);
   }
   else {
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
