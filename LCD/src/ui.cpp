/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"
#include "display.h"

#define countof(x) sizeof(x)/sizeof(x[0])

struct DisplayDeviceEntry {
  const char *desc;
  const char *lib;
  const char *dev;
} DisplayDevices[] = {
  { "Simulated LCD", "SIMLCD", NULL },
  { "LCDriver 1.x", "LCDRVR1", NULL },
  { "Crystalfontz 632SS (16x2)", "CFONTZLCD", "632SS" },
  { "Crystalfontz 632SG (16x2, backlight)", "CFONTZLCD", "632SG" },
  { "Crystalfontz 634SS (20x4)", "CFONTZLCD", "634SS" },
  { "Crystalfontz 634SG (20x4, backlight)", "CFONTZLCD", "634SG" },
  { "Crystalfontz 633 (16x2, backlight, keypad)", "CFONTZLCD", "633" },
  { "Crystalfontz 631 (20x2, backlight, keypad)", "CFONTZLCD", "631" },
  { "Matrix Orbital LCD2021 (20x2)", "MOLCD", "LCD2021" }, // discontinued 7/20/00
  { "Matrix Orbital LCD4021 (40x2)", "MOLCD", "LCD4021" },
  { "Matrix Orbital LCD2041 (20x4)", "MOLCD", "LCD2041" },
  { "Matrix Orbital LCD4041 (40x4)", "MOLCD", "LCD4041" },
  { "Matrix Orbital LK162-12 (16x2, keypad)", "MOLCD", "LK162" },
  { "Matrix Orbital LK202-25 (20x2, keypad)", "MOLCD", "LK202" },
  { "Matrix Orbital LK204-25 (20x4, keypad)", "MOLCD", "LK204" },
  { "Matrix Orbital LK202-24-USB (20x2, keypad, USB)", "MOLCD", "LK202U" },
  { "Matrix Orbital LK204-24-USB (20x4, keypad, USB)", "MOLCD", "LK204U" },
  { "Matrix Orbital LK402-12 (40x2, keypad)", "MOLCD", "LK402" },
  { "Matrix Orbital LK404-55 (40x4, keypad)", "MOLCD", "LK404" },
  { "Matrix Orbital VFD2021 (20x2, vfd)", "MOLCD", "VFD2021" }, // discontinued 7/20/00
  { "Matrix Orbital VFD2041 (20x4, vfd)", "MOLCD", "VFD2041" },
  { "Matrix Orbital VK202-25 (20x2, vfd, keypad)", "MOLCD", "VK202" },
  { "Matrix Orbital VK204-25 (20x4, vfd, keypad)", "MOLCD", "VK204" },
  { "Matrix Orbital VK202-24-USB (20x2, vfd, keypad, USB)", "MOLCD", "VK202U" },
  { "Matrix Orbital VK204-24-USB (20x4, vfd, keypad, USB)", "MOLCD", "VK204U" },
  { "Futaba VFD (20x4)", "FutabaVFD", "M204" },
  { "Futaba VFD (40x2)", "FutabaVFD", "M402" },
  { "HD44780 (parallel)", "PARALCD", NULL }
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
  { "o", "General Purpose Output", valBOOL, DisplayGPO },
};

DisplayAction *FindDisplayAction(p_command command)
{
  PCHAR key = command->svalue2;
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

  for (size_t i = 0; i < countof(DisplayActions); i++) {
    if (!strcmp(DisplayActions[i].key, key))
      return DisplayActions + i;
  }
  return NULL;
}

/* Local variables */
DisplayDevice *g_editDevice = NULL;
HMODULE g_editDevlib = NULL;

HANDLE g_configThread = NULL;
HWND g_configDialog = NULL;

p_command g_editCommand = NULL;
HWND g_commandDialog = NULL;
HANDLE g_commandThread = NULL;

