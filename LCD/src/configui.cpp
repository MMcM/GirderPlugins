/* Config (display device settings) user interface. 
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
  { "Matrix Orbital LK204-25-PC (20x4, keypad)", "MOLCD", "LK204PC" },
  { "Matrix Orbital LK202-24-USB / MX2 (20x2, keypad, USB)", "MOLCD", "LK202U" },
  { "Matrix Orbital LK204-24-USB / MX4 (20x4, keypad, USB)", "MOLCD", "LK204U" },
  { "Matrix Orbital LK402-12 (40x2, keypad)", "MOLCD", "LK402" },
  { "Matrix Orbital LK404-55 (40x4, keypad)", "MOLCD", "LK404" },
  { "Matrix Orbital VFD2021 (20x2, vfd)", "MOLCD", "VFD2021" }, // discontinued 7/20/00
  { "Matrix Orbital VFD2041 (20x4, vfd)", "MOLCD", "VFD2041" },
  { "Matrix Orbital VK202-25 (20x2, vfd, keypad)", "MOLCD", "VK202" },
  { "Matrix Orbital VK204-25 (20x4, vfd, keypad)", "MOLCD", "VK204" },
  { "Matrix Orbital VK202-24-USB / MX3 (20x2, vfd, keypad, USB)", "MOLCD", "VK202U" },
  { "Matrix Orbital VK204-24-USB / MX5 (20x4, vfd, keypad, USB)", "MOLCD", "VK204U" },
  { "Futaba VFD (20x4)", "FutabaVFD", "M204" },
  { "Futaba VFD (40x2)", "FutabaVFD", "M402" },
  { "HD44780 (parallel)", "PARALCD", NULL }
};

static DisplayDeviceEntry *FindDeviceEntry(DisplayDevice *dev)
{
  if (NULL == dev)
    return NULL;

  LPCSTR devlib = dev->GetFactory()->GetName();
  LPCSTR devtype = dev->GetDeviceType();
  for (size_t i = 0; i < countof(DisplayDevices); i++) {
    DisplayDeviceEntry *entry = DisplayDevices+i;
    if (!strcmp(devlib, entry->lib) &&
        ((NULL == devtype) || (NULL == entry->dev) || !strcmp(devtype, entry->dev))) {
      return entry;
    }
  }
  return NULL;
}

/* Local variables */
static DisplayDeviceList g_editDevices;
static DisplayDevice *g_editOrigDevice = NULL;
static DisplayDevice *g_editDevice = NULL;

static HANDLE g_configThread = NULL;
static HWND g_configDialog = NULL;

