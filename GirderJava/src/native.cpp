/* Various conversions between Girder and Java handles and data. */

#include "stdafx.h"
#include "plugin.h"

jclass g_stringClass, 
  g_commandClass, g_eventClass, g_linkClass, g_supportClass, g_pluginClass;
jmethodID g_stringMID, g_toStringMID,
  g_commandMID, g_eventMID, g_linkMID, g_supportMID, g_pluginMID;
jmethodID g_stringBytesMID, 
  g_eventParsePayloadMID, g_eventBuildPayloadMID, 
  g_pluginCloseMID, g_pluginConfigMID, g_pluginStartMID, g_pluginStopMID, g_pluginLearnMID, 
  g_pluginCommandMID, g_pluginCommandGUIMID, g_pluginCommandChangedMID;
jfieldID g_commandFID, g_linkFID, g_eventFID;

JavaVM *g_jvm = NULL;

// Get an environment handle.  When called the first time, create a new JVM.
JNIEnv *getJNIEnv()
{
  JNIEnv *jenv;

  if (NULL != g_jvm) {
    // Existing JVM, get jenv for this thread.
    g_jvm->AttachCurrentThread((void **)&jenv, NULL);
    return jenv;
  }

  char exepath[MAX_PATH], classpath[1024], libpath[1024], *cp;
  GetModuleFileName(NULL, exepath, sizeof(exepath));
  cp = strrchr(exepath, '\\');
  if (NULL != cp)
    *++cp = '\0';
  // TODO: Where do the extension classes come from?
  strcpy(classpath, "-Djava.class.path=");
  strcat(classpath, exepath);
  strcat(classpath, "GirderJava.jar");
  strcpy(libpath, "-Djava.library.path=");
  strcat(libpath, exepath);
  strcat(libpath, "Plugins");

  JavaVMInitArgs jvm_args;
  JavaVMOption options[4];

  options[0].optionString = classpath;
  options[1].optionString = libpath;
  
  jvm_args.version = JNI_VERSION_1_2;
  jvm_args.options = options;
#if 1
  // Allow attaching a remote debugger (like BugSeeker).
  options[2].optionString = "-Xdebug";
  char wrdbgstr[1024];          // Debugger modifies string, so cannot be constant.
  strcpy(wrdbgstr, "-Xrunjdwp:transport=dt_socket,address=1999,server=y,suspend=n");
  options[3].optionString = wrdbgstr;
  jvm_args.nOptions = 4;
#else
  jvm_args.nOptions = 2;
#endif
  jvm_args.ignoreUnrecognized = FALSE;

  jint ret = JNI_CreateJavaVM(&g_jvm, (void**)&jenv, &jvm_args);
  if (ret < 0)
    return NULL;
  
  jclass jc;

  jc = jenv->FindClass("java/lang/String");
  g_stringClass = (jclass)jenv->NewGlobalRef(jc);
  g_stringMID = jenv->GetMethodID(jc, "<init>", "([B)V");
  g_stringBytesMID = jenv->GetMethodID(jc, "getBytes", "()[B");
  jenv->DeleteLocalRef(jc);

  jc = jenv->FindClass("java/lang/Object");
  g_toStringMID = jenv->GetMethodID(jc, "toString", "()Ljava/lang/String;");
  jenv->DeleteLocalRef(jc);

  jc = jenv->FindClass("nl/girder/api/GirderCommand");
  g_commandClass = (jclass)jenv->NewGlobalRef(jc);
  g_commandMID = jenv->GetMethodID(jc, "<init>", "(I)V");
  g_commandFID = jenv->GetFieldID(jc, "m_nativeCommand", "I");
  jenv->DeleteLocalRef(jc);

  jc = jenv->FindClass("nl/girder/api/GirderEvent");
  g_eventClass = (jclass)jenv->NewGlobalRef(jc);
  g_eventMID = jenv->GetMethodID(jc, "<init>", "(Ljava/lang/String;)V");
  g_eventParsePayloadMID = jenv->GetMethodID(jc, "parsePayload", "([B)V");
  g_eventBuildPayloadMID = jenv->GetMethodID(jc, "buildPayload", "()[B");
  g_eventFID = jenv->GetFieldID(jc, "m_eventString", "Ljava/lang/String;");
  jenv->DeleteLocalRef(jc);

  jc = jenv->FindClass("nl/girder/api/GirderLink");
  g_linkClass = (jclass)jenv->NewGlobalRef(jc);
  g_linkMID = jenv->GetMethodID(jc, "<init>", "(I)V");
  g_linkFID = jenv->GetFieldID(jc, "m_nativeLink", "I");
  jenv->DeleteLocalRef(jc);

  jc = jenv->FindClass("nl/girder/api/GirderSupport");
  g_supportClass = (jclass)jenv->NewGlobalRef(jc);
  g_supportMID = jenv->GetMethodID(jc, "<init>", "()V");
  jenv->DeleteLocalRef(jc);

  jc = jenv->FindClass("nl/girder/api/GirderPlugin");
  g_pluginClass = (jclass)jenv->NewGlobalRef(jc);
  g_pluginMID = jenv->GetMethodID(jc, "<init>", "(Lnl/girder/api/GirderSupport;)V");
  g_pluginCloseMID = jenv->GetMethodID(jc, "close", "()V");
  g_pluginConfigMID = jenv->GetMethodID(jc, "config", "()V");
  g_pluginStartMID = jenv->GetMethodID(jc, "start", "()V");
  g_pluginStopMID = jenv->GetMethodID(jc, "stop", "()V");
  g_pluginLearnMID = jenv->GetMethodID(jc, "learn", "(Ljava/lang/String;)Ljava/lang/String;");
  g_pluginCommandMID = jenv->GetMethodID(jc, "command", 
    "(Lnl/girder/api/GirderCommand;Lnl/girder/api/GirderEvent;[Ljava/lang/String;)I");
  g_pluginCommandGUIMID = jenv->GetMethodID(jc, "commandGUI", "()V");
  g_pluginCommandChangedMID = jenv->GetMethodID(jc, "commandChanged", 
                                                "(Lnl/girder/api/GirderCommand;)V");
  jenv->DeleteLocalRef(jc);

  if (!checkException(jenv))
    return NULL;
  else
    return jenv;
}

