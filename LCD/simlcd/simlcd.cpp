/* Simulated LCD device implementation 
$Header$
*/

#include "stdafx.h"
#include "resource.h"

HINSTANCE g_hInstance;
ATOM g_wndClass = NULL;

class SimulatedLCD : public DisplayDevice
{
public:
  SimulatedLCD(HWND parent, LPCSTR title);
  ~SimulatedLCD();
  
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual BOOL DeviceHasSetSize();
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

  void Resize(HWND hwnd = NULL);
  void OnClose();
  HBRUSH InitBackgroundBrush(HDC hDC);
  void PaintFixed(HDC hDC, int row, int col, LPCSTR str, int length);
  void OnPaint();
  void OnContextMenu(int x, int y);
  void OnChangeFont();
  void OnChangeColor();
  void OnChangeBackground();
  void OnMove(int x, int y);
  void OnToggleOnTop();

protected:
  HWND m_parent, m_hwnd;
  char m_title[32];
  int m_fontSize;
  char m_fontName[LF_FACESIZE], m_fontStyle[LF_FACESIZE];
  COLORREF m_textColor, m_backColor;
  LOGFONT m_logFont;
  int m_charWidth, m_lineHeight;
  BOOL m_fixedPitch;
  int m_xPos, m_yPos;
  BOOL m_onTop;
  HBRUSH m_bgBrush;
  HBITMAP m_customBitmap;
};

SimulatedLCD::SimulatedLCD(HWND parent, LPCSTR title)
{
  m_cols = 20;
  m_rows = 4;
  m_parent = parent;
  m_hwnd = NULL;
  if (NULL == title)
    title = "Simulated LCD";
  strncpy(m_title, title, sizeof(m_title));
  m_fontSize = 10;
  strncpy(m_fontName, "Courier New", sizeof(m_fontName));
  strncpy(m_fontStyle, "Bold", sizeof(m_fontStyle));
  m_textColor = RGB(0, 0, 0);
  m_backColor = RGB(240,240,40);
  memset(&m_logFont, 0, sizeof(m_logFont));
  m_xPos = m_yPos = -1;
  m_onTop = TRUE;
  m_bgBrush = NULL;
  m_customBitmap = NULL;
}

SimulatedLCD::~SimulatedLCD()
{
  if (NULL != m_bgBrush)
    DeleteObject(m_bgBrush);
  if (NULL != m_customBitmap)
    DeleteObject(m_customBitmap);
}

void SimulatedLCD::OnClose()
{
  m_hwnd = NULL;
  Close();
}

