
package nl.girder.api;

/** Girder link.<p>
 * A link points to another command in the Girder tree.
 * @see GirderSupport#pickLink
 */
public class GirderLink {
  protected int m_nativeLink;
  /** Called by Girder to wrap the native link. */
  protected GirderLink(int nativeLink) {
    m_nativeLink = nativeLink;
  }

  /** Get the command id for this link. */
  public int getID() {
    return m_nativeLink;
  }

  /** Get the name of the command to which this link points. */
  public native String getName();

  // TODO: Would it be better to use exceptions for retExplicitReturn
  // and retStopProcessing?
  /** Run this command (like Goto). */
  public native int trigger();
}