// Copy settings from edited device into controls.
static void LoadDeviceSettings(HWND hwnd)
{
  if (NULL == g_editDevice) {
    UpDown_SetPos(GetDlgItem(hwnd, IDC_ROW_SPIN), 0);
    UpDown_SetPos(GetDlgItem(hwnd, IDC_COL_SPIN), 0);
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
  
  UpDown_SetPos(GetDlgItem(hwnd, IDC_ROW_SPIN), g_editDevice->GetHeight());
  UpDown_SetPos(GetDlgItem(hwnd, IDC_COL_SPIN), g_editDevice->GetWidth());
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
      ComboBox_ResetContent(combo);
      int selidx = 0;
      for (int i = 1; i < 20; i++) {
        char dev[8];
        sprintf(dev, "COM%d", i);
        COMMCONFIG cfg;
        DWORD dwSize = sizeof(cfg);
        if (!GetDefaultCommConfig(dev, &cfg, &dwSize))
          continue;
        int idx = ComboBox_AddString(combo, dev);
        if (!strcmp(dev, g_editDevice->GetPort()))
          selidx = idx;
      }
      ComboBox_SetCurSel(combo, selidx);
    }
    {
      static const int speeds[] = {
        CBR_1200, CBR_2400, CBR_4800, CBR_9600, 
        CBR_19200, CBR_57600, CBR_115200
      };
      HWND combo = GetDlgItem(hwnd, IDC_SPEED);
      ComboBox_ResetContent(combo);
      int selidx = 0;
      for (int i = 1; i < countof(speeds); i++) {
        char speed[8];
        sprintf(speed, "%d", speeds[i]);
        int idx = ComboBox_AddString(combo, speed);
        ComboBox_SetItemData(combo, idx, speeds[i]);
        if (speeds[i] == g_editDevice->GetPortSpeed())
          selidx = idx;
      }
      ComboBox_SetCurSel(combo, selidx);
    }
    ShowWindow(GetDlgItem(hwnd, IDC_PORTL), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_PORT), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEEDL), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEED), SW_SHOW);
    break;
  case DisplayDevice::portPARALLEL:
    {
      HWND combo = GetDlgItem(hwnd, IDC_PORT);
      ComboBox_ResetContent(combo);
      int selidx = 0;
      for (int i = 1; i <= 3; i++) {
        char dev[8];
        sprintf(dev, "LPT%d", i);
        int idx = ComboBox_AddString(combo, dev);
        if (!strcmp(dev, g_editDevice->GetPort()))
          selidx = idx;
      }
      ComboBox_SetCurSel(combo, selidx);
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
    TrackBar_SetPos(GetDlgItem(hwnd, IDC_CONTRAST), 
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
    TrackBar_SetPos(GetDlgItem(hwnd, IDC_BRIGHTNESS), 
                    TRUE, g_editDevice->GetBrightness());
  }
  else {
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESS), SW_HIDE);
  }

  if (g_editDevice->HasKeypad()) {
    ShowWindow(GetDlgItem(hwnd, IDC_INPUT), SW_SHOW);
    Button_SetCheck(GetDlgItem(hwnd, IDC_INPUT), g_editDevice->GetEnableInput());
  }
  else {
    ShowWindow(GetDlgItem(hwnd, IDC_INPUT), SW_HIDE);
  }

  EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_TEST), TRUE);
}

// Copy settings controls into device.
static void SaveDeviceSettings(HWND hwnd, BOOL updateDevices, BOOL continueEdit)
{
  if (NULL != g_editDevice) {

    if (g_editDevice->HasSetSize()) {
      int rows = UpDown_GetPos(GetDlgItem(hwnd, IDC_ROW_SPIN));
      int cols = UpDown_GetPos(GetDlgItem(hwnd, IDC_COL_SPIN));
      g_editDevice->SetSize(cols, rows);
    }

    switch (g_editDevice->GetPortType()) {
    case DisplayDevice::portSERIAL:
      {
        HWND combo = GetDlgItem(hwnd, IDC_SPEED);
        int idx = ComboBox_GetCurSel(combo);
        int speed = ComboBox_GetItemData(combo, idx);
        g_editDevice->SetPortSpeed(speed);
      }
      /* falls through */
    case DisplayDevice::portPARALLEL:
      {
        HWND combo = GetDlgItem(hwnd, IDC_PORT);
        int idx = ComboBox_GetCurSel(combo);
        char port[8];
        ComboBox_GetLBText(combo, idx, port);
        g_editDevice->SetPort(port);
      }
      break;
    }

    if (g_editDevice->HasContrast()) {
      g_editDevice->SetContrast(TrackBar_GetPos(GetDlgItem(hwnd, IDC_CONTRAST)));
    }
    if (g_editDevice->HasBrightness()) {
      g_editDevice->SetBrightness(TrackBar_GetPos(GetDlgItem(hwnd, IDC_BRIGHTNESS)));
    }

    if (g_editDevice->HasKeypad()) {
      g_editDevice->SetEnableInput(Button_GetCheck(GetDlgItem(hwnd, IDC_INPUT)));
    }

  }
  
  if (updateDevices) {
    g_editDevices.Replace(g_editOrigDevice, g_editDevice);
    g_editOrigDevice = g_editDevice;
    g_editDevice = NULL;
    if (TRUE)
      g_editDevices.SaveToRegistry();
  }
  if (continueEdit) {
    if (NULL != g_editOrigDevice)
      g_editDevice = g_editOrigDevice->Duplicate();
  }
}