void SimulatedLCD::Resize(HWND hwnd)
{
  if (NULL == m_hwnd)
    m_hwnd = hwnd;
  HDC hDC = GetDC(m_hwnd);
  if ('\0' == m_logFont.lfFaceName[0]) {
    m_logFont.lfHeight = -MulDiv(m_fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    strcpy(m_logFont.lfFaceName, m_fontName);
    if (!strcmp(m_fontStyle, "Bold"))
      m_logFont.lfWeight = FW_BOLD;
  }
  HFONT hFont = CreateFontIndirect(&m_logFont);
  HFONT hOld = (HFONT)SelectObject(hDC, hFont);
  TEXTMETRIC tm;
  GetTextMetrics(hDC, &tm);
  SelectObject(hDC, hOld);
  ReleaseDC(m_hwnd, hDC);

  m_charWidth = tm.tmAveCharWidth;
  m_lineHeight = tm.tmHeight + tm.tmExternalLeading;
  m_fixedPitch = !(tm.tmPitchAndFamily & TMPF_FIXED_PITCH);

  RECT wrect, crect;
  GetWindowRect(m_hwnd, &wrect);
  GetClientRect(m_hwnd, &crect);
  LONG width = m_charWidth * m_cols + 
    (wrect.right - wrect.left) - (crect.right - crect.left);
  LONG height = m_lineHeight * m_rows +
    (wrect.bottom - wrect.top) - (crect.bottom - crect.top);
  if (m_xPos < 0)
    m_xPos = ((wrect.left + wrect.right) / 2) - (width / 2);
  if (m_yPos < 0)
    m_yPos = ((wrect.top + wrect.bottom) / 2) - (height / 2);
  MoveWindow(m_hwnd, m_xPos, m_yPos, width, height, TRUE);
}

HBRUSH SimulatedLCD::InitBackgroundBrush(HDC hDC)
{
  if (NULL == m_bgBrush) {
    LOGBRUSH lbrush;
    lbrush.lbStyle = 0;
    lbrush.lbColor = m_backColor;
    lbrush.lbHatch = 0;
    m_bgBrush = CreateBrushIndirect(&lbrush);
  }
  return m_bgBrush;
}

void SimulatedLCD::PaintFixed(HDC hDC, int row, int col, LPCSTR str, int length)
{
  HDC custDC = NULL;
  HBITMAP oldBitmap = NULL;
  while (length > 0) {
    int nch = 0;
    while (nch < length) {
      if ((unsigned char)str[nch] < NCUSTCHARS)
        break;
      nch++;
    }
    if (nch > 0) {
      // Next segment is normal characters.
      RECT rect;
      rect.left = col * m_charWidth;
      rect.top = row * m_lineHeight;
      rect.right = rect.left + nch * m_charWidth;
      rect.bottom = rect.top + m_lineHeight;
      DrawText(hDC, str, nch, &rect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
      col += nch;
      str += nch;
      length -= nch;
    }
    else {
      // A custom character.
      if (NULL == custDC) {
        if (NULL == m_customBitmap) {
          BYTE bits[NCUSTCHARS * NCUSTROWS];
          for (int i = 0; i < NCUSTCHARS; i++) {
            const BYTE *cbits = m_customCharacters[i].GetBits();
            for (int j = 0; j < NCUSTROWS; j++) {
              bits[j * NCUSTCHARS + i] = ~cbits[j]; // zero is foreground color
            }
          }
          m_customBitmap = CreateBitmap(NCUSTCHARS * 8, NCUSTROWS, 1, 1, bits);
        }
        custDC = CreateCompatibleDC(NULL);
        oldBitmap = (HBITMAP)SelectObject(custDC, m_customBitmap);
      }
      SetStretchBltMode(hDC, BLACKONWHITE);
      StretchBlt(hDC, col * m_charWidth, row * m_lineHeight, m_charWidth, m_lineHeight,
                 custDC, *str * 8 + (8 - NCUSTCOLS), 0, NCUSTCOLS + 1, NCUSTROWS, SRCCOPY);
      col++;
      str++;
      length--;
    }
  }
  if (NULL != custDC) {
    SelectObject(custDC, oldBitmap);
    DeleteDC(custDC);
  }
}

void SimulatedLCD::OnPaint()
{
  PAINTSTRUCT ps;
  HDC hDC = BeginPaint(m_hwnd, &ps);

  COLORREF oldText = SetTextColor(hDC, m_textColor);
  COLORREF oldBack = SetBkColor(hDC, m_backColor);
  HFONT hFont = CreateFontIndirect(&m_logFont);
  HFONT hOld = (HFONT)SelectObject(hDC, hFont);

  if (m_fixedPitch) {
    // Fixed pitch; do each line segment.
    PCHAR pb = (PCHAR)m_buffer;
    for (int i = 0; i < m_rows; i++) {
      PaintFixed(hDC, i, 0, pb, m_cols);
      pb += m_cols;
    }
  }
  else {
    // Variable pitch; don't get custom characters.
    RECT rect;
    GetClientRect(m_hwnd, &rect);
  
    PCHAR pb = (PCHAR)m_buffer;
    for (int i = 0; i < m_rows; i++) {
      rect.top += DrawText(hDC, pb, m_cols, &rect, 
                           DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
      pb += m_cols;
    }
  }
  
  SetTextColor(hDC, oldText);
  SetBkColor(hDC, oldBack);
  SelectObject(hDC, hOld);
  DeleteObject(hFont);

  EndPaint(m_hwnd, &ps);
}

void SimulatedLCD::OnContextMenu(int x, int y)
{ 
  HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU));
  if (NULL == hMenu) return;
 
  HMENU hMenuPopUp = GetSubMenu(hMenu, 0);
  if (NULL == hMenuPopUp) return;

  CheckMenuItem(hMenuPopUp, IDM_ALWAYSONTOP,
                MF_BYCOMMAND | ((m_onTop) ? MF_CHECKED : MF_UNCHECKED));

  TrackPopupMenu(hMenuPopUp, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, m_hwnd, NULL);

  DestroyMenu(hMenu); 
}

void SimulatedLCD::OnChangeFont()
{ 
  CHOOSEFONT cfont;
  memset(&cfont, 0, sizeof(cfont));
  cfont.lStructSize = sizeof(cfont);
  cfont.lpLogFont = &m_logFont;
  cfont.iPointSize = m_fontSize * 10;
  cfont.lpszStyle = m_fontStyle;
  cfont.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_USESTYLE;
  cfont.nFontType = SCREEN_FONTTYPE;
  if (ChooseFont(&cfont)) {
    strcpy(m_fontName, m_logFont.lfFaceName);
    m_fontSize = cfont.iPointSize / 10;
    Resize();
    InvalidateRect(m_hwnd, NULL, TRUE);

    HKEY hkey = GetSettingsKey();
    SetSettingInt(hkey, "SimLCDFontSize", m_fontSize);
    SetSettingString(hkey, "SimLCDFont", m_fontName);
    SetSettingString(hkey, "SimLCDFontStyle", m_fontStyle);
    RegCloseKey(hkey);
  }
}

static COLORREF custColors[16] = { 0 };

void SimulatedLCD::OnChangeColor()
{ 
  CHOOSECOLOR ccolor;
  memset(&ccolor, 0, sizeof(ccolor));
  ccolor.lStructSize = sizeof(ccolor);
  ccolor.hwndOwner = m_hwnd;
  ccolor.rgbResult = m_textColor;
  ccolor.lpCustColors = custColors;
  ccolor.Flags = CC_RGBINIT | CC_FULLOPEN;
  if (ChooseColor(&ccolor)) {
    m_textColor = ccolor.rgbResult;
    InvalidateRect(m_hwnd, NULL, TRUE);

    HKEY hkey = GetSettingsKey();
    SetSettingInt(hkey, "SimLCDTextColor", *(int*)&m_textColor);
    RegCloseKey(hkey);
  }
}

void SimulatedLCD::OnChangeBackground()
{ 
  CHOOSECOLOR ccolor;
  memset(&ccolor, 0, sizeof(ccolor));
  ccolor.lStructSize = sizeof(ccolor);
  ccolor.hwndOwner = m_hwnd;
  ccolor.rgbResult = m_backColor;
  ccolor.lpCustColors = custColors;
  ccolor.Flags = CC_RGBINIT | CC_FULLOPEN;
  if (ChooseColor(&ccolor)) {
    m_backColor = ccolor.rgbResult;
    DeleteObject(m_bgBrush);
    m_bgBrush = NULL;
    InvalidateRect(m_hwnd, NULL, TRUE);

    HKEY hkey = GetSettingsKey();
    SetSettingInt(hkey, "SimLCDBackColor", *(int*)&m_textColor);
    RegCloseKey(hkey);
  }
}

void SimulatedLCD::OnMove(int x, int y)
{
  m_xPos = x;
  m_yPos = y;
  HKEY hkey = GetSettingsKey();
  SetSettingInt(hkey, "SimLCDXPos", m_xPos);
  SetSettingInt(hkey, "SimLCDYPos", m_yPos);
  RegCloseKey(hkey);
}

void SimulatedLCD::OnToggleOnTop()
{
  m_onTop = !m_onTop;
  SetWindowPos(m_hwnd, (m_onTop) ? HWND_TOPMOST : HWND_NOTOPMOST,
               0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

  HKEY hkey = GetSettingsKey();
  SetSettingBool(hkey, "SimLCDAlwaysOnTop", m_onTop);
  RegCloseKey(hkey);
}

const int GWL_DEVICE = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  SimulatedLCD *device;
  if (WM_CREATE == nMsg) {
    device = (SimulatedLCD *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
    SetWindowLong(hWnd, GWL_DEVICE, (LONG)device);
  }
  else
    device = (SimulatedLCD *)GetWindowLong(hWnd, GWL_DEVICE);

  if (NULL != device) {
    switch (nMsg) {
    case WM_CREATE:
      device->Resize(hWnd);
      return 0;
    case WM_DESTROY:
      device->OnClose();
      return 0;
    case WM_ERASEBKGND:
      {
        HDC hDC = (HDC)wParam; 
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hDC, &rc, device->InitBackgroundBrush(hDC));
        return 1;
      }
      break;
    case WM_PAINT:
      device->OnPaint();
      return 0;
    case WM_MOVE:
      {
        // lParam is position of client area.  We want window position.
        RECT rect;
        GetWindowRect(hWnd, &rect);
        device->OnMove(rect.left, rect.top);
      }
      break;
    case WM_CONTEXTMENU:
      {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        RECT rect;
        GetClientRect(hWnd, &rect);
        POINT pt = { x, y };
        ScreenToClient(hWnd, &pt); 
        if (!PtInRect(&rect, pt))
          break;
        device->OnContextMenu(x, y);
        return 0;
      }
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDM_FONT:
        device->OnChangeFont();
        return 0;
      case IDM_COLOR:
        device->OnChangeColor();
        return 0;
      case IDM_BACKGROUND:
        device->OnChangeBackground();
        return 0;
      case IDM_ALWAYSONTOP:
        device->OnToggleOnTop();
        return 0;
      }
      break;
    }
  }
  return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

BOOL SimulatedLCD::DeviceOpen()
{
  if (NULL == g_wndClass) {
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 4;
    wc.hInstance = g_hInstance;
    wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PLUGIN));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "SIMLCD";
    g_wndClass = RegisterClass(&wc);
    if (NULL == g_wndClass)
      return FALSE;
  }

  DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_TOOLWINDOW;
  if (m_onTop)
    exStyle |= WS_EX_TOPMOST;
  m_hwnd = CreateWindowEx(exStyle, "SIMLCD", m_title, WS_OVERLAPPED | WS_SYSMENU,
                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                          m_parent, NULL, g_hInstance, this);
  if (NULL == m_hwnd)
    return FALSE;

  ShowWindow(m_hwnd, SW_SHOW);
  UpdateWindow(m_hwnd);

  return TRUE;
}

