/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

#include <dui.h>

static sGroups GROUP = 
  // {661DD302-90C5-45e9-8B20-D979D5E3ADFA}
  { { 0x661dd302, 0x90c5, 0x45e9, { 0x8b, 0x20, 0xd9, 0x79, 0xd5, 0xe3, 0xad, 0xfa } },
    "MBM" };

#define MBM_DUI_GUID  "{661DD303-90C5-45e9-8B20-D979D5E3ADFA}"

PFTree g_DUI;

BOOL DUIOpen()
{
  char path[MAX_PATH];
  strcpy(path, SF.CoreVars->ExePath);
  strcat(path, "plugins\\UI\\MBM.xml");
  g_DUI = LoadDUI(path);
  if (NULL == g_DUI) {
    GirderLogMessageEx(PLUGINNAME, "Could not open DUI File (MBM.xml).",
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
