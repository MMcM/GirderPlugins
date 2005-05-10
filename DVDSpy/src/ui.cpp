/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

#include <dui.h>

static sGroups GROUP = 
  // {0AA4874A-2EF5-447e-A7DD-AF4DFEBECE50}
  { { 0xaa4874a, 0x2ef5, 0x447e, { 0xa7, 0xdd, 0xaf, 0x4d, 0xfe, 0xbe, 0xce, 0x50 } },
    "DVDSpy" };

#define MBM_DUI_GUID  "{0AA4874B-2EF5-447e-A7DD-AF4DFEBECE50}"

PFTree g_DUI;

BOOL DUIOpen()
{
  char path[MAX_PATH];
  strcpy(path, SF.CoreVars->ExePath);
  strcat(path, "plugins\\UI\\DVDSpy.xml");
  g_DUI = LoadDUI(path);
  if (NULL == g_DUI) {
    GirderLogMessageEx(PLUGINNAME, "Could not open DUI File (DVDSpy.xml).",
                       GLM_ERROR_ICON);
    return FALSE;
  }

  return TRUE;
}

void DUIClose()
{
  DeleteFTree(g_DUI);
  g_DUI = NULL;
}

void DUIOpenConfig(PFTree tree)
{
}

void DUICloseConfig(PFTree tree)
{
}

void DUIOpenCommand(PFTree tree)
{
}

void DUICloseCommand(PFTree tree)
{
}
