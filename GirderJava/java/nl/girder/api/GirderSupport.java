
package nl.girder.api;

/** Call back into Girder from a plugin. */
public class GirderSupport {
  protected GirderSupport() {
  }

  /** Expand variable values in a string. */
  public native String parseRegString(String str);
  /** Translate a string, if possible. */
  public native String i18nTranslate(String str);

  /** Choose a link from the command tree. 
   * @param def an existing default link
   */
  public native GirderLink pickLink(GirderLink def);

  /** Get the value of an integer variable. */
  public native int getIntegerVariable(String var);
  /** Get the value of a double variable. */
  public native double getDoubleVariable(String var);
  /** Get the value of a string variable. */
  public native String getStringVariable(String var);

  /** Set the value of an integer variable. */
  public native void setIntegerVariable(String var, int value);
  /** Set the value of a double variable. */
  public native void setDoubleVariable(String var, double value);
  /** Set the value of a string variable. */
  public native void setStringVariable(String var, String value);

  /** Delete a variable. */
  public native void deleteVariable(String var);

  /** Send an event. */
  public native void sendEvent(GirderEvent event);
}
