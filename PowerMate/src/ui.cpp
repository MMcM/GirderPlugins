/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

PFTree g_DUI;

BOOL DUIOpen()
{
  PCHAR buf = PathExpand("%GIRDER%Plugins\\UI\\PowerMate.xml");
  g_DUI = LoadDUI(buf);
  SafeFree(buf);
  if (NULL == g_DUI) {
    GirderLogMessageEx(PLUGINNAME, "Could not open DUI File (PowerMate.xml).",
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

void DUIOpenCommand(PFTree tree)
{
  LockTree(tree, TreeLockWrite);
  MergeDUITrees(tree, g_DUI, duOnHookCommand);
  UnlockTree(tree, TreeLockWrite);
}

void DUICloseCommand(PFTree tree)
{
  LockTree(tree, TreeLockWrite);
  UnmergeDUITrees(tree, g_DUI, duOnUnHookCommand);
  UnlockTree(tree, TreeLockWrite);
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