BOOL checkException(JNIEnv *jenv)
{
  jthrowable jerr = jenv->ExceptionOccurred();
  if (NULL == jerr) return TRUE;

  jenv->ExceptionClear();
  jstring jmsg = (jstring)jenv->CallObjectMethod(jerr, g_toStringMID);
  jenv->DeleteLocalRef(jerr);
  PCHAR msg = jstringToNative(jenv, jmsg);
  jenv->DeleteLocalRef(jmsg);

  MessageBox(SF.parent_hwnd, msg, "Java Error", MB_ICONERROR);
  SF.gir_free(msg);

  return FALSE;
}

// Convert a native string (byte array) into a Java string.
jstring jstringFromNative(JNIEnv *jenv, PCHAR str)
{
  jstring result = NULL;

  if (NULL != str) {
    size_t slen = strlen(str);
    jbyteArray jbytes = jenv->NewByteArray(slen);
    if (NULL != jbytes) {
      jenv->SetByteArrayRegion(jbytes, 0, slen, (jbyte*)str);
      result = (jstring)jenv->NewObject(g_stringClass, g_stringMID, jbytes);
      jenv->DeleteLocalRef(jbytes);
    }
  }

  return result;
}

// Convert a Java string into a native string.
// Result comes from the Girder heap and must be freed with gir_free.
PCHAR jstringToNative(JNIEnv *jenv, jstring jstr)
{
  PCHAR result = NULL;

  if (NULL != jstr) {
    jbyteArray jbytes = (jbyteArray)jenv->CallObjectMethod(jstr, g_stringBytesMID);
    if (NULL != jbytes) {
      jsize slen = jenv->GetArrayLength(jbytes);
      result = (PCHAR)SF.gir_malloc(slen + 1);
      jenv->GetByteArrayRegion(jbytes, 0, slen, (jbyte*)result);
      result[slen] = '\0';
      jenv->DeleteLocalRef(jbytes);
    }
  }

  return result;
}

// Wrap a command.
jobject createCommand(JNIEnv *jenv, p_command cmd)
{
  jobject result = NULL;

  if (0 != cmd) {
    result = jenv->NewObject(g_commandClass, g_commandMID, (jint)cmd);
  }

  return result;
}

// Unwrap a command.
p_command getCommand(JNIEnv *jenv, jobject jcmd)
{
  return (p_command)jenv->GetIntField(jcmd, g_commandFID);
}

// Wrap an event.
jobject createEvent(JNIEnv *jenv, char *eventString, void *payload, int len)
{
  jstring jeventstr = jstringFromNative(jenv, eventString);
  jobject jevent = jenv->NewObject(g_eventClass, g_eventMID, jeventstr);
  jenv->DeleteLocalRef(jeventstr);
  
  if (NULL != payload) {
    jbyteArray jpayload = jenv->NewByteArray(len);
    jenv->SetByteArrayRegion(jpayload, 0, len, (jbyte*)payload);
    jenv->CallVoidMethod(jevent, g_eventParsePayloadMID, jpayload);
    jenv->DeleteLocalRef(jpayload);
  }

  return jevent;
}

// Unwrap an event.
PCHAR getEvent(JNIEnv *jenv, jobject jevent, void*& payload, int& len)
{
  jstring jeventstr = (jstring)jenv->GetObjectField(jevent, g_eventFID);
  PCHAR event = jstringToNative(jenv, jeventstr);
  jenv->DeleteLocalRef(jeventstr);

  jbyteArray jpayload = (jbyteArray)jenv->CallObjectMethod(jevent, 
                                                           g_eventBuildPayloadMID);
  if (NULL != jpayload) {
    len = jenv->GetArrayLength(jpayload);
    payload = SF.gir_malloc(len);
    jenv->GetByteArrayRegion(jpayload, 0, len, (jbyte*)payload);
    jenv->DeleteLocalRef(jpayload);
  }

  return event;
}

// Wrap a link.
jobject createLink(JNIEnv *jenv, int link)
{
  jobject result = NULL;

  if (0 != link) {
    result = jenv->NewObject(g_linkClass, g_linkMID, link);
  }

  return result;
}

// Unwrap a link.
int getLink(JNIEnv *jenv, jobject jlink)
{
  if (NULL == jlink)
    return 0;
  else
    return (int)jenv->GetIntField(jlink, g_linkFID);
}

// Create the main plugin dispatcher object.
jobject createPlugin(JNIEnv *jenv)
{
  jobject jsupport = jenv->NewObject(g_supportClass, g_supportMID);
  jobject result = jenv->NewObject(g_pluginClass, g_pluginMID, jsupport);
  jenv->DeleteLocalRef(jsupport);
  return result;
}