// Copy settings from registry (via new device) into controls.
static void LoadConfigSettings(HWND hwnd)
{
  int idx = SendMessage(GetDlgItem(hwnd, IDC_DEVICE), CB_GETCURSEL, 0, 0);
  DisplayDeviceEntry *entry = (DisplayDeviceEntry *)
    SendMessage(GetDlgItem(hwnd, IDC_DEVICE), CB_GETITEMDATA, idx, 0);
  if (NULL == entry) return;
  if (NULL != g_editDevice)
    DisplayDevice::Destroy(g_editDevice, g_editDevlib);
  if (!DisplayDevice::Create(g_editDevice, g_editDevlib,
                             GetParent(hwnd), entry->lib, entry->dev)) {
    SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETPOS, 0, 0);
    SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETPOS, 0, 0);
    ShowWindow(GetDlgItem(hwnd, IDC_ROW_SPIN), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_COL_SPIN), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_PORTL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_PORT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEEDL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEED), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRASTL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRAST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESS), SW_HIDE);
    EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_TEST), FALSE);
    return;
  }
  
  SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETPOS, 0, g_editDevice->GetHeight());
  SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETPOS, 0, g_editDevice->GetWidth());
  if (g_editDevice->HasSetSize()) {
    EnableWindow(GetDlgItem(hwnd, IDC_ROW), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_COL), TRUE);
    ShowWindow(GetDlgItem(hwnd, IDC_ROW_SPIN), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_COL_SPIN), SW_SHOW);
  }
  else {
    EnableWindow(GetDlgItem(hwnd, IDC_ROW), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_COL), FALSE);
    ShowWindow(GetDlgItem(hwnd, IDC_ROW_SPIN), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_COL_SPIN), SW_HIDE);
  }

  switch (g_editDevice->GetPortType()) {
  case DisplayDevice::portNONE:
    ShowWindow(GetDlgItem(hwnd, IDC_PORTL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_PORT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEEDL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEED), SW_HIDE);
    break;
  case DisplayDevice::portSERIAL:
    {
      HWND combo = GetDlgItem(hwnd, IDC_PORT);
      SendMessage(combo, CB_RESETCONTENT, 0, 0);
      int selidx = 0;
      for (int i = 1; i < 20; i++) {
        char dev[8];
        sprintf(dev, "COM%d", i);
        COMMCONFIG cfg;
        DWORD dwSize = sizeof(cfg);
        if (!GetDefaultCommConfig(dev, &cfg, &dwSize))
          continue;
        int idx = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)dev);
        if (!strcmp(dev, g_editDevice->GetPort()))
          selidx = idx;
      }
      SendMessage(combo, CB_SETCURSEL, selidx, 0);
    }
    {
      static const int speeds[] = {
        CBR_1200, CBR_2400, CBR_4800, CBR_9600, 
        CBR_19200, CBR_57600, CBR_115200
      };
      HWND combo = GetDlgItem(hwnd, IDC_SPEED);
      SendMessage(combo, CB_RESETCONTENT, 0, 0);
      int selidx = 0;
      for (int i = 1; i < countof(speeds); i++) {
        char speed[8];
        sprintf(speed, "%d", speeds[i]);
        int idx = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)speed);
        SendMessage(combo, CB_SETITEMDATA, idx, (LPARAM)speeds[i]);
        if (speeds[i] == g_editDevice->GetPortSpeed())
          selidx = idx;
      }
      SendMessage(combo, CB_SETCURSEL, selidx, 0);
    }
    ShowWindow(GetDlgItem(hwnd, IDC_PORTL), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_PORT), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEEDL), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEED), SW_SHOW);
    break;
  case DisplayDevice::portPARALLEL:
    {
      HWND combo = GetDlgItem(hwnd, IDC_PORT);
      SendMessage(combo, CB_RESETCONTENT, 0, 0);
      int selidx = 0;
      for (int i = 1; i <= 3; i++) {
        char dev[8];
        sprintf(dev, "LPT%d", i);
        int idx = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)dev);
        if (!strcmp(dev, g_editDevice->GetPort()))
          selidx = idx;
      }
      SendMessage(combo, CB_SETCURSEL, selidx, 0);
    }
    ShowWindow(GetDlgItem(hwnd, IDC_PORTL), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_PORT), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEEDL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEED), SW_HIDE);
    break;
  }

  if (g_editDevice->HasContrast()) {
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRASTL), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRAST), SW_SHOW);
    SendMessage(GetDlgItem(hwnd, IDC_CONTRAST), TBM_SETPOS, 
                TRUE, g_editDevice->GetContrast());
  }
  else {
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRASTL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRAST), SW_HIDE);
  }
  if (g_editDevice->HasBrightness()) {
    char trans[256];
    SF.i18n_translate(g_editDevice->HasBacklight() ? "Backlight:" : "Brightness:", 
                      trans, sizeof(trans));
    SetWindowText(GetDlgItem(hwnd, IDC_BRIGHTNESSL), trans);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSL), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESS), SW_SHOW);
    SendMessage(GetDlgItem(hwnd, IDC_BRIGHTNESS), TBM_SETPOS, 
                TRUE, g_editDevice->GetBrightness());
  }
  else {
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESS), SW_HIDE);
  }

  if (g_editDevice->HasKeypad()) {
    ShowWindow(GetDlgItem(hwnd, IDC_INPUT), SW_SHOW);
    SendMessage(GetDlgItem(hwnd, IDC_INPUT), BM_SETCHECK, 
                g_editDevice->GetEnableInput(), 0);
  }
  else {
    ShowWindow(GetDlgItem(hwnd, IDC_INPUT), SW_HIDE);
  }

  EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_TEST), TRUE);
}

