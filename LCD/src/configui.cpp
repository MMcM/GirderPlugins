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
static HWND g_displaysDialog = NULL, g_displayPropertySheet = NULL;
static HIMAGELIST g_hChecks = NULL;

const WPARAM PSQS_NEW_DEVICE = 1;
const WPARAM PSQS_SAVE_FOR_TEST = 2;

// Enable Apply button.
static void SetPageModified(HWND hwnd)
{
  PropSheet_Changed(GetParent(hwnd), hwnd);
}

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
    if ((NULL != g_editDevice) && 
        g_editDevice->IsDefault() && 
        (g_editDevice != g_editDevices.GetFirst()))
      // The General page was used to make this the default.  
      // Replace does not honor that flag, so move to the front.
      g_editDevices.SetDefault(g_editDevice);
    g_editOrigDevice = g_editDevice;
    g_editDevice = NULL;
    if (NULL == g_displaysDialog)
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

static void SaveGeneralSettings(HWND hwnd)
{
  if (NULL == g_editDevice) return;

  char name[128];
  Edit_GetText(GetDlgItem(hwnd, IDC_NAME), name, sizeof(name));
  g_editDevice->SetName(name);

  g_editDevice->SetEnabled(Button_GetCheck(GetDlgItem(hwnd, IDC_ENABLED)));
  g_editDevice->SetDefault(Button_GetCheck(GetDlgItem(hwnd, IDC_DEFAULT)));
}

static BOOL CALLBACK GeneralPageDialogProc(HWND hwnd, UINT uMsg, 
                                           WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      if (NULL != g_editDevice) {
        Edit_SetText(GetDlgItem(hwnd, IDC_NAME), g_editDevice->GetName());

        Button_SetCheck(GetDlgItem(hwnd, IDC_ENABLED), g_editDevice->IsEnabled());
        Button_SetCheck(GetDlgItem(hwnd, IDC_DEFAULT), g_editDevice->IsDefault());
        if (g_editDevice->IsDefault()) {
          Button_Enable(GetDlgItem(hwnd, IDC_ENABLED), FALSE);
          Button_Enable(GetDlgItem(hwnd, IDC_DEFAULT), FALSE);
        }
      }
      return FALSE;
    }

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDC_WIDTH:
      if (HIWORD(wParam) == EN_CHANGE) {
        SetPageModified(hwnd);
      }
      break;

    case IDC_ENABLED:
    case IDC_DEFAULT:
      SetPageModified(hwnd);
      break;
    }
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
        SaveGeneralSettings(hwnd);
        if ((NULL != g_editDevice) && g_editDevice->IsDefault()) {
          // Once this is saved, it cannot be turned off here.
          // Another device has to be turned on.
          Button_Enable(GetDlgItem(hwnd, IDC_ENABLED), FALSE);
          Button_Enable(GetDlgItem(hwnd, IDC_DEFAULT), FALSE);
        }
        OnApply(hwnd, lParam);
        break;
      }
    }
    break;

  case PSM_QUERYSIBLINGS:
    switch (wParam) {
    case PSQS_NEW_DEVICE:
      // Note that we do not pick up the name from a new device.
      break;
    case PSQS_SAVE_FOR_TEST:
      SaveGeneralSettings(hwnd);
      break;
    }
    break;

  }

  return FALSE;
}

static void FillGeneralPage(LPPROPSHEETPAGE ppsp)
{
  ppsp->dwSize = sizeof(PROPSHEETPAGE);
  ppsp->dwFlags = PSP_DEFAULT;
  ppsp->hInstance = g_hInstance;
  ppsp->pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
  ppsp->pfnDlgProc = GeneralPageDialogProc;
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
    Button_Enable(GetDlgItem(hwnd, IDC_TEST), FALSE);
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

  Button_Enable(GetDlgItem(hwnd, IDC_TEST), TRUE);
}

