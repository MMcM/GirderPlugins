/* Girder entry functions. */

#include "stdafx.h"
#include "plugin.h"

s_functions SF;
p_command g_CurCommand = NULL;
jobject g_jplugin = NULL;

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
  strncpy(buffer, "Girder Java plugin", length);
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

  JNIEnv *jenv = getJNIEnv();
  if (NULL == jenv)
    return GIR_FALSE;

  jobject jplugin = createPlugin(jenv);
  if (!checkException(jenv))
    return GIR_FALSE;
  g_jplugin = jenv->NewGlobalRef(jplugin);
  jenv->DeleteLocalRef(jplugin);
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_close()
{
  JNIEnv *jenv = getJNIEnv();
  jenv->CallVoidMethod(g_jplugin, g_pluginCloseMID);
  jenv->DeleteGlobalRef(g_jplugin);
  g_jplugin = NULL;
  if (!checkException(jenv))
    return GIR_FALSE;
  // TODO: Delete the JVM here?
  return GIR_TRUE;
}

extern "C" void WINAPI
gir_config()
{
  // TODO: Would it be better to do this in Windows or Java?  Does it
  // deal with the registry or Java state?
  JNIEnv *jenv = getJNIEnv();
  jenv->CallVoidMethod(g_jplugin, g_pluginConfigMID);
  checkException(jenv);
}

extern "C" int WINAPI
gir_start()
{
  JNIEnv *jenv = getJNIEnv();
  jenv->CallVoidMethod(g_jplugin, g_pluginStartMID);
  if (!checkException(jenv))
    // TODO: Or is it better to always return true?
    return GIR_FALSE;
  else
    return GIR_TRUE;
}

extern "C" int WINAPI
gir_stop()
{
  JNIEnv *jenv = getJNIEnv();
  jenv->CallVoidMethod(g_jplugin, g_pluginStopMID);
  if (!checkException(jenv))
    // TODO: Or is it better to always return true?
    return GIR_FALSE;
  else
    return GIR_TRUE;
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
  JNIEnv *jenv = getJNIEnv();
  jstring jold = jstringFromNative(jenv, oldevent);
  jstring jnew = (jstring)jenv->CallObjectMethod(g_jplugin, g_pluginLearnMID, jold);
  if (!checkException(jenv))
    return GIR_FALSE;
  jenv->DeleteLocalRef(jold);
  if (NULL != jnew) {
    PCHAR nstr = jstringToNative(jenv, jnew);
    jenv->DeleteLocalRef(jnew);
    strncpy(newevent, nstr, len);
    SF.gir_free(nstr);
  }
  // TODO: Provision for modeless?  return GIR_ASYNC_LEARN and use sendEvent.
  return GIR_TRUE;
}

extern "C" int WINAPI
gir_event(p_command command, 
          char *eventString, void *payload, int len,
          char *status, int statuslen)
{
  JNIEnv *jenv = getJNIEnv();
  jobject jcmd = createCommand(jenv, command);
  jobject jevent = createEvent(jenv, eventString, payload, len);
  jobjectArray jstatusp = jenv->NewObjectArray(1, g_stringClass, NULL);
  int result = jenv->CallIntMethod(g_jplugin, g_pluginCommandMID, jcmd, jevent, jstatusp);
  jstring jstatus = (jstring)jenv->GetObjectArrayElement(jstatusp, 0);
  jenv->DeleteLocalRef(jcmd);
  jenv->DeleteLocalRef(jevent);
  jenv->DeleteLocalRef(jstatusp);

  if (!checkException(jenv))
    return retStopProcessing;

  if (NULL != jstatus) {
    PCHAR str = jstringToNative(jenv, jstatus);
    jenv->DeleteLocalRef(jstatus);
    strncpy(status, str, statuslen);
    SF.gir_free(str);
  }
  
  return result;
}

extern "C" void WINAPI
gir_command_gui()
{
  JNIEnv *jenv = getJNIEnv();
  jenv->CallVoidMethod(g_jplugin, g_pluginCommandGUIMID);
  checkException(jenv);
}

extern "C" void WINAPI
gir_command_changed(p_command command)
{
  g_CurCommand = command;         // For save() check.

  JNIEnv *jenv = getJNIEnv();
  jobject jcmd = createCommand(jenv, command);
  jenv->CallVoidMethod(g_jplugin, g_pluginCommandChangedMID, jcmd);
  jenv->DeleteLocalRef(jcmd);
  checkException(jenv);
}

extern "C" int WINAPI
gir_info(int message, int wparam, int lparam)
{
  // TODO: Is any of this needed?
  return GIR_TRUE;
}

/* Called by windows */
BOOL WINAPI DllMain( HANDLE hModule, DWORD dwReason,  LPVOID lpReserved )
{
  switch(dwReason) {
  case DLL_PROCESS_ATTACH:
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
