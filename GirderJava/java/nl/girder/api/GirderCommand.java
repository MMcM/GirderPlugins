
package nl.girder.api;

/** Girder command structure. */
public class GirderCommand {
  protected int m_nativeCommand;
  /** Called by Girder to wrap the native command. */
  protected GirderCommand(int nativeCommand) {
    m_nativeCommand = nativeCommand;
  }

  /** Get the name of this command. */
  public native String getName();

  /** Get the action type of this command. */
  public native int getActionType();
  /** Get the action subtype for this command. */
  public native int getActionSubType();

  /** Set the action subtype for this command. */
  public native void setActionSubType(int type);

  /** Get a string value. */
  public native String getStringValue1();
  /** Get a string value. */
  public native String getStringValue2();
  /** Get a string value. */
  public native String getStringValue3();

  /** Get a boolean value. */
  public native boolean getBooleanValue1();
  /** Get a boolean value. */
  public native boolean getBooleanValue2();
  /** Get a boolean value. */
  public native boolean getBooleanValue3();

  /** Get an integer value. */
  public native int getIntegerValue1();
  /** Get an integer value. */
  public native int getIntegerValue2();
  /** Get an integer value. */
  public native int getIntegerValue3();

  /** Get a link value. */
  public native GirderLink getLinkValue1();
  /** Get a link value. */
  public native GirderLink getLinkValue2();
  /** Get a link value. */
  public native GirderLink getLinkValue3();

  /** Set a string value. */
  public native void setStringValue1(String value);
  /** Set a string value. */
  public native void setStringValue2(String value);
  /** Set a string value. */
  public native void setStringValue3(String value);

  /** Set a boolean value. */
  public native void setBooleanValue1(boolean value);
  /** Set a boolean value. */
  public native void setBooleanValue2(boolean value);
  /** Set a boolean value. */
  public native void setBooleanValue3(boolean value);

  /** Set an integer value. */
  public native void setIntegerValue1(int value);
  /** Set an integer value. */
  public native void setIntegerValue2(int value);
  /** Set an integer value. */
  public native void setIntegerValue3(int value);

  /** Set a link integer value. */
  public native void setLinkValue1(GirderLink value);
  /** Set a link integer value. */
  public native void setLinkValue2(GirderLink value);
  /** Set a link integer value. */
  public native void setLinkValue3(GirderLink value);

  /** Save back the edited command. */
  public native void save();
}
