/* Girder entry functions. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

HINSTANCE g_hInstance;
s_functions SF;

static BOOL GetField(LPCSTR file, ID3_FrameID frame, ID3_FieldID field,
                     ID3_FieldID keyfld, LPCSTR keyval,
                     PCHAR buf, size_t buflen)
{
  static ID3_Tag tag;
  if (strcmp(file, tag.GetFileName())) {
    tag.Clear();
    tag.Link(file);
  }
  
  ID3_Frame *pframe;
  if (ID3FN_NOFIELD == keyfld)
    pframe = tag.Find(frame);
  else
    pframe = tag.Find(frame, keyfld, keyval);
  if (NULL == pframe) return FALSE;

  ID3_Field *pfield = pframe->GetField(field);
  if (NULL == pfield) return FALSE;
  
  pfield->Get(buf, buflen);
  return TRUE;
}

extern "C" void WINAPI
gir_version(PCHAR buffer, BYTE length)
{
  strncpy(buffer, PLUGINVERSION, length);
}

extern "C" void WINAPI
gir_name(PCHAR buffer, BYTE length)
{
  strncpy(buffer, PLUGINNAME, length);
}

extern "C" void WINAPI
gir_description(PCHAR buffer, BYTE length)
{
  strncpy(buffer, "Girder ID3 plugin", length);
}

extern "C" int WINAPI
gir_devicenum()
{
  return PLUGINNUM;
}

extern "C" int WINAPI
gir_requested_api(int maxapi)
{
  return 1;
}

extern "C" int WINAPI
gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions p)
{
  if (p->size != sizeof(SF)) {
    return GIR_FALSE;
  }
  memcpy(&SF, p, p->size);
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_close()
{
  CloseCommandUI();
  return GIR_TRUE;
}

#if 0
extern "C" void WINAPI
gir_config()
{
}

extern "C" int WINAPI
gir_start()
{
}

extern "C" int WINAPI
gir_stop()
{
}

extern "C" int WINAPI
gir_compare(PCHAR orig, PCHAR recv)
{
  // Unlikely to need this.
  return strcmp(orig, recv);
}

extern "C" int WINAPI
gir_learn_event(char *oldevent, char *newevent, int len)
{
  return GIR_FALSE;
}
#endif

extern "C" int WINAPI
gir_event(p_command command, 
          char *eventString, void *payload, int len,
          char *status, int statuslen)
{
  char buf[1024];
  SF.parse_reg_string(command->svalue2, buf, sizeof(buf));
  if (GetField(buf, 
               (ID3_FrameID)command->ivalue1, (ID3_FieldID)command->ivalue2,
               (ID3_FieldID)command->ivalue3, command->svalue3,
               buf, sizeof(buf))) {
    SF.set_string_var(command->svalue1, buf);
    strncpy(status, "Field found", statuslen);
  }
  else {
    SF.delete_var(command->svalue1);
    strncpy(status, "Field not found", statuslen);
  }

  return retContinue;
}

extern "C" void WINAPI
gir_command_gui()
{
  OpenCommandUI();
}

extern "C" void WINAPI
gir_command_changed(p_command command)
{
  UpdateCommandUI(command);
}

#if 0
extern "C" int WINAPI
gir_info(int message, int wparam, int lparam)
{
  return GIR_TRUE;
}
#endif

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
