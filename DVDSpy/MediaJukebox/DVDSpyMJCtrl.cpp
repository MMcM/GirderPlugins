// DVDSpyMJCtrl.cpp : Implementation of CDVDSpyMJCtrl
// $Header$

#include "stdafx.h"
#include "DVDSpyMJ.h"
#include "DVDSpyMJCtrl.h"
#include "DVDSpyMJEvent.h"

HWND g_hwndDVDSpy = NULL;

BOOL CALLBACK FindMonitorWindow(HWND hwnd, LPARAM lparam)
{
  char cname[256];
  GetClassName(hwnd, cname, sizeof(cname));
  if (!strcmp(cname, "Girder DVDSpy Monitor Window")) {
    *(HWND*)lparam = hwnd;
    return FALSE;
  }
  return TRUE;                  // Keep trying.
}

/////////////////////////////////////////////////////////////////////////////
// CDVDSpyMJCtrl


CDVDSpyMJCtrl::CDVDSpyMJCtrl()
{
  m_bWindowOnly = TRUE;
  CalcExtent(m_sizeExtent);

  m_nRefreshInterval = 1000;
}

CDVDSpyMJCtrl::~CDVDSpyMJCtrl()
{
  if (NULL != g_hwndDVDSpy) {
    COPYDATASTRUCT cd;
    cd.dwData = 0;
    cd.lpData = "MediaJukebox.Close\0";
    cd.cbData = 20;
    SendMessage(g_hwndDVDSpy, WM_COPYDATA, 0, (LPARAM)&cd);
  }
}

STDMETHODIMP CDVDSpyMJCtrl::Init(LPDISPATCH pDisp)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState())

  m_pMJ = pDisp;

  char szRegistryPath[1024];
  strcpy(szRegistryPath, REGISTRY_PATH_MJ_PLUGINS_INTERFACE);
  strcat(szRegistryPath, "DVDSpy");
  CRegKey reg;
  if (ERROR_SUCCESS == reg.Create(HKEY_CURRENT_USER, szRegistryPath)) {
    DWORD dwVal, dwType, dwSize;
    dwSize = sizeof(dwVal);
    if ((ERROR_SUCCESS == RegQueryValueEx(reg, "RefreshInterval", NULL,
                                          &dwType, (LPBYTE)&dwVal, &dwSize)) &&
        (REG_DWORD == dwType)) {
      m_nRefreshInterval = dwVal;
    }
  }

  CString sRefreshInterval;
  sRefreshInterval.Format("%d", m_nRefreshInterval);
  ::SetWindowText(GetDlgItem(IDC_INTERVAL), sRefreshInterval);

  Reset();

  return S_OK;
}

STDMETHODIMP CDVDSpyMJCtrl::get_RefreshInterval(short *pVal)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState())

  *pVal = (short)m_nRefreshInterval;

  return S_OK;
}

STDMETHODIMP CDVDSpyMJCtrl::put_RefreshInterval(short newVal)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState())

  m_nRefreshInterval = newVal;

  char szRegistryPath[1024];
  strcpy(szRegistryPath, REGISTRY_PATH_MJ_PLUGINS_INTERFACE);
  strcat(szRegistryPath, "DVDSpy");
  CRegKey reg;
  if (ERROR_SUCCESS == reg.Create(HKEY_CURRENT_USER, szRegistryPath)) {		
    reg.SetValue(m_nRefreshInterval, "RefreshInterval");
  }

  return S_OK;
}

const UINT IDT_REFRESH = 1;

STDMETHODIMP CDVDSpyMJCtrl::Reset()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState())

  KillTimer(IDT_REFRESH);

  EnumWindows(FindMonitorWindow, (LPARAM)&g_hwndDVDSpy);
  // TODO: Consider start the timer anyway and having it check for the spy.
  if (NULL != g_hwndDVDSpy)
    SetTimer(IDT_REFRESH, m_nRefreshInterval);

  return S_OK;
}

LRESULT CDVDSpyMJCtrl::OnClickedReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  char szRefreshInterval[32];
  ::GetWindowText(GetDlgItem(IDC_INTERVAL), szRefreshInterval, sizeof(szRefreshInterval));

  int nRefreshInterval = atoi(szRefreshInterval);
  if (nRefreshInterval != m_nRefreshInterval)
    put_RefreshInterval((short)nRefreshInterval);

  Reset();
  return 0;
}

LRESULT CDVDSpyMJCtrl::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  // TODO: Consider an IsWindow check here.
  DVDSpyMJDoEvents(m_pMJ, g_hwndDVDSpy);

  return TRUE;
}