// Create new edit device and set fresh state from it.
static void CreateEditDevice(HWND hwnd)
{
  if (NULL != g_editDevice) {
    delete g_editDevice;
    g_editDevice = NULL;
  }

  int idx = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_DEVICE));
  DisplayDeviceEntry *entry = (DisplayDeviceEntry *)
    ComboBox_GetItemData(GetDlgItem(hwnd, IDC_DEVICE), idx);
  if (NULL != entry) {
    DisplayDeviceFactory *fact = DisplayDeviceFactory::GetFactory(entry->lib);
    if (NULL != fact) {
      LPCSTR name = NULL;
      g_editDevice = fact->CreateDisplayDevice(entry->dev);
      // TODO: Set name from name control.
    }
  }

  LoadDeviceSettings(hwnd);
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

      DisplayDeviceEntry *selent = NULL;
      if (NULL != g_editDevice)
        selent = FindDeviceEntry(g_editDevice);
      
      HWND combo = GetDlgItem(hwnd, IDC_DEVICE);
      int selidx = 0;
      for (size_t i = 0; i < countof(DisplayDevices); i++) {
        DisplayDeviceEntry *entry = DisplayDevices+i;
        int idx = ComboBox_AddString(combo, entry->desc);
        ComboBox_SetItemData(combo, idx, entry);
        if (entry == selent)
          selidx = idx;
      }
      ComboBox_SetCurSel(combo, selidx);
      
      UpDown_SetRange(GetDlgItem(hwnd, IDC_ROW_SPIN), 4, 0);
      UpDown_SetRange(GetDlgItem(hwnd, IDC_COL_SPIN), 40, 0);

      TrackBar_SetRange(GetDlgItem(hwnd, IDC_CONTRAST), 0, 100);
      TrackBar_SetRange(GetDlgItem(hwnd, IDC_BRIGHTNESS), 0, 100);

      LoadDeviceSettings(hwnd);

      return FALSE;
    }

  case WM_DESTROY: 
    if (NULL != g_editDevice) {
      delete g_editDevice;
      g_editDevice = NULL;
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
      DisplayClose();
      SaveDeviceSettings(hwnd, TRUE, FALSE);
      DisplayLeaveCS();
      EndDialog(hwnd, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return TRUE;

    case IDC_APPLY:
      DisplayEnterCS();
      DisplayClose();
      SaveDeviceSettings(hwnd, TRUE, TRUE);
      DisplayLeaveCS();
      return TRUE;

    case IDC_TEST:
      DisplayEnterCS();
      DisplayClose();
      SaveDeviceSettings(hwnd, FALSE, FALSE);
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
        CreateEditDevice(hwnd);
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
  g_editDevices.LoadFromRegistry();
  g_editOrigDevice = g_editDevices.GetDefault();
  g_editDevice = (NULL == g_editOrigDevice) ? NULL : g_editOrigDevice->Duplicate();
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CONFIG), NULL,
                               ConfigDialogProc, (LPARAM)lpParam);
  g_configDialog = NULL;
  g_editDevices.Clear();
  return 0;
}

// Plugin request for Settings dialog.
void OpenConfigUI()
{
  // TODO: Close timing window.
  if (NULL != g_configDialog) {
    SetForegroundWindow(g_configDialog);
  }
  else {
    if (NULL != g_configThread)
      CloseHandle(g_configThread);
    DWORD dwThreadId;
    g_configThread = CreateThread(NULL, 0, ConfigThread, NULL, 0, &dwThreadId);
    if (NULL == g_configThread)
      MessageBox(0, "Cannot create dialog thread.", "Error", MB_OK);
  }
}

// Plugin request to close any Settings dialog.
void CloseConfigUI()
{
  if (NULL != g_configDialog)
    SendMessage(g_configDialog, WM_DESTROY, 0, 0);
  if (NULL != g_configThread) {
    WaitForSingleObject(g_configThread, 5000);
    CloseHandle(g_configThread);
    g_configThread = NULL;
  }
}