// Copy settings controls into device.
static void SaveConfigSettings(HWND hwnd, BOOL persist)
{
  if (NULL == g_editDevice) return;
  
  if (g_editDevice->HasSetSize()) {
    int rows = SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_GETPOS, 0, 0);
    int cols = SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_GETPOS, 0, 0);
    g_editDevice->SetSize(cols, rows);
  }

  switch (g_editDevice->GetPortType()) {
  case DisplayDevice::portSERIAL:
    {
      HWND combo = GetDlgItem(hwnd, IDC_SPEED);
      int idx = SendMessage(combo, CB_GETCURSEL, 0, 0);
      int speed = SendMessage(combo, CB_GETITEMDATA, idx, 0);
      g_editDevice->SetPortSpeed(speed);
    }
  /* falls through */
  case DisplayDevice::portPARALLEL:
    {
      HWND combo = GetDlgItem(hwnd, IDC_PORT);
      int idx = SendMessage(combo, CB_GETCURSEL, 0, 0);
      char port[8];
      SendMessage(combo, CB_GETLBTEXT, idx, (LPARAM)port);
      g_editDevice->SetPort(port);
    }
    break;
  }

  if (g_editDevice->HasContrast()) {
    g_editDevice->SetContrast(SendMessage(GetDlgItem(hwnd, IDC_CONTRAST), 
                                       TBM_GETPOS, 0, 0));
  }
  if (g_editDevice->HasBrightness()) {
    g_editDevice->SetBrightness(SendMessage(GetDlgItem(hwnd, IDC_BRIGHTNESS), 
                                         TBM_GETPOS, 0, 0));
  }

  if (g_editDevice->HasKeypad()) {
    g_editDevice->SetEnableInput(SendMessage(GetDlgItem(hwnd, IDC_INPUT), 
                                             BM_GETCHECK, 0, 0));
  }
  
  if (persist) {
    HKEY hkey = DisplayDevice::GetSettingsKey();
    int idx = SendMessage(GetDlgItem(hwnd, IDC_DEVICE), CB_GETCURSEL, 0, 0);
    DisplayDeviceEntry *entry = (DisplayDeviceEntry *)
      SendMessage(GetDlgItem(hwnd, IDC_DEVICE), CB_GETITEMDATA, idx, 0);
    if (NULL != entry) {
      DisplayDevice::SetSettingString(hkey, "Library", entry->lib);
      DisplayDevice::SetSettingString(hkey, "Device", entry->dev);
    }
    g_editDevice->SaveSettings(hkey);
    RegCloseKey(hkey);
  }
}

static BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT uMsg, 
                                      WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      g_configDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PLUGIN)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.i18n_translate("Device:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_DEVICEL), trans);

      SF.i18n_translate("Rows:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_ROWL), trans);

      SF.i18n_translate("Columns:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_COLL), trans);

      SF.i18n_translate("Port:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_PORTL), trans);

      SF.i18n_translate("Speed:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_SPEEDL), trans);

      SF.i18n_translate("Contrast:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_CONTRASTL), trans);

      SF.i18n_translate("Test...", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_TEST), trans);

      SF.i18n_translate("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.i18n_translate("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      SF.i18n_translate("Apply", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      HKEY hkey = DisplayDevice::GetSettingsKey();
  
      LPCSTR libname = NULL;
      LPCSTR devname = NULL;

      char libbuf[MAX_PATH], devbuf[256];
      if (DisplayDevice::GetSettingString(hkey, "Library", libbuf, sizeof(libbuf)))
        libname = libbuf;
      if (DisplayDevice::GetSettingString(hkey, "Device", devbuf, sizeof(devbuf)))
        devname = devbuf;
  
      RegCloseKey(hkey);

      HWND combo = GetDlgItem(hwnd, IDC_DEVICE);
      int selidx = 0;
      for (size_t i = 0; i < countof(DisplayDevices); i++) {
        DisplayDeviceEntry *entry = DisplayDevices+i;
        int idx = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)entry->desc);
        SendMessage(combo, CB_SETITEMDATA, idx, (LPARAM)entry);
        if (((NULL != libname) && !_stricmp(libname, entry->lib)) &&
            ((NULL == devname) || (NULL == entry->dev) ||
             !strcmp(devname, entry->dev)))
          selidx = idx;
      }
      SendMessage(combo, CB_SETCURSEL, selidx, 0);
      LoadConfigSettings(hwnd);
      
      SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(4, 0));
      SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(40, 0));

      SendMessage(GetDlgItem(hwnd, IDC_CONTRAST), TBM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(0, 100));
      SendMessage(GetDlgItem(hwnd, IDC_BRIGHTNESS), TBM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(0, 100));

      return FALSE;
    }

  case WM_DESTROY: 
    if (NULL != g_editDevice) {
      DisplayDevice::Destroy(g_editDevice, g_editDevlib);
    }
    PostQuitMessage(0); 
    return FALSE;

  case WM_CLOSE: 
    EndDialog(hwnd, FALSE);
    return TRUE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      DisplayEnterCS();
      SaveConfigSettings(hwnd, TRUE);
      DisplayReopen(g_editDevice, g_editDevlib);
      DisplayLeaveCS();
      EndDialog(hwnd, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return TRUE;

    case IDC_APPLY:
      DisplayEnterCS();
      SaveConfigSettings(hwnd, TRUE);
      DisplayClose();
      DisplayLeaveCS();
      return TRUE;

    case IDC_TEST:
      DisplayEnterCS();
      SaveConfigSettings(hwnd, FALSE);
      DisplayClose();
      if (!g_editDevice->Open()) {
        DisplayLeaveCS();
        MessageBox(hwnd, "Cannot open device", "Test", MB_OK | MB_ICONERROR);
        return TRUE;
      }
      g_editDevice->Test();
      MessageBox(hwnd, "Test screen should be displayed", "Test", MB_OK);
      g_editDevice->Clear();
      g_editDevice->Close();
      DisplayLeaveCS();
      return TRUE;

    case IDC_DEVICE:
      if (HIWORD(wParam) == CBN_SELENDOK) {
        LoadConfigSettings(hwnd);
      }
      break;

    case IDC_ROW:
    case IDC_COL:
      if (HIWORD(wParam) == EN_CHANGE) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_PORT:
    case IDC_SPEED:
      if (HIWORD(wParam) == CBN_SELENDOK) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;
    }
    break;

  case WM_HSCROLL:
    EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
    break;
  }
  return FALSE;
}

static DWORD WINAPI ConfigThread(LPVOID lpParam)
{
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CONFIG), NULL,
                               ConfigDialogProc, (LPARAM)lpParam);
  g_configDialog = NULL;
  return 0;
}