void SimulatedLCD::DeviceClose()
{
  if (NULL != m_hwnd) {
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;
  }
}

void SimulatedLCD::DeviceClear()
{
  InvalidateRect(m_hwnd, NULL, TRUE);
}

void SimulatedLCD::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  if (m_fixedPitch) {
    // Paint right away if fixed pitch.
    HDC hDC = GetDC(m_hwnd);
    COLORREF oldText = SetTextColor(hDC, m_textColor);
    COLORREF oldBack = SetBkColor(hDC, m_backColor);
    HFONT hFont = CreateFontIndirect(&m_logFont);
    HFONT hOld = (HFONT)SelectObject(hDC, hFont);
    PaintFixed(hDC, row, col, (LPCSTR)str, length);
    SetTextColor(hDC, oldText);
    SetBkColor(hDC, oldBack);
    SelectObject(hDC, hOld);
    DeleteObject(hFont);
    ReleaseDC(m_hwnd, hDC);
    return;
  }
  else {
    // Invalidate the whole row if variable pitch.
    RECT rect;
    GetClientRect(m_hwnd, &rect);
    rect.top = row * m_lineHeight;
    rect.bottom = rect.top + m_lineHeight;
    InvalidateRect(m_hwnd, &rect, TRUE);
  }
}

void SimulatedLCD::DeviceDefineCustomCharacter(int index, const CustomCharacter& cust)
{
  // Invalidate current source bitmap.
  if (NULL != m_customBitmap) {
    DeleteObject(m_customBitmap);
    m_customBitmap = NULL;
  }
}

