// $Header$

package nl.girder.api;

import java.util.Date;

/** The Girder plugin dispatcher.<p>
 * There is one instance of this class.  It is called by Girder and
 * must pass control to various other Java plugins.
 */
public class GirderPlugin {
  static {
    System.loadLibrary("GirderJava");
  }

  private GirderSupport m_support;

  protected GirderPlugin(GirderSupport support) {
    m_support = support;
  }

  /** Return codes from {@link #command} (and {@link GirderLink#trigger}). */
  public static final int EVENT_CONTINUE = 0;
  public static final int EVENT_RETURN = 1;
  public static final int EVENT_STOP = 2;

  /** Called from Girder to dispatch an event.
   * @param status holder for command status output
   */
  protected int command(GirderCommand command, GirderEvent event, String[] status) {
    // TODO: Dispatch to user defined code in Java plugins.
    m_support.setStringVariable("then", command.getStringValue1());
    m_support.setStringVariable("now", new Date().toString());

    GirderLink l;
    if (command.getBooleanValue1())
      l = command.getLinkValue1();
    else
      l = command.getLinkValue2();
    if (null != l)
      return l.trigger();
    else {
      status[0] = "No link.";
      return EVENT_CONTINUE;
    }
  }

  private GirderCommand m_currentCommand;

  /** Called from Girder when Settings chosen for command. */
  protected void commandGUI() {
    // TODO: Real config dialog.
    if (null != m_currentCommand) {
      m_currentCommand.setStringValue1(new Date().toString());
      m_currentCommand.setLinkValue1(m_support.pickLink(m_currentCommand.getLinkValue1()));
      m_currentCommand.setActionSubType(69);
      m_currentCommand.save();
    }
  }
  
  /** Called from Girder when selected command changes. */
  protected void commandChanged(GirderCommand command) {
    m_currentCommand = command;
  }

  /** Called from Girder when plugin is closed. */
  protected void close() {
    // TODO: Close Java plugins.
  }

  /** Called from Girder to display plugin Settings. */
  protected void config() {
    // TODO: Config Java plugins.
  }

  private Thread m_thread;

  /** Called from Girder when event are enabled. */
  protected void start() {
    // TODO: Start Java plugins.
    m_thread = new Thread(new Runnable() {
        public void run() {
          while (true) {
            try {
              Thread.sleep(10000);
            }
            catch (InterruptedException ex) {
              break;
            }
            GirderEvent event = new GirderEvent("bang");
            event.setStringPayload(0, new Date().toString());
            m_support.sendEvent(event);
          }
        }
      });
    m_thread.start();
  }

  /** Called from Girder when event are disabled. */
  protected void stop() {
    if (null == m_thread) 
      return;

    m_thread.interrupt();
    try {
      m_thread.join();
    }
    catch (InterruptedException ex) {
    }
    m_thread = null;
  }

  /** Called from Girder to learn an event. */
  protected String learn(String defevent) {
    // TODO: Learn from Java plugins.  Should this be asynch (modeless)?
    return "bang";
  }
}
