/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

#define POWERMATE_GROUP_NAME "PowerMate"
#define POWERMATE_GROUP_GUID "{9685E045-28DA-45A2-865B-6FDE6514459F}"
#define POWERMATE_DUI_GUID   "{9685E046-28DA-45A2-865B-6FDE6514459F}"
#define REQUEST_PAGE_GUID    "{9685E047-28DA-45A2-865B-6FDE6514459F}"
#define REPEAT_PAGE_GUID     "{9685E048-28DA-45A2-865B-6FDE6514459F}"

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