BOOL SimulatedLCD::DeviceHasSetSize()
{
  return TRUE;
}

void SimulatedLCD::DeviceLoadSettings(HKEY hkey)
{
  GetSettingInt(hkey, "SimLCDFontSize", m_fontSize);
  GetSettingString(hkey, "SimLCDFont", m_fontName, sizeof(m_fontName));
  GetSettingString(hkey, "SimLCDFontStyle", m_fontStyle, sizeof(m_fontStyle));
  GetSettingInt(hkey, "SimLCDTextColor", *(int*)&m_textColor);
  GetSettingInt(hkey, "SimLCDBackColor", *(int*)&m_backColor);
  GetSettingInt(hkey, "SimLCDXPos", m_xPos);
  GetSettingInt(hkey, "SimLCDYPos", m_yPos);
  GetSettingBool(hkey, "SimLCDAlwaysOnTop", m_onTop);
}

void SimulatedLCD::DeviceSaveSettings(HKEY hkey)
{
  SetSettingInt(hkey, "SimLCDFontSize", m_fontSize);
  SetSettingString(hkey, "SimLCDFont", m_fontName);
  SetSettingString(hkey, "SimLCDFontStyle", m_fontStyle);
  SetSettingInt(hkey, "SimLCDTextColor", *(int*)&m_textColor);
  SetSettingInt(hkey, "SimLCDBackColor", *(int*)&m_backColor);
  SetSettingInt(hkey, "SimLCDXPos", m_xPos);
  SetSettingInt(hkey, "SimLCDYPos", m_yPos);
  SetSettingBool(hkey, "SimLCDAlwaysOnTop", m_onTop);
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(HWND parent, LPCSTR name)
{
  return new SimulatedLCD(parent, name);
}

/* Called by windows */
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason,  LPVOID lpReserved)
{
  switch(dwReason) {
  case DLL_PROCESS_ATTACH:
    g_hInstance = (HINSTANCE)hModule;
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}
