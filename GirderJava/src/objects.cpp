/* Native method implementations. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

#include "nl_girder_api_GirderCommand.h"
#include "nl_girder_api_GirderLink.h"
#include "nl_girder_api_GirderSupport.h"

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderCommand_getName(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return jstringFromNative(jenv, cmd->name);
}

JNIEXPORT jint JNICALL 
Java_nl_girder_api_GirderCommand_getActionType(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->actiontype;
}

JNIEXPORT jint JNICALL 
Java_nl_girder_api_GirderCommand_getActionSubType(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->actionsubtype;
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setActionSubType(JNIEnv *jenv, jobject jthis, jint jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->actionsubtype = jval;
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderCommand_getStringValue1(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return jstringFromNative(jenv, cmd->svalue1);
}

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderCommand_getStringValue2(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return jstringFromNative(jenv, cmd->svalue2);
}

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderCommand_getStringValue3(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return jstringFromNative(jenv, cmd->svalue3);
}

JNIEXPORT jboolean JNICALL 
Java_nl_girder_api_GirderCommand_getBooleanValue1(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->bvalue1;
}

JNIEXPORT jboolean JNICALL 
Java_nl_girder_api_GirderCommand_getBooleanValue2(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->bvalue2;
}

JNIEXPORT jboolean JNICALL 
Java_nl_girder_api_GirderCommand_getBooleanValue3(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->bvalue3;
}

JNIEXPORT jint JNICALL 
Java_nl_girder_api_GirderCommand_getIntegerValue1(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->ivalue1;
}

JNIEXPORT jint JNICALL 
Java_nl_girder_api_GirderCommand_getIntegerValue2(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->ivalue2;
}

JNIEXPORT jint JNICALL 
Java_nl_girder_api_GirderCommand_getIntegerValue3(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return cmd->ivalue3;
}

JNIEXPORT jobject JNICALL 
Java_nl_girder_api_GirderCommand_getLinkValue1(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return createLink(jenv, cmd->lvalue1);
}

JNIEXPORT jobject JNICALL 
Java_nl_girder_api_GirderCommand_getLinkValue2(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return createLink(jenv, cmd->lvalue2);
}

JNIEXPORT jobject JNICALL 
Java_nl_girder_api_GirderCommand_getLinkValue3(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  return createLink(jenv, cmd->lvalue3);
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setStringValue1(JNIEnv *jenv, jobject jthis, 
                                                 jstring jval)
{
  p_command cmd = getCommand(jenv, jthis);
  PCHAR val = jstringToNative(jenv, jval);
  EnterCriticalSection(&cmd->critical_section);
#if 0
  SF.gir_free(cmd->svalue1);
  cmd->svalue1 = val;
#else
  SF.realloc_pchar(&cmd->svalue1, val);
  SF.gir_free(val);
#endif
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setStringValue2(JNIEnv *jenv, jobject jthis, 
                                                 jstring jval)
{
  p_command cmd = getCommand(jenv, jthis);
  PCHAR val = jstringToNative(jenv, jval);
  EnterCriticalSection(&cmd->critical_section);
#if 0
  SF.gir_free(cmd->svalue2);
  cmd->svalue2 = val;
#else
  SF.realloc_pchar(&cmd->svalue2, val);
  SF.gir_free(val);
#endif
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setStringValue3(JNIEnv *jenv, jobject jthis, 
                                                 jstring jval)
{
  p_command cmd = getCommand(jenv, jthis);
  PCHAR val = jstringToNative(jenv, jval);
  EnterCriticalSection(&cmd->critical_section);
#if 0
  SF.gir_free(cmd->svalue3);
  cmd->svalue3 = val;
#else
  SF.realloc_pchar(&cmd->svalue3, val);
  SF.gir_free(val);
#endif
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setBooleanValue1(JNIEnv *jenv, jobject jthis, 
                                                  jboolean jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->bvalue1 = !!jval;
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setBooleanValue2(JNIEnv *jenv, jobject jthis, 
                                                  jboolean jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->bvalue2 = !!jval;
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setBooleanValue3(JNIEnv *jenv, jobject jthis, 
                                                  jboolean jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->bvalue3 = !!jval;
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setIntegerValue1(JNIEnv *jenv, jobject jthis, 
                                                  jint jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->ivalue1 = jval;
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setIntegerValue2(JNIEnv *jenv, jobject jthis, 
                                                  jint jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->ivalue2 = jval;
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setIntegerValue3(JNIEnv *jenv, jobject jthis, 
                                                  jint jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->ivalue3 = jval;
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setLinkValue1(JNIEnv *jenv, jobject jthis, 
                                               jobject jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->lvalue1 = getLink(jenv, jval);
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setLinkValue2(JNIEnv *jenv, jobject jthis, 
                                               jobject jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->lvalue2 = getLink(jenv, jval);
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_setLinkValue3(JNIEnv *jenv, jobject jthis, 
                                               jobject jval)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  cmd->lvalue3 = getLink(jenv, jval);
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderCommand_save(JNIEnv *jenv, jobject jthis)
{
  p_command cmd = getCommand(jenv, jthis);
  EnterCriticalSection(&cmd->critical_section);
  if (cmd == g_CurCommand) {
    cmd->actiontype = PLUGINNUM;
    SF.set_command(cmd);
  }
  // TODO: else what?  an error?
  LeaveCriticalSection(&cmd->critical_section);  
}

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderLink_getName(JNIEnv *jenv, jobject jthis)
{
  int link = getLink(jenv, jthis);
  if (0 == link)
    return NULL;
  else {
    char buf[256];
    SF.get_link_name(link, buf, sizeof(buf));
    return jstringFromNative(jenv, buf);
  }
}

JNIEXPORT jint JNICALL 
Java_nl_girder_api_GirderLink_trigger(JNIEnv *jenv, jobject jthis)
{
  int link = getLink(jenv, jthis);
  return (jint)SF.trigger_command(link);
}

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderSupport_parseRegString(JNIEnv *jenv, jobject jthis, 
                                                jstring jval)
{
  PCHAR value = jstringToNative(jenv, jval);
  if (NULL == value)
    return NULL;
  
  char buf[1024];
  SF.parse_reg_string(value, buf, sizeof(buf));
  SF.gir_free(value);
  return jstringFromNative(jenv, buf);
}

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderSupport_i18nTranslate(JNIEnv *jenv, jobject jthis, 
                                               jstring jval)
{
  PCHAR value = jstringToNative(jenv, jval);
  if (NULL == value)
    return NULL;
  
  char buf[1024];
  SF.i18n_translate(value, buf, sizeof(buf));
  SF.gir_free(value);
  return jstringFromNative(jenv, buf);
}

JNIEXPORT jobject JNICALL 
Java_nl_girder_api_GirderSupport_pickLink(JNIEnv *jenv, jobject jthis, jobject jdef)
{
  int dlink = getLink(jenv, jdef);
  int nlink = SF.treepicker_show(SF.parent_hwnd, dlink);
  if (nlink < 0)
    return jdef;                // Cancelled.
  else
    return createLink(jenv, nlink);
}

JNIEXPORT jint JNICALL 
Java_nl_girder_api_GirderSupport_getIntegerVariable(JNIEnv *jenv, jobject jthis, 
                                                    jstring jname)
{
  PCHAR name = jstringToNative(jenv, jname);
  int result = SF.get_int_var(name);
  SF.gir_free(name);
  return result;
}

JNIEXPORT jdouble JNICALL 
Java_nl_girder_api_GirderSupport_getDoubleVariable(JNIEnv *jenv, jobject jthis, 
                                                   jstring jname)
{
  PCHAR name = jstringToNative(jenv, jname);
  double result = SF.get_double_var(name);
  SF.gir_free(name);
  return result;
}

JNIEXPORT jstring JNICALL 
Java_nl_girder_api_GirderSupport_getStringVariable(JNIEnv *jenv, jobject jthis, 
                                                   jstring jname)
{
  PCHAR name = jstringToNative(jenv, jname);
  char buf[1024];
  SF.get_string_var(name, buf, sizeof(buf));
  SF.gir_free(name);
  return jstringFromNative(jenv, buf);
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderSupport_setIntegerVariable(JNIEnv *jenv, jobject jthis, 
                                                    jstring jname, jint jval)
{
  PCHAR name = jstringToNative(jenv, jname);
  SF.set_int_var(name, jval);
  SF.gir_free(name);
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderSupport_setDoubleVariable(JNIEnv *jenv, jobject jthis, 
                                                   jstring jname, jdouble jval)
{
  PCHAR name = jstringToNative(jenv, jname);
  SF.set_double_var(name, jval);
  SF.gir_free(name);
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderSupport_setStringVariable(JNIEnv *jenv, jobject jthis, 
                                                   jstring jname, jstring jval)
{
  PCHAR name = jstringToNative(jenv, jname);
  PCHAR value = jstringToNative(jenv, jval);
  SF.set_string_var(name, value);
  SF.gir_free(name);
  SF.gir_free(value);
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderSupport_deleteVariable(JNIEnv *jenv, jobject jthis, 
                                                jstring jname)
{
  PCHAR name = jstringToNative(jenv, jname);
  SF.delete_var(name);
  SF.gir_free(name);
}

JNIEXPORT void JNICALL 
Java_nl_girder_api_GirderSupport_sendEvent(JNIEnv *jenv, jobject jthis, 
                                           jobject jevent)
{
  void *payload;
  int len;
  PCHAR event = getEvent(jenv, jevent, payload, len);
  SF.send_event(event, payload, len, PLUGINNUM);
  SF.gir_free(event);
  if (NULL != payload)
    SF.gir_free(payload);
}
