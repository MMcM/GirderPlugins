/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

PFTree g_DUI;

BOOL DUIOpen()
{
  PCHAR buf = PathExpand("%GIRDER%Plugins\\UI\\MBM.xml");
  g_DUI = LoadDUI(buf);
  SafeFree(buf);
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
  LockTree(tree, TreeLockWrite);
  MergeDUITrees(tree, g_DUI, duOnHookConfig);
  UnlockTree(tree, TreeLockWrite);
}

void DUICloseConfig(PFTree tree)
{
  LockTree(tree, TreeLockWrite);
  UnmergeDUITrees(tree, g_DUI, duOnUnHookConfig);
  UnlockTree(tree, TreeLockWrite);
}
