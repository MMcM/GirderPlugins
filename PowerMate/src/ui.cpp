/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

#include <dui.h>

#define POWERMATE_DUI_GUID  "{9685E046-28DA-45A2-865B-6FDE6514459F}"
#define REQUEST_PAGE_GUID   "{9685E047-28DA-45A2-865B-6FDE6514459F}"
#define REPEAT_PAGE_GUID    "{9685E048-28DA-45A2-865B-6FDE6514459F}"

PFTree g_DUI;
PFTreeNode g_RequestPage, g_RepeatPage;
PFTreeNode g_RequestPageActive, g_RepeatPageActive;

void * WINAPI
RequestPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                    int val1, int val2, void *userdata)
{
  return NULL;
}

void * WINAPI
RepeatPageCallback(pLuaRec lua, PFTree tree, PFTreeNode node, PBaseNode baseNode,
                   int val1, int val2, void *userdata)
{
  return NULL;
}

BOOL DUIOpen()
{
  char path[MAX_PATH];
  strcpy(path, SF.CoreVars->ExePath);
  strcat(path, "plugins\\UI\\PowerMate.xml");
  g_DUI = LoadDUI(path);
  if (NULL == g_DUI) {
    GirderLogMessageEx(PLUGINNAME, "Could not open DUI File (PowerMate.xml).",
                       GLM_ERROR_ICON);
    return FALSE;
  }
  
  g_RequestPage = FindNodeS(REQUEST_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_RequestPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the Request Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_RequestPage, RequestPageCallback, TRUE);

  g_RepeatPage = FindNodeS(REPEAT_PAGE_GUID, g_DUI, NULL);
  if (NULL == g_RepeatPage) {	
    GirderLogMessageEx(PLUGINNAME, "Could not find the Repeat Page in PowerMate.xml.", 
                       GLM_ERROR_ICON);
    return FALSE;
  }
  SetCallbacks(g_DUI, g_RepeatPage, RepeatPageCallback, TRUE);

  return TRUE;
}

void DUIClose()
{
  DeleteFTree(g_DUI);
  g_DUI = NULL;
  g_RequestPage = g_RepeatPage = NULL;
}

void DUIOpenConfig(PFTree tree)
{
}

void DUICloseConfig(PFTree tree)
{
}

static sGroups GROUP = 
  // {9685E045-28DA-45A2-865B-6FDE6514459F}
  { {0x9685E045, 0x28DA, 0x45A2, {0x86, 0X5B, 0x6F, 0xDE, 0x65, 0x14, 0x45, 0x9F} }, 
    "PowerMate" };

void DUIOpenCommand(PFTree tree)
{
  g_RequestPageActive = InsertDUIPage(tree, g_DUI, g_RequestPage,
                                      &GROUP.PageGUID, &GROUP);
  g_RepeatPageActive = InsertDUIPage(tree, g_DUI, g_RepeatPage,
                                     &GROUP.PageGUID, &GROUP);
}

void DUICloseCommand(PFTree tree)
{
  RemoveDUIPageS(tree, REQUEST_PAGE_GUID);
  g_RequestPageActive = NULL;
  RemoveDUIPageS(tree, REPEAT_PAGE_GUID);
  g_RepeatPageActive = NULL;
}