void OpenConfigUI()
{
  DWORD dwThreadId;
  if (NULL != g_configDialog) {
    SetForegroundWindow(g_configDialog);
  }
  else {
    g_configThread = CreateThread(NULL, 0, ConfigThread, NULL, 0, &dwThreadId);
    if (NULL == g_configThread)
      MessageBox(0, "Cannot create dialog thread.", "Error", MB_OK);
  }
}

void CloseConfigUI()
{
  if (NULL != g_configDialog) {
    SendMessage(g_configDialog, WM_DESTROY, 0, 0);
    WaitForSingleObject(g_configThread, 5000);
    CloseHandle(g_configThread);
  }
}

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
    SendMessage(GetDlgItem(hwnd, IDC_VALBOOL), BM_SETCHECK, g_editCommand->bvalue1, 0);
    if (valINT == valueType)
      SetWindowText(GetDlgItem(hwnd, IDC_VALINT), g_editCommand->svalue1);
    else
      SetWindowText(GetDlgItem(hwnd, IDC_VALSTR), g_editCommand->svalue1);
  }
}

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
    SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETPOS, 0, 
                (IDC_ROW == ctlid) ? g_editCommand->ivalue1 : 0);
    SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETPOS, 0, 
                (IDC_ROW == ctlid) ? g_editCommand->ivalue2 : 0);
    BOOL wrap = (IDC_ROW == ctlid) && (g_editCommand->ivalue2 < 0);
    SendMessage(GetDlgItem(hwnd, IDC_USE_WRAP), BM_SETCHECK, wrap, 0);
    SendMessage(GetDlgItem(hwnd, IDC_USE_COL), BM_SETCHECK, !wrap, 0);
    SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_SETPOS, 0, g_editCommand->ivalue3);
    EnableWindow(GetDlgItem(hwnd, IDC_COL), !wrap);
    EnableWindow(GetDlgItem(hwnd, IDC_COL_SPIN), !wrap);
    EnableWindow(GetDlgItem(hwnd, IDC_USE_REST), !wrap);
    EnableWindow(GetDlgItem(hwnd, IDC_USE_WIDTH), !wrap);
    BOOL rest = (wrap || (IDC_ROW != ctlid) || (g_editCommand->ivalue3 <= 0));
    SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_SETCHECK, rest, 0);
    SendMessage(GetDlgItem(hwnd, IDC_USE_WIDTH), BM_SETCHECK, !rest, 0);
    EnableWindow(GetDlgItem(hwnd, IDC_WIDTH), !rest);
    EnableWindow(GetDlgItem(hwnd, IDC_WIDTH_SPIN), !rest);
  }

  sw = (IDC_GPO == ctlid) ? SW_SHOW : SW_HIDE;
  ShowWindow(GetDlgItem(hwnd, IDC_GPOL), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_GPO), sw);
  ShowWindow(GetDlgItem(hwnd, IDC_GPO_SPIN), sw);
  if (reload)
    SendMessage(GetDlgItem(hwnd, IDC_GPO_SPIN), UDM_SETPOS, 0, 
                (IDC_GPO == ctlid) ? g_editCommand->ivalue1 : 0);
}

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
      SendMessage(GetDlgItem(hwnd, IDC_ENABLE_LINE1 + i), BM_SETCHECK, 
                  !show || !(g_editCommand->ivalue1 & (1 << i)),
                  0);
      SendMessage(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i), BM_SETCHECK, 
                  show && !!(g_editCommand->ivalue2 & (1 << i)),
                  0);

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

static void LoadCommandSettings(HWND hwnd)
{
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);

  if (NULL == g_editCommand) {
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_SETCURSEL, CB_ERR, 0);
    ShowCommandInputs(hwnd, NULL, TRUE);
    return;
  }

  EnterCriticalSection(&g_editCommand->critical_section);

  DisplayAction *action = FindDisplayAction(g_editCommand);
  if (NULL == action)
    action = DisplayActions;    // String

  for (int idx = 0; idx < countof(DisplayActions); idx++) {
    DisplayAction *itemAction = (DisplayAction *)
      SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETITEMDATA, idx, 0);
    if (itemAction == action) {
      SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_SETCURSEL, idx, 0);      
      break;
    }
  }
  
  ShowCommandInputs(hwnd, action, TRUE);

  LeaveCriticalSection(&g_editCommand->critical_section);
}

