/* Global interfaces */

#define PLUGINNUM 43
#define PLUGINNAME "Java"
#define PLUGINVERSION "1.0"

extern s_functions SF;
extern p_command g_CurCommand;		

extern "C" {
JNIEnv *getJNIEnv();
BOOL checkException(JNIEnv *jenv);
jstring jstringFromNative(JNIEnv *jenv, PCHAR str);
PCHAR jstringToNative(JNIEnv *jenv, jstring jstr);
jobject createCommand(JNIEnv *jenv, p_command cmd);
p_command getCommand(JNIEnv *jenv, jobject jcmd);
jobject createLink(JNIEnv *jenv, int link);
int getLink(JNIEnv *jenv, jobject jlink);
jobject createEvent(JNIEnv *jenv, char *eventString, void *payload, int len);
PCHAR getEvent(JNIEnv *jenv, jobject jevent, void*& payload, int& len);
jobject createPlugin(JNIEnv *jenv);
}

extern jclass g_stringClass, 
  g_commandClass, g_eventClass, g_linkClass, g_supportClass, g_pluginClass;
extern jmethodID g_stringMID, g_toStringMID,
  g_commandMID, g_eventMID, g_linkMID, g_supportMID, g_pluginMID;
extern jmethodID g_stringBytesMID, 
  g_eventParsePayloadMID, g_eventBuildPayloadMID, 
  g_pluginCloseMID, g_pluginConfigMID, g_pluginStartMID, g_pluginStopMID, g_pluginLearnMID, 
  g_pluginCommandMID, g_pluginCommandGUIMID, g_pluginCommandChangedMID;
extern jfieldID g_commandFID, g_linkFID, g_eventFID;
