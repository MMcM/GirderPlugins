
package nl.girder.api;

import java.io.*;

/** Girder event.<p> 
 * An event is identified by a string (sometimes called the IR code
 * for historical reasons.)  It can also have a payload consisting of
 * zero or more strings and / or a binary portion.
 */
public class GirderEvent {
  protected String m_eventString;
  protected String[] m_stringPayloads;
  protected byte[] m_rawPayload;
  
  /** Make a new event with the given event string. */
  public GirderEvent(String eventString) {
    m_eventString = eventString;
  }

  /** Get the list of string payloads. */
  public String[] getStringPayloads() {
    return m_stringPayloads;
  }
  /** Set the list of string payloads. */
  public void setStringPayloads(String[] payloads) {
    m_stringPayloads = payloads;
  }
  /** Set one of the string payloads.
   * If the list is not long enough, it is extended. */
  public void setStringPayload(int index, String payload) {
    if ((null == m_stringPayloads) ||
        (index >= m_stringPayloads.length)) {
      String[] payloads = new String[index + 1];
      if (null != m_stringPayloads)
        System.arraycopy(m_stringPayloads, 0, payloads, 0, m_stringPayloads.length);
      m_stringPayloads = payloads;
    }
    m_stringPayloads[index] = payload;
  }

  /** Get the raw payload. */
  public byte[] getRawPayload() {
    return m_rawPayload;
  }
  /** Set the raw payload. */
  public void setRawPayload(byte[] payload) {
    m_rawPayload = payload;
  }

  /** Parse the payload array passed internally to the command into
   * the separate string and raw payloads. */
  protected void parsePayload(byte[] payload) {
    if (null == payload)
      return;
    int idx = 0;
    int nstr = payload[idx++];
    if (nstr > 0) {
      m_stringPayloads = new String[nstr];
      for (int i = 0; i < nstr; i++) {
        int oidx = idx;
        while (0 != payload[idx++]);
        byte[] bytes = new byte[idx - oidx - 1];
        System.arraycopy(payload, oidx, bytes, 0, bytes.length);
        m_stringPayloads[i] = new String(bytes);
      }
    }
    if (idx < payload.length) {
      m_rawPayload = new byte[payload.length - idx];
      System.arraycopy(payload, idx, m_rawPayload, 0, m_rawPayload.length);
    }
  }

  /** Build the internal form of the payload from the separate string
   * and raw pieces. */
  protected byte[] buildPayload() {
    if ((null == m_stringPayloads) &&
        (null == m_rawPayload))
      return null;
    try {
      ByteArrayOutputStream bstr = new ByteArrayOutputStream();
      if (null == m_stringPayloads)
        bstr.write(0);
      else {
        bstr.write((byte)m_stringPayloads.length);
        for (int i = 0; i < m_stringPayloads.length; i++) {
          String payload = m_stringPayloads[i];
          if (null != payload)
            bstr.write(payload.getBytes());
          bstr.write(0);
        }
      }
      if (null != m_rawPayload)
        bstr.write(m_rawPayload);
      return bstr.toByteArray();
    }
    catch (IOException ex) {
      throw new RuntimeException(ex.toString());
    }
  }
}
