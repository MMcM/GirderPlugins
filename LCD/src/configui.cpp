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
static HWND g_configPropertySheet = NULL;

const WPARAM PSQS_NEW_DEVICE = 1;
const WPARAM PSQS_SAVE_FOR_TEST = 2;

// On XP, a single notification arrives at the property sheet for button pushes.
// On 2K/Me, all that is available is the apply notification to every active page.
// So we have to figure out which one is the last active one and do the actual update
// then.  On even older systems without the messages used here, we make it look
// like every page is the last page so that there are too many updates rather than
// too few.
static BOOL IsLastPage(HWND hwnd)
{
  HWND sheet = GetParent(hwnd);
  int index = PropSheet_HwndToIndex(sheet, hwnd);
  while (TRUE) {
    index++;
    if (NULL == PropSheet_IndexToPage(sheet, index))
      return TRUE;              // No following page.
    if (NULL != PropSheet_IndexToHwnd(sheet, index))
      return FALSE;             // Following page has window for notification.
  }
}

static void OnApply(HWND hwnd, LPARAM lParam)
{
  if (NULL == g_editDevice) return;
  if (IsLastPage(hwnd)) {
    // Update device list and then registry if no main dialog.
    g_editDevices.Replace(g_editOrigDevice, g_editDevice);
    g_editOrigDevice = g_editDevice;
    g_editDevice = NULL;
    if (TRUE)
      g_editDevices.SaveToRegistry();
    LPPSHNOTIFY pshn = (LPPSHNOTIFY)lParam;
    if (!pshn->lParam) {        // For Apply, need a new copy for further editing.
      if (NULL != g_editOrigDevice)
        g_editDevice = g_editOrigDevice->Duplicate();
    }
  }
}

static void OnTest(HWND hwnd)
{
  // Save to device but no further.
  PropSheet_QuerySiblings(GetParent(hwnd), PSQS_SAVE_FOR_TEST, 0L);

  DisplayEnterCS();
  DisplayClose();
  if (!g_editDevice->Open()) {
    DisplayLeaveCS();
    MessageBox(hwnd, "Cannot open device", "Test", MB_OK | MB_ICONERROR);
    return;
  }
  g_editDevice->Test();
  MessageBox(hwnd, "Test screen should be displayed", "Test", MB_OK);
  g_editDevice->Clear();
  g_editDevice->Close();
  DisplayLeaveCS();
}

// Enable Apply button.
static void SetPageModified(HWND hwnd)
{
  PropSheet_Changed(GetParent(hwnd), hwnd);
}

// Copy settings from edited device into controls.
static void LoadDisplaySettings(HWND hwnd)
{
  if (NULL == g_editDevice) {
    ShowWindow(GetDlgItem(hwnd, IDC_PORTL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_PORT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEEDL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_SPEED), SW_HIDE);
    UpDown_SetPos(GetDlgItem(hwnd, IDC_ROW_SPIN), 0);
    UpDown_SetPos(GetDlgItem(hwnd, IDC_COL_SPIN), 0);
    ShowWindow(GetDlgItem(hwnd, IDC_ROW_SPIN), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_COL_SPIN), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRASTL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_CONTRAST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_BRIGHTNESS), SW_HIDE);
    EnableWindow(GetDlgItem(hwnd, IDC_TEST), FALSE);
    return;
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
    char legend[256];
    if (LoadString(g_hInstance,
                   g_editDevice->HasBacklight() ? IDS_BACKLIGHT : IDS_BRIGHTNESS,
                   legend, sizeof(legend)))
      SetWindowText(GetDlgItem(hwnd, IDC_BRIGHTNESSL), legend);
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

  EnableWindow(GetDlgItem(hwnd, IDC_TEST), TRUE);
}

// Copy settings controls into device.
static void SaveDisplaySettings(HWND hwnd)
{
  if (NULL != g_editDevice) {

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

    if (g_editDevice->HasSetSize()) {
      int rows = UpDown_GetPos(GetDlgItem(hwnd, IDC_ROW_SPIN));
      int cols = UpDown_GetPos(GetDlgItem(hwnd, IDC_COL_SPIN));
      g_editDevice->SetSize(cols, rows);
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
}

// Create new edit device and set fresh state from it.
static void CreateEditDevice(HWND hwnd)
{
  int idx = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_DEVICE));
  DisplayDeviceEntry *entry = (DisplayDeviceEntry *)
    ComboBox_GetItemData(GetDlgItem(hwnd, IDC_DEVICE), idx);

  DisplayDeviceEntry *oentry = FindDeviceEntry(g_editDevice);

  if (entry == oentry) return;  // Unchanged.

  if (NULL != g_editDevice) {
    delete g_editDevice;
    g_editDevice = NULL;
  }

  if (NULL != entry) {
    if (entry == FindDeviceEntry(g_editOrigDevice))
      // Keep as much state as possible.
      g_editDevice = g_editOrigDevice->Duplicate();
    else {
      // Make a new device with its defaults, ignoring previous state.
      DisplayDeviceFactory *fact = DisplayDeviceFactory::GetFactory(entry->lib);
      if (NULL != fact) {
        LPCSTR name = NULL;
        g_editDevice = fact->CreateDisplayDevice(entry->dev);
      }
    }
  }

  PropSheet_QuerySiblings(GetParent(hwnd), PSQS_NEW_DEVICE, (LPARAM)g_editDevice);
}