// Copy settings controls into device.
static void SaveDisplaySettings(HWND hwnd)
{
  if (NULL == g_editDevice) return;

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
      // Make a new device with defaults, ignoring all but the General state.
      DisplayDeviceFactory *fact = DisplayDeviceFactory::GetFactory(entry->lib);
      if (NULL != fact)
        g_editDevice = fact->CreateDisplayDevice(entry->dev);
      if ((NULL != g_editOrigDevice) && (NULL != g_editDevice)) {
        // If the General page has been active, it will overwrite these on apply.
        g_editDevice->SetName(g_editOrigDevice->GetName());
        g_editDevice->SetDefault(g_editOrigDevice->IsDefault());
        g_editDevice->SetEnabled(g_editOrigDevice->IsEnabled());
      }
    }
  }

  PropSheet_QuerySiblings(GetParent(hwnd), PSQS_NEW_DEVICE, (LPARAM)g_editDevice);
}

static BOOL CALLBACK DisplayPageDialogProc(HWND hwnd, UINT uMsg, 
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
  ppsp->pfnDlgProc = DisplayPageDialogProc;
}

static int PropertySheetCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
{
  switch (uMsg) {
  case PSCB_INITIALIZED:
    g_displayPropertySheet = hwnd;
    break;
  }
  return 0;
}

static void EditDevice(DisplayDevice *dev)
{
  g_editOrigDevice = dev;
  g_editDevice = (NULL == g_editOrigDevice) ? NULL : g_editOrigDevice->Duplicate();

  PROPSHEETPAGE pages[4];
  UINT nPages, nStartPage;
  nPages = 0;
  if (NULL != g_displaysDialog)
    FillGeneralPage(pages + nPages++);
  nStartPage = nPages;
  FillDisplayPage(pages + nPages++);

  PROPSHEETHEADERA pshead;
  pshead.dwSize = sizeof(pshead);
  pshead.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
  pshead.hwndParent = (NULL != g_displaysDialog) ? 
    g_displaysDialog : DisplayWindowParent();
  pshead.hInstance = g_hInstance;
  pshead.pszIcon = MAKEINTRESOURCE(IDI_PLUGIN);
  pshead.pszCaption = PLUGINNAME;
  pshead.nPages = nPages;
  pshead.nStartPage = nStartPage;
  pshead.ppsp = pages;
  pshead.pfnCallback = PropertySheetCallback;
  int result = PropertySheet(&pshead);
  g_displayPropertySheet = NULL;

  if (NULL != g_editDevice) {
    delete g_editDevice;
    g_editDevice = NULL;
  }
  if (!g_editDevices.Contains(g_editOrigDevice)) {
    delete g_editOrigDevice;
    g_editOrigDevice = NULL;
  }
}

inline LPARAM ListView_GetItemData(HWND hwnd, int nItem)
{
  LV_ITEM lvi;
  memset(&lvi, 0, sizeof(LVITEM));
  lvi.iItem = nItem;
  lvi.mask = LVIF_PARAM;
  ListView_GetItem(hwnd, &lvi);
  return lvi.lParam;
}

void UpdateButtonEnabling(HWND hwnd)
{
  HWND list = GetDlgItem(hwnd, IDC_DISPLAYS);
  DisplayDevice *dev = NULL;
  BOOL enable = FALSE;
  int nSelItem = ListView_GetNextItem(list, -1, LVNI_SELECTED);
  if (nSelItem >= 0) {
    enable = TRUE;
    dev = (DisplayDevice *)ListView_GetItemData(list, nSelItem);
  }
  Button_Enable(GetDlgItem(hwnd, IDC_EDIT), enable);
  Button_Enable(GetDlgItem(hwnd, IDC_REMOVE), enable);
  Button_Enable(GetDlgItem(hwnd, IDC_DEFAULT), enable && !dev->IsDefault());
}

static void FillDisplaysList(HWND hwnd)
{
  HWND list = GetDlgItem(hwnd, IDC_DISPLAYS);
  ListView_DeleteAllItems(list);
  LV_ITEM lvi;
  memset(&lvi, 0, sizeof(LVITEM));
  for (DisplayDevice *dev = g_editDevices.GetFirst(); NULL != dev; 
       dev = dev->GetNext()) {
    lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM | LVIS_SELECTED;
    lvi.iSubItem = 0;
    if (dev->IsDefault())
      lvi.state = INDEXTOSTATEIMAGEMASK(3);
    else if (dev->IsEnabled())
      lvi.state = INDEXTOSTATEIMAGEMASK(2);
    else
      lvi.state = INDEXTOSTATEIMAGEMASK(1);
    if (dev == g_editOrigDevice)
      lvi.state |= LVIS_SELECTED;
    lvi.stateMask = LVIS_STATEIMAGEMASK | LVIS_SELECTED;
    lvi.pszText = (LPSTR)dev->GetName();
    if (NULL == lvi.pszText)
      lvi.mask &= ~LVIF_TEXT;
    lvi.lParam = (LPARAM)dev;
    lvi.iItem = ListView_InsertItem(list, &lvi); // May move when sorted.

    DisplayDeviceEntry *entry = FindDeviceEntry(dev);
    if (NULL != entry) {
      lvi.mask = LVIF_TEXT;
      lvi.iSubItem = 1;
      lvi.pszText = (LPSTR)entry->desc;
      ListView_SetItem(list, &lvi);
    }
    lvi.iItem++;
  }
  UpdateButtonEnabling(hwnd);
}