static BOOL SaveCommandSettings(HWND hwnd)
{
  char buf[1024];

  if (g_editCommand == NULL)
     return FALSE;

  EnterCriticalSection(&g_editCommand->critical_section);

  int idx = SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETCURSEL, 0, 0);
  DisplayAction *action = (DisplayAction *)
    SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETITEMDATA, idx, 0);
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

      if (!SendMessage(GetDlgItem(hwnd, IDC_ENABLE_LINE1 + i), BM_GETCHECK, 0, 0))
        g_editCommand->ivalue1 |= (1 << i);
      if (SendMessage(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i), BM_GETCHECK, 0, 0))
        g_editCommand->ivalue2 |= (1 << i);

      GetWindowText(GetDlgItem(hwnd, IDC_LINE1 + i), pval, sizeof(buf) - (pval - buf));
      pval += strlen(pval);
    }
    SF.realloc_pchar(&g_editCommand->svalue1, buf);
  }
  else if (DisplayGPO == action->function) {
    g_editCommand->bvalue1 = SendMessage(GetDlgItem(hwnd, IDC_VALBOOL), BM_GETCHECK, 0, 0);
    g_editCommand->ivalue1 = SendMessage(GetDlgItem(hwnd, IDC_GPO_SPIN), UDM_GETPOS, 0, 0);
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

    g_editCommand->bvalue1 = SendMessage(GetDlgItem(hwnd, IDC_VALBOOL), BM_GETCHECK, 0, 0);

    g_editCommand->ivalue1 = SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_GETPOS, 0, 0);
    if (SendMessage(GetDlgItem(hwnd, IDC_USE_WRAP), BM_GETCHECK, 0, 0))
      g_editCommand->ivalue2 = -1;
    else
      g_editCommand->ivalue2 = SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_GETPOS, 0, 0);
    if (SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_GETCHECK, 0, 0))
      g_editCommand->ivalue3 = -1;
    else
      g_editCommand->ivalue3 = SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_GETPOS, 0, 0);
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
        int idx = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)action->name);
        SendMessage(combo, CB_SETITEMDATA, idx, (LPARAM)action);
      }

      int nrows = DisplayHeight();
      int ncols = DisplayWidth();

      SendMessage(GetDlgItem(hwnd, IDC_ROW_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(nrows-1, 0));
      SendMessage(GetDlgItem(hwnd, IDC_COL_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(ncols-1, 0));
      SendMessage(GetDlgItem(hwnd, IDC_WIDTH_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(ncols, 1));
      SendMessage(GetDlgItem(hwnd, IDC_GPO_SPIN), UDM_SETRANGE, 0, 
                  (LPARAM)MAKELONG(ngpos, 1));

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
        int idx = SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETCURSEL, 0, 0);
        DisplayAction *action = (DisplayAction *)
          SendMessage(GetDlgItem(hwnd, IDC_TYPE), CB_GETITEMDATA, idx, 0);
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
      SendMessage(GetDlgItem(hwnd, IDC_USE_REST), BM_SETCHECK, TRUE, 0);
      SendMessage(GetDlgItem(hwnd, IDC_USE_WIDTH), BM_SETCHECK, FALSE, 0);
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
        BOOL check = !SendMessage(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + off), 
                                  BM_GETCHECK, 0, 0);
        for (int i = 0; i < 4; i++) {
          SendMessage(GetDlgItem(hwnd, IDC_MARQUEE_LINE1 + i), BM_SETCHECK, 
                      ((i == off) & check), 0);
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

void UpdateCommandUI(p_command command)
{
  g_editCommand = command;
  if (NULL != g_commandDialog)
    SendMessage(g_commandDialog, WM_USER+100, 0, 0);
}

void CloseCommandUI()
{
  if (NULL != g_commandDialog) {
    SendMessage(g_commandDialog, WM_DESTROY, 0, 0);
    WaitForSingleObject(g_commandThread, 4000);
    CloseHandle(g_commandThread);
  }
}