static BOOL CALLBACK DisplayDialogProc(HWND hwnd, UINT uMsg, 
                                      WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
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

      LoadDisplaySettings(hwnd);

      return FALSE;
    }

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDC_TEST:
      OnTest(hwnd);
      return TRUE;

    case IDC_DEVICE:
      if (HIWORD(wParam) == CBN_SELENDOK) {
        CreateEditDevice(hwnd);
        SetPageModified(hwnd);
      }
      break;

    case IDC_PORT:
    case IDC_SPEED:
      if (HIWORD(wParam) == CBN_SELENDOK) {
        SetPageModified(hwnd);
      }
      break;

    case IDC_ROW:
    case IDC_COL:
      if (HIWORD(wParam) == EN_CHANGE) {
        SetPageModified(hwnd);
      }
      break;

    case IDC_INPUT:
      SetPageModified(hwnd);
      break;
    }
    break;

  case WM_HSCROLL:
    SetPageModified(hwnd);
    break;

  case WM_NOTIFY:
    {
      LPNMHDR phdr = (LPNMHDR)lParam;
      switch (phdr->code) {
      case PSN_SETACTIVE:
        break;
      case PSN_KILLACTIVE:
        break;
      case PSN_APPLY:
        SaveDisplaySettings(hwnd);
        OnApply(hwnd, lParam);
        break;
      }
    }
    break;

  case PSM_QUERYSIBLINGS:
    switch (wParam) {
    case PSQS_NEW_DEVICE:
      LoadDisplaySettings(hwnd);
      break;
    case PSQS_SAVE_FOR_TEST:
      SaveDisplaySettings(hwnd);
      break;
    }
    break;

  }

  return FALSE;
}

static void FillDisplayPage(LPPROPSHEETPAGE ppsp)
{
  ppsp->dwSize = sizeof(PROPSHEETPAGE);
  ppsp->dwFlags = PSP_DEFAULT;
  ppsp->hInstance = g_hInstance;
  ppsp->pszTemplate = MAKEINTRESOURCE(IDD_DISPLAY);
  ppsp->pfnDlgProc = DisplayDialogProc;
}

static int PropertySheetCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
{
  switch (uMsg) {
  case PSCB_INITIALIZED:
    g_configPropertySheet = hwnd;
    break;
  }
  return 0;
}

static DWORD WINAPI ConfigThread(LPVOID lpParam)
{
  g_editDevices.LoadFromRegistry();
  g_editOrigDevice = g_editDevices.GetDefault();
  g_editDevice = (NULL == g_editOrigDevice) ? NULL : g_editOrigDevice->Duplicate();

  PROPSHEETPAGE pages[4];
  UINT nPages, nStatePage;
  nPages = 0;
  nStatePage = nPages;
  FillDisplayPage(pages + nPages++);

  PROPSHEETHEADERA pshead;
  pshead.dwSize = sizeof(pshead);
  pshead.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
  pshead.hwndParent = DisplayWindowParent();
  pshead.hInstance = g_hInstance;
  pshead.pszIcon = MAKEINTRESOURCE(IDI_PLUGIN);
  pshead.pszCaption = "LCD 3.0";
  pshead.nPages = nPages;
  pshead.nStartPage = 0;
  pshead.ppsp = pages;
  pshead.pfnCallback = PropertySheetCallback;
  int result = PropertySheet(&pshead);
  g_configPropertySheet = NULL;

  if (NULL != g_editDevice) {
    delete g_editDevice;
    g_editDevice = NULL;
  }
  g_editDevices.Clear();
  return 0;
}

// Plugin request for Settings dialog.
void OpenConfigUI()
{
  // TODO: Close timing window.
  if (NULL != g_configPropertySheet) {
    SetForegroundWindow(g_configPropertySheet);
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
  if (NULL != g_configPropertySheet)
    PropSheet_PressButton(g_configPropertySheet, PSBTN_CANCEL);
  if (NULL != g_configThread) {
    WaitForSingleObject(g_configThread, 5000);
    CloseHandle(g_configThread);
    g_configThread = NULL;
  }
}