static void EditSelectedDevice(HWND hwnd, BOOL copy)
{
  DisplayDevice *dev = NULL;
  {
    HWND list = GetDlgItem(hwnd, IDC_DISPLAYS);
    int nSelItem = ListView_GetNextItem(list, -1, LVNI_SELECTED);
    if (nSelItem >= 0)
      dev = (DisplayDevice *)ListView_GetItemData(list, nSelItem);
  }
  if (!copy)
    EditDevice(dev);
  else {
    DisplayDevice *ndev;
    char name[256];
    if (NULL != dev) {
      ndev = dev->Duplicate();
      ndev->SetDefault(FALSE);
      strncpy(name, ndev->GetName(), sizeof(name)-3);
    }
    else {
      DisplayDeviceFactory *fact = DisplayDeviceFactory::GetFactory(DisplayDevices->lib);
      if (NULL == fact) return;
      ndev = fact->CreateDisplayDevice(DisplayDevices->dev);
      if (NULL == ndev) return;
      if (NULL == g_editDevices.GetFirst()) ndev->SetDefault(TRUE);
      LoadString(g_hInstance, IDS_NEW_NAME, name, sizeof(name)-3);
    }
    // Add a number to the end to make a unique name.
    size_t idx = strlen(name);
    while ((idx > 0) && (NULL != strchr("0123456789", name[idx-1])))
      idx--;
    int n = atoi(name+idx);
    while (TRUE) {
      if (n > 0)
        sprintf(name+idx, "%d", n);
      if (NULL == g_editDevices.Get(name))
        break;                  // Name not in use.
      n++;
    }
    ndev->SetName(name);
    EditDevice(ndev);
  }
  FillDisplaysList(hwnd);  
}

static BOOL CALLBACK DisplaysDialogProc(HWND hwnd, UINT uMsg, 
                                        WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      g_displaysDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PLUGIN)));

      SetWindowText(hwnd, PLUGINNAME);

      HWND list = GetDlgItem(hwnd, IDC_DISPLAYS);

      if (NULL == g_hChecks)
        g_hChecks = ImageList_LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_CHECKS),
                                         16, 1, RGB(255, 0, 0));
      ListView_SetImageList(list, g_hChecks, LVSIL_STATE);

      char title[128];

      LV_COLUMN lvc;
      memset(&lvc, 0, sizeof(lvc));
      lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
      lvc.cx = 100;
      if (LoadString(g_hInstance, IDS_NAME, title, sizeof(title)))
        lvc.pszText = title;
      else
        lvc.pszText = "Name";
      lvc.iSubItem = 0;
      ListView_InsertColumn(list, 0, &lvc);
      lvc.cx = 225;
      if (LoadString(g_hInstance, IDS_TYPE, title, sizeof(title)))
        lvc.pszText = title;
      else
        lvc.pszText = "Type";
      lvc.iSubItem = 1;
      ListView_InsertColumn(list, 1, &lvc);

      FillDisplaysList(hwnd);

      UpdateButtonEnabling(hwnd);

      Button_Enable(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      return FALSE;
    }

  case WM_DESTROY: 
    PostQuitMessage(0); 
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      g_editDevices.SaveToRegistry();
      EndDialog(hwnd, TRUE);
      return TRUE;

    case IDC_APPLY:
      g_editDevices.SaveToRegistry();
      return TRUE;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return TRUE;

    case IDC_ADD:
      EditSelectedDevice(hwnd, TRUE);
      return TRUE;

    case IDC_EDIT:
      EditSelectedDevice(hwnd, FALSE);
      return TRUE;

    case IDC_REMOVE:
      {
        HWND list = GetDlgItem(hwnd, IDC_DISPLAYS);
        int nSelItem = -1;
        while (TRUE) {
          nSelItem = ListView_GetNextItem(list, nSelItem, LVNI_SELECTED);
          if (nSelItem < 0) break;
          DisplayDevice *dev = (DisplayDevice *)ListView_GetItemData(list, nSelItem);
          g_editDevices.Replace(dev, NULL);
        }
        g_editOrigDevice = g_editDevices.GetFirst();
        if (NULL != g_editOrigDevice)
          g_editOrigDevice->SetDefault(TRUE); // In case old default deleted.
        FillDisplaysList(hwnd);
      }
      return TRUE;

    case IDC_DEFAULT:
      {
        HWND list = GetDlgItem(hwnd, IDC_DISPLAYS);
        int nSelItem = ListView_GetNextItem(list, -1, LVNI_SELECTED);
        if (nSelItem < 0) return TRUE;
        g_editOrigDevice = (DisplayDevice *)ListView_GetItemData(list, nSelItem);
        g_editDevices.SetDefault(g_editOrigDevice);
        FillDisplaysList(hwnd);
      }
      return TRUE;
    }
    break;

  case WM_NOTIFY:
    {
      LPNMHDR phdr = (LPNMHDR)lParam;
      switch (phdr->code) {
      case NM_CLICK:
        {
          LPNMITEMACTIVATE pNMITEM = (LPNMITEMACTIVATE)phdr;
          HWND list = GetDlgItem(hwnd, IDC_DISPLAYS);
          LVHITTESTINFO hti;
          hti.pt = pNMITEM->ptAction;
          int nHitItem = ListView_HitTest(list, &hti);
          if (hti.flags & LVHT_ONITEMSTATEICON) {
            DisplayDevice *dev = (DisplayDevice *)ListView_GetItemData(list, nHitItem);
            UINT nState = ListView_GetItemState(list, nHitItem, LVIS_STATEIMAGEMASK);
            switch (nState) {
            case INDEXTOSTATEIMAGEMASK(1):
              dev->SetEnabled(TRUE);
              nState = INDEXTOSTATEIMAGEMASK(2);
              break;
            case INDEXTOSTATEIMAGEMASK(2):
              dev->SetEnabled(FALSE);
              nState = INDEXTOSTATEIMAGEMASK(1);
              break;
            }
            ListView_SetItemState(list, nHitItem, nState, LVIS_STATEIMAGEMASK);
          }
        }
        break;
      case NM_DBLCLK:
        EditSelectedDevice(hwnd, FALSE);
        break;
      case LVN_ITEMCHANGED:
        {
          LPNMLISTVIEW pNMLIST = (LPNMLISTVIEW)phdr;
          if ((pNMLIST->uNewState ^ pNMLIST->uOldState) & LVNI_SELECTED) {
            UpdateButtonEnabling(hwnd);
          }
        }
        break;
      }
    }
    break;
  }
  return FALSE;
}

static DWORD WINAPI ConfigThread(LPVOID lpParam)
{
  g_editDevices.LoadFromRegistry();

  DisplayDevice *defdev = g_editDevices.GetDefault();
  if ((NULL != defdev) && (NULL == defdev->GetName()))
    EditDevice(defdev);         // Simple single device edit mode.
  else {
    // Full multiple device edit mode.
    g_editOrigDevice = defdev;  // Initially select any default.
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DISPLAYS), DisplayWindowParent(), 
                   DisplaysDialogProc, (LPARAM)lpParam);
    g_displaysDialog = NULL;
  }
   
  g_editDevices.Clear();
  return 0;
}

// Plugin request for Settings dialog.
void OpenConfigUI()
{
  // TODO: I think there is a race condition here when thread is
  // starting or finishing
  if (NULL != g_displayPropertySheet)
    SetForegroundWindow(g_displayPropertySheet);
  else if (NULL != g_displaysDialog)
    SetForegroundWindow(g_displaysDialog);
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
  if (NULL != g_displayPropertySheet)
    PropSheet_PressButton(g_displayPropertySheet, PSBTN_CANCEL);
  if (NULL != g_configThread) {
    WaitForSingleObject(g_configThread, 5000);
    CloseHandle(g_configThread);
    g_configThread = NULL;
  }
}
