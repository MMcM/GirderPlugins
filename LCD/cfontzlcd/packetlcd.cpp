/* Crystalfontz command packet device implementation 
$Header$
*/

#include "stdafx.h"

#include "packetlcd.h"

class Packet
{
public:
  Packet(BYTE type, BYTE length) {
    m_bytes = (LPBYTE)malloc(length + 4); // type, length, data, CRC
    m_bytes[0] = type;
    m_bytes[1] = length;
  }
  ~Packet() {
    free(m_bytes);
  }
  BYTE GetType() const {
    return m_bytes[0];
  }
  BYTE GetDataLength() const {
    return m_bytes[1];
  }
  LPCBYTE GetData() const {
    return m_bytes + 2;
  }
  LPBYTE GetData() {
    return m_bytes + 2;
  }
  LPCBYTE GetBytes() const {
    return m_bytes;
  }
  LPBYTE GetBytes() {
    return m_bytes;
  }
  int GetTotalLength() const {
    return (GetDataLength() + 4);
  }
  WORD GetCRC() const {
    return *(UNALIGNED WORD *)(GetData() + GetDataLength());
  }
  void SetCRC(WORD crc) const {
    *(UNALIGNED WORD *)(GetData() + GetDataLength()) = crc;
  }
  static WORD ComputeCRC(LPCBYTE data, size_t len);
 
protected:
  LPBYTE m_bytes;
};

const BYTE MAX_DATA_LENGTH = 22;

class ReceivePacket : public Packet
{
public:
  ReceivePacket(BYTE type, BYTE length) : Packet(type, length) {
  }
  
  BOOL CheckCRC() const {
    return (GetCRC() == ComputeCRC(GetBytes(), GetDataLength() + 2));
  }

protected:
};

class SendPacket : public Packet
{
public:
  SendPacket(BYTE type, BYTE length);
  ~SendPacket();

  void UpdateCRC() {
    SetCRC(ComputeCRC(GetBytes(), GetDataLength() + 2));
  }

  enum State_t { TRANSMIT_NEEDED, TRANSMITTING, AWAITING_ACK, ACKED, TIMED_OUT };
  State_t GetState() const {
    return m_state;
  }
  void SetState(State_t state) {
    m_state = state;
  }

  int GetRetries() const {
    return m_retries;
  }
  int IncrementRetries() {
    return ++m_retries;
  }

  BOOL IsTwoWay() const {
    return (NULL != m_event);
  }
  HANDLE GetEvent() const {
    return m_event;
  }
  void SetTwoWay() {
    m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  }

  ReceivePacket *GetReply() const {
    return m_reply;
  }
  void SetReply(ReceivePacket *reply) {
    m_reply = reply;
  }
  
  SendPacket *GetNext() {
    return m_next;
  }
  void SetNext(SendPacket *next) {
    m_next = next;
  }

protected:
  State_t m_state;
  int m_retries;
  HANDLE m_event;
  ReceivePacket *m_reply;
  SendPacket *m_next;
};

const char *KEYPAD_6 =
  "1	Up\n"
  "2	Down\n"
  "3	Left\n"
  "4	Right\n"
  "5	Enter\n"
  "6	Cancel\n"
  ;
// 7-12 are release of the above.

static LPCSTR KeypadDefaultEvents[] = {
  "UL",                         // 13
  "UR",                         // 14
  "LL",                         // 15
  "LR",                         // 16
  NULL
};
// 17-20 are release of the above.

// These are the ones in the CFA-631 firmware.
static LPCSTR KeypadLegendEvents[] = {
  "",                           // KEY_LEGEND_BLANK
  "Cancel",                     // KEY_LEGEND_CANCEL
  "Enter",                      // KEY_LEGEND_CHECK
  "Up",                         // KEY_LEGEND_UP
  "Down",                       // KEY_LEGEND_DOWN
  "Right",                      // KEY_LEGEND_RIGHT
  "Left",                       // KEY_LEGEND_LEFT
  "Plus",                       // KEY_LEGEND_PLUS
  "Minus",                      // KEY_LEGEND_MINUS
  NULL                          // KEY_LEGEND_NONE
};

CrystalfontzPacketLCD::CrystalfontzPacketLCD
(DisplayDeviceFactory *factory, LPCSTR devtype,
 int cols, int rows, int devmode)
  : DisplayDevice(factory, devtype)
{
  m_hasSendData = (devmode != OLD_CMDS);
  m_hasKeypadLegends = (devmode == LEGENDS);

  m_cols = cols;
  m_rows = rows;

  if (devmode == OLD_CMDS)
    m_inputMap.LoadFromString(KEYPAD_6);
  else {
    for (int i = 0; i < 4; i++) {
      char buf[8];
      sprintf(buf, "%d", i+13);
      m_inputMap.Put(buf, KeypadDefaultEvents[i]);
    }
  }

  m_portType = portSERIAL;
  strcpy(m_port, "COM1");
  m_portSpeed = (devmode == OLD_CMDS) ? CBR_19200 : CBR_115200;

  m_inputEnabled = FALSE;

  m_sendHead = m_sendTail = NULL;
  InitializeCriticalSection(&m_inputCS);
}

CrystalfontzPacketLCD::CrystalfontzPacketLCD(const CrystalfontzPacketLCD& other)
  : DisplayDevice(other)
{
  m_hasSendData = other.m_hasSendData;

  m_inputEnabled = FALSE;

  m_sendHead = m_sendTail = NULL;
  InitializeCriticalSection(&m_inputCS);
}

CrystalfontzPacketLCD::~CrystalfontzPacketLCD()
{
  DeleteCriticalSection(&m_inputCS);
}

DisplayDevice *CrystalfontzPacketLCD::Duplicate() const
{
  return new CrystalfontzPacketLCD(*this);
}

BOOL CrystalfontzPacketLCD::DeviceOpen()
{
  if (!OpenSerial(TRUE))
    return FALSE;

  COMMTIMEOUTS timeouts;
  if (GetCommTimeouts(m_portHandle, &timeouts)) {
    // Don't wait more than 2ms between characters.  This is important
    // because we specify a read buffer that is larger than the
    // packet.
    timeouts.ReadIntervalTimeout=2;
    SetCommTimeouts(m_portHandle, &timeouts);
  }

  m_sendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  EnableSerialInput();

  // Get Hardware & Firmware Version
  SendPacket *spkt = new SendPacket(1, 0);
  ReceivePacket *rpkt = Send(spkt);
  // Report errors from this command since they probably indicate
  // incorrect hardware configuration.
  if (NULL == rpkt) {
    MessageBox(NULL, "Cannot communicate with LCD display",
               "Error", MB_OK | MB_ICONERROR);
    DeviceClose();
    return FALSE;
  }
  else if (0x90 == (rpkt->GetType() & 0x90)) {
    MessageBox(NULL, "Initial command to LCD failed",
               "Error", MB_OK | MB_ICONERROR);
    delete rpkt;
    DeviceClose();
    return FALSE;
  }
  else {
#if _DEBUG
    {
      char dbuf[128], *dbp;
      strcpy(dbuf, "PKTLCD: Version: ");
      dbp = dbuf + strlen(dbuf);
      memcpy(dbp, (LPCSTR)rpkt->GetData(), rpkt->GetDataLength());
      dbp += rpkt->GetDataLength();
      strcpy(dbp, "\n");
      OutputDebugString(dbuf);
    }
#endif
    delete rpkt;
  }
  
  spkt = new SendPacket(13, 1); // Set LCD Contrast
  *spkt->GetData() = (m_contrast * 256) / 100;
  SendOneWay(spkt);
  
  spkt = new SendPacket(14, 1); // Set LCD & Keypad Backlight
  *spkt->GetData() = m_brightness;
  SendOneWay(spkt);

  UpdateKeypadLegends();

  return TRUE;
}

void CrystalfontzPacketLCD::DeviceClose()
{
  SendPacket *spkt = new SendPacket(6, 0); // Clear LCD Screen
  SendOneWay(spkt); 
  spkt = new SendPacket(14, 1); // Set LCD & Keypad Backlight
  *spkt->GetData() = 0;         // Off
  Send(spkt);                   // Wait for ACK before close

  DisableSerialInput();
  CloseHandle(m_sendEvent);
  while (NULL != m_sendHead) {
    SendPacket *next = m_sendHead->GetNext();
    delete m_sendHead;
    m_sendHead = next;
    if (NULL == next)
      m_sendTail = NULL;
  }
  CloseSerial();
}

void CrystalfontzPacketLCD::DeviceClear()
{
  SendOneWay(new SendPacket(6, 0)); // Clear LCD Screen
}

void CrystalfontzPacketLCD::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  SendPacket *spkt;
  if (m_hasSendData) {
    spkt = new SendPacket(31, length + 2); // Send Data to LCD
    LPBYTE pb = spkt->GetData();
    *pb++ = col;
    *pb++ = row;
    memcpy(pb, str, length);
  }
  else {
    switch (row) {
    case 0:
      spkt = new SendPacket(7, 16); // Set LCD Contents, Line 1
      break;
    case 1:
      spkt = new SendPacket(8, 16); // Set LCD Contents, Line 2
      break;
    default:
      return;
    }
    // Must display the whole row; buffer should have been updated already.
    memcpy(spkt->GetData(), m_buffer->GetBuffer(row, 0), 16);
  }
  SendOneWay(spkt);
}

int CrystalfontzPacketLCD::DeviceNCustomCharacters()
{
  if (m_hasKeypadLegends)
    return 2;                   // Takes six positions, so needs that many chars.
  else
    return DisplayDevice::DeviceNCustomCharacters();
}

void CrystalfontzPacketLCD::DeviceDefineCustomCharacter(int index, 
                                                        const CustomCharacter& cust)
{
  SendPacket *spkt = new SendPacket(9, 9); // Set LCD Special Character Data
  LPBYTE pb = spkt->GetData();
  *pb++ = index;
  memcpy(pb, cust.GetBits(), NCUSTROWS);
  SendOneWay(spkt);
}

BOOL CrystalfontzPacketLCD::DeviceHasContrast()
{
  return TRUE;
}

BOOL CrystalfontzPacketLCD::DeviceHasBrightness()
{
  return TRUE;
}

BOOL CrystalfontzPacketLCD::DeviceHasBacklight()
{
  return TRUE;
}

BOOL CrystalfontzPacketLCD::DeviceHasKeypad()
{
  return TRUE;
}

BOOL CrystalfontzPacketLCD::DeviceHasKeypadLegends()
{
  return m_hasKeypadLegends;
}

LPCSTR *CrystalfontzPacketLCD::DeviceGetKeypadButtonChoices()
{
  return KeypadDefaultEvents;
}

LPCSTR *CrystalfontzPacketLCD::DeviceGetKeypadLegendChoices()
{
  return KeypadLegendEvents;
}

void CrystalfontzPacketLCD::DeviceSetKeypadLegend(LPCSTR button, LPCSTR legend)
{
  char buf[8];
  if ('1' != button[0]) {
    // A button can be specified by its default event: that's what's in the combo.
    for (int i = 0; i < 4; i++) {
      if (!strcmp(button, KeypadDefaultEvents[i])) {
        sprintf(buf, "%d", i+13);
        button = buf;
        break;
      }
    }
  }
  DisplayDevice::DeviceSetKeypadLegend(button, legend);
  UpdateKeypadLegends();
}

void CrystalfontzPacketLCD::UpdateKeypadLegends()
{
  SendPacket *spkt = new SendPacket(32, m_hasKeypadLegends ? 5 : 1); // Key Legends
  LPBYTE pb = spkt->GetData();
  *pb = m_hasKeypadLegends;
  if (m_hasKeypadLegends) {
    for (int i = 0; i < 4; i++) {
      char buf[8];
      sprintf(buf, "%d", i+13);
      LPCSTR event = m_inputMap.Get(buf);
      if (NULL == event) {
        pb[i+1] = 9;
        continue;
      }
      pb[i+1] = 0;
      for (int j = 1; j < countof(KeypadLegendEvents)-1; j++) {
        if (!strcmp(event, KeypadLegendEvents[j])) {
          pb[i+1] = j;
          break;
        }
      }
    }
  }
  SendOneWay(spkt);
}

BOOL CrystalfontzPacketLCD::DeviceEnableInput()
{
  // The input thread is running whenever we are open; we just tell it
  // whether to post events or discard notifications.
  if (!Open()) return FALSE;
  m_inputEnabled = TRUE;
  return TRUE;
}

void CrystalfontzPacketLCD::DeviceDisableInput()
{
  m_inputEnabled = FALSE;
}

const DWORD ACK_TIMEOUT = 250;  // Wait for ack to command.
const DWORD IN_PACKET_TIMEOUT = 100; // Wait for more of an incomplete packet.
const int NRETRIES = 1;         // Try twice.

void CrystalfontzPacketLCD::DeviceSerialInputThread()
{
  HANDLE stopEvent = m_inputStopEvent;
  if (NULL == stopEvent) return;

  HANDLE sendEvent = m_sendEvent;
  if (NULL == sendEvent) return;

  HANDLE portHandle = m_portHandle;
  if (NULL == portHandle) return;

  OVERLAPPED olOutput, olInput;
  memset(&olOutput, 0, sizeof(olOutput));
  olOutput.hEvent = m_outputEvent;
  if (NULL == olOutput.hEvent) return;
  memset(&olInput, 0, sizeof(olInput));
  olInput.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  DWORD nbOutput, nbInput;
  BYTE inputBuf[32];
  enum { NONE, PENDING, COMPLETE } inputState = NONE;

  SendPacket *spkt = NULL;
  ReceivePacket *rpkt = NULL;
  BYTE rtype, rlen;
  int rindex = 0;

  while (TRUE) {
    DWORD timeout = INFINITE;
    HANDLE handles[4];
    int nh = 0;
    handles[nh++] = stopEvent;
    handles[nh++] = sendEvent;

    if (NULL == spkt) {
      // Check for new packet to send.
      EnterCriticalSection(&m_inputCS);
      spkt = m_sendHead;
      LeaveCriticalSection(&m_inputCS);
    }
    if (NULL != spkt) {
      if (SendPacket::TRANSMIT_NEEDED == spkt->GetState()) {
        if (WriteFile(portHandle, spkt->GetBytes(), spkt->GetTotalLength(),
                      &nbOutput, &olOutput))
          spkt->SetState(SendPacket::AWAITING_ACK);
        else if (ERROR_IO_PENDING == GetLastError())
          spkt->SetState(SendPacket::TRANSMITTING);
        else {
          // Is stopping this thread on device error too drastic?
          DisplayWin32Error(GetLastError());
          break;
        }
      }
      if (SendPacket::TRANSMITTING == spkt->GetState()) {
        handles[nh++] = olOutput.hEvent; // Wait for transmit complete.
      }
      else if (SendPacket::AWAITING_ACK == spkt->GetState()) {
        timeout = ACK_TIMEOUT;  // Should expect answer soon.
      }
    }

    if (NONE == inputState) {
      if (ReadFile(portHandle, inputBuf, sizeof(inputBuf), &nbInput, &olInput)) {
        inputState = COMPLETE;
      }
      else if (ERROR_IO_PENDING == GetLastError()) {
        inputState = PENDING;
      }
      else {
        // Is stopping this thread on device error too drastic?
        DisplayWin32Error(GetLastError());
        break;
      }
    }
    if (PENDING == inputState) {
      handles[nh++] = olInput.hEvent; // Wait for input data.
      if (rindex > 0)
        timeout = IN_PACKET_TIMEOUT; // Partial packet should finish soon.
    }

    HANDLE wakeup;
    if (COMPLETE == inputState) {
      wakeup = olInput.hEvent;  // No need to wait.
    }
    else {
      DWORD dwIndex = WaitForMultipleObjects(nh, handles, FALSE, timeout);
      if (WAIT_TIMEOUT == dwIndex)
        wakeup = NULL;
      else
        wakeup = handles[dwIndex - WAIT_OBJECT_0];
    }

    if (m_inputStopEvent == wakeup) {
      // Stop request.
      break;
    }
    else if (NULL == wakeup) {
      // Timeout.  Pending read can remain, but data should start a new packet.
#if _DEBUG
      {
        char dbuf[128];
        sprintf(dbuf, "PKTLCD: Timeout waiting for input.\n");
        OutputDebugString(dbuf);
      }
#endif
      if (NULL != rpkt) {
        delete rpkt;
        rpkt = NULL;
      }
      rindex = 0;
      if ((NULL != spkt) &&
          (SendPacket::AWAITING_ACK == spkt->GetState())) {
        if (spkt->IncrementRetries() > NRETRIES) {
#if _DEBUG
          {
            char dbuf[128];
            sprintf(dbuf, "PKTLCD: %d command timed out.\n", spkt->GetType());
            OutputDebugString(dbuf);
          }
#endif
          spkt->SetState(SendPacket::TIMED_OUT);
          EnterCriticalSection(&m_inputCS);
          m_sendHead = spkt->GetNext();
          if (NULL == m_sendHead)
            m_sendTail = NULL;
          LeaveCriticalSection(&m_inputCS);
          if (spkt->IsTwoWay())
            SetEvent(spkt->GetEvent());
          spkt = NULL;
        }
        else {
#if _DEBUG
          {
            char dbuf[128];
            sprintf(dbuf, "PKTLCD: Retransmitting %d command.\n", spkt->GetType());
            OutputDebugString(dbuf);
          }
#endif
          spkt->SetState(SendPacket::TRANSMIT_NEEDED);
        }
      }
    }
    else if (olOutput.hEvent == wakeup) {
      // Output complete.
      if (GetOverlappedResult(portHandle, &olOutput, &nbOutput, TRUE)) {
        spkt->SetState(SendPacket::AWAITING_ACK);
      }
      else {
        // Is stopping this thread on device error too drastic?
        DisplayWin32Error(GetLastError());
        break;
      }
    }
    else if (olInput.hEvent == wakeup) {
      // Input complete.
      if (PENDING == inputState) {
        if (GetOverlappedResult(portHandle, &olInput, &nbInput, TRUE)) {
          inputState = COMPLETE;
        }
        else {
          // Is stopping this thread on device error too drastic?
          DisplayWin32Error(GetLastError());
          break;
        }
      }
      if (COMPLETE == inputState) {
#if _DEBUG
        static enum { NORMAL, CORRUPT, DROP, ADD } FailureSimulation = NORMAL;
        static int FailureIndex = 0;
        if ((NORMAL != FailureSimulation) &&
            (rindex <= FailureIndex)) {
          DWORD offset = FailureIndex - rindex;
          if (offset < nbInput) {
            {
              char dbuf[128];
              sprintf(dbuf, "PKTLCD: Simulating communications failure at byte %d.\n", 
                      offset);
              OutputDebugString(dbuf);
            }
            switch (FailureSimulation) {
            case CORRUPT:
              inputBuf[offset] ^= 0xFF;
              break;
            case DROP:
              memmove(inputBuf + offset, inputBuf + offset + 1, nbInput - offset - 1);
              nbInput -= 1;
              break;
            case ADD:
              memmove(inputBuf + offset + 1, inputBuf + offset, nbInput - offset);
              nbInput += 1;
              break;
            }
            FailureSimulation = NORMAL;
          }
        }
#endif
        DWORD i = 0;
        while (i < nbInput) {
          switch (rindex) {
          case 0:
            rtype = inputBuf[i++];
            rindex++;
            break;
          case 1:
            rlen = inputBuf[i++];
            if (rlen > MAX_DATA_LENGTH) {
              nbInput = 0;
              PurgeComm(portHandle, PURGE_RXCLEAR);
#if _DEBUG
              {
                char dbuf[128];
                sprintf(dbuf, "PKTLCD: Received %d packet with impossible length %d.\n",
                        rtype, rlen);
                OutputDebugString(dbuf);
              }
#endif
              break;
            }
            rindex++;
            break;
          case 2:
            rpkt = new ReceivePacket(rtype, rlen);
            /* falls through */
          default:
            {
              DWORD nb = rpkt->GetTotalLength() - rindex;
              if (nb > nbInput - i)
                nb = nbInput - i;
              memcpy(rpkt->GetBytes() + rindex, inputBuf + i, nb);
              i += nb;
              rindex += nb;
              if (rindex >= rpkt->GetTotalLength()) {
                // Packet full.
                if (rpkt->CheckCRC()) {
                  Receive(rpkt);
                  spkt = NULL;  // May be affected if acked.
                }
                else {
                  // CRC error: discard any pending input to get better synchronization.
                  nbInput = 0;
                  PurgeComm(portHandle, PURGE_RXCLEAR);
#if _DEBUG
                  {
                    char dbuf[128];
                    sprintf(dbuf, "PKTLCD: Received packet with bad CRC %X.\n", 
                            rpkt->GetCRC());
                    OutputDebugString(dbuf);
                  }
#endif
                  delete rpkt;
                }
                rpkt = NULL;
                rindex = 0;
              }
            }
          }
        }
        inputState = NONE;
      }
    }
  }

  if (NULL != rpkt)
    delete rpkt;

  CancelIo(portHandle);

  CloseHandle(olInput.hEvent);
}

void CrystalfontzPacketLCD::Receive(ReceivePacket *rpkt)
{
  switch (rpkt->GetType() & 0xC0) {
  case 0x00:
    // Might indicate some kind of loopback.  Maybe compare against send head?
#if _DEBUG
    {
      char dbuf[128];
      sprintf(dbuf, "PKTLCD: Command %d received.\n", rpkt->GetType());
      OutputDebugString(dbuf);
    }
#endif          
    break;
  case 0x40:
  case 0xC0:
    {
      SendPacket *spkt;
      EnterCriticalSection(&m_inputCS);
      spkt = m_sendHead;
      if (NULL != spkt) {
        if ((rpkt->GetType() & 0x3F) == (spkt->GetType() & 0x3F)) {
          m_sendHead = spkt->GetNext();
          if (NULL == m_sendHead)
            m_sendTail = NULL;
          spkt->SetNext(NULL);
        }
        else
          spkt = NULL;
      }
      LeaveCriticalSection(&m_inputCS);
      if (NULL != spkt) {
        spkt->SetState(SendPacket::ACKED);
        if (spkt->IsTwoWay()) {
          spkt->SetReply(rpkt);
          rpkt = NULL;
          SetEvent(spkt->GetEvent());
        }
        else {
          if (rpkt->GetType() & 0x80) {
#if _DEBUG
            {
              char dbuf[128];
              sprintf(dbuf, "PKTLCD: Error reply for one-way command %d.\n", 
                      spkt->GetType());
              OutputDebugString(dbuf);
            }
#endif          
          }
          delete spkt;
        }
      }
    }
    break;
  case 0x80:
    if (m_inputEnabled) {
      switch (rpkt->GetType()) {
      case 0x80:
        if (1 == rpkt->GetDataLength()) {
          char buf[8];
          sprintf(buf, "%d", rpkt->GetData()[0]); // Decimal as in manual.
          MapInput(buf);
        }
        break;
      case 0x81:
        if (4 == rpkt->GetDataLength()) {
          char event[8], payload[16];
          sprintf(event, "FAN%d", rpkt->GetData()[0] + 1);
          int tach = rpkt->GetData()[1];
          int ticks = (rpkt->GetData()[2] << 8) + rpkt->GetData()[3];
          if (tach < 3)
            strcpy(payload, "STOP");
          else if (tach < 4)
            strcpy(payload, "SLOW");
          else if (0xFF == tach)
            strcpy(payload, "????");
          else {
            int ppr = 1;
            sprintf(payload, "%5d", ((27692308 / ppr) * (tach - 3) / ticks));
          }
          DisplaySendEvent(event, payload);
        }
        break;
      case 0x82:
        if (3 == rpkt->GetDataLength()) {
          char event[8], payload[16];
          sprintf(event, "TEMP%d", rpkt->GetData()[0] + 1);
          int count = (rpkt->GetData()[1] << 8) + rpkt->GetData()[2];
          sprintf(payload, "%9.4f", (double)count / 16.0);
          DisplaySendEvent(event, payload);
        }
        break;
      }
    }
    break;
  }
  if (NULL != rpkt)              // Unless saved elsewhere.
    delete rpkt;
}

void CrystalfontzPacketLCD::SendOneWay(SendPacket *spkt)
{
  spkt->UpdateCRC();

  // If anything depended on the limited speed of the device to clock
  // their program, we would need to limit the number of packets that
  // could be in the send queue and wait here sometimes.

  BOOL wakeup = FALSE;

  EnterCriticalSection(&m_inputCS);
  if (NULL == m_sendHead) {
    m_sendHead = m_sendTail = spkt;
    wakeup = TRUE;
  }
  else {
    m_sendTail->SetNext(spkt);
    m_sendTail = spkt;
  }
  LeaveCriticalSection(&m_inputCS);

  if (wakeup)
    SetEvent(m_sendEvent);
}

ReceivePacket *CrystalfontzPacketLCD::Send(SendPacket *spkt)
{
  spkt->SetTwoWay();
  SendOneWay(spkt);
  WaitForSingleObject(spkt->GetEvent(), INFINITE);
  ReceivePacket *rpkt = spkt->GetReply();
  spkt->SetReply(NULL);
  delete spkt;
  return rpkt;
}

// Taken from 633_WinTest sample program.
WORD Packet::ComputeCRC(LPCBYTE data, size_t len)
{
  // CRC lookup table to avoid bit-shifting loops.
  static const WORD crcLookupTable[256] = {
    0x00000, 0x01189, 0x02312, 0x0329B, 0x04624, 0x057AD, 0x06536, 0x074BF, 
    0x08C48, 0x09DC1, 0x0AF5A, 0x0BED3, 0x0CA6C, 0x0DBE5, 0x0E97E, 0x0F8F7, 
    0x01081, 0x00108, 0x03393, 0x0221A, 0x056A5, 0x0472C, 0x075B7, 0x0643E, 
    0x09CC9, 0x08D40, 0x0BFDB, 0x0AE52, 0x0DAED, 0x0CB64, 0x0F9FF, 0x0E876, 
    0x02102, 0x0308B, 0x00210, 0x01399, 0x06726, 0x076AF, 0x04434, 0x055BD, 
    0x0AD4A, 0x0BCC3, 0x08E58, 0x09FD1, 0x0EB6E, 0x0FAE7, 0x0C87C, 0x0D9F5, 
    0x03183, 0x0200A, 0x01291, 0x00318, 0x077A7, 0x0662E, 0x054B5, 0x0453C, 
    0x0BDCB, 0x0AC42, 0x09ED9, 0x08F50, 0x0FBEF, 0x0EA66, 0x0D8FD, 0x0C974, 
    0x04204, 0x0538D, 0x06116, 0x0709F, 0x00420, 0x015A9, 0x02732, 0x036BB, 
    0x0CE4C, 0x0DFC5, 0x0ED5E, 0x0FCD7, 0x08868, 0x099E1, 0x0AB7A, 0x0BAF3, 
    0x05285, 0x0430C, 0x07197, 0x0601E, 0x014A1, 0x00528, 0x037B3, 0x0263A, 
    0x0DECD, 0x0CF44, 0x0FDDF, 0x0EC56, 0x098E9, 0x08960, 0x0BBFB, 0x0AA72, 
    0x06306, 0x0728F, 0x04014, 0x0519D, 0x02522, 0x034AB, 0x00630, 0x017B9, 
    0x0EF4E, 0x0FEC7, 0x0CC5C, 0x0DDD5, 0x0A96A, 0x0B8E3, 0x08A78, 0x09BF1, 
    0x07387, 0x0620E, 0x05095, 0x0411C, 0x035A3, 0x0242A, 0x016B1, 0x00738, 
    0x0FFCF, 0x0EE46, 0x0DCDD, 0x0CD54, 0x0B9EB, 0x0A862, 0x09AF9, 0x08B70, 
    0x08408, 0x09581, 0x0A71A, 0x0B693, 0x0C22C, 0x0D3A5, 0x0E13E, 0x0F0B7, 
    0x00840, 0x019C9, 0x02B52, 0x03ADB, 0x04E64, 0x05FED, 0x06D76, 0x07CFF, 
    0x09489, 0x08500, 0x0B79B, 0x0A612, 0x0D2AD, 0x0C324, 0x0F1BF, 0x0E036, 
    0x018C1, 0x00948, 0x03BD3, 0x02A5A, 0x05EE5, 0x04F6C, 0x07DF7, 0x06C7E, 
    0x0A50A, 0x0B483, 0x08618, 0x09791, 0x0E32E, 0x0F2A7, 0x0C03C, 0x0D1B5, 
    0x02942, 0x038CB, 0x00A50, 0x01BD9, 0x06F66, 0x07EEF, 0x04C74, 0x05DFD, 
    0x0B58B, 0x0A402, 0x09699, 0x08710, 0x0F3AF, 0x0E226, 0x0D0BD, 0x0C134, 
    0x039C3, 0x0284A, 0x01AD1, 0x00B58, 0x07FE7, 0x06E6E, 0x05CF5, 0x04D7C, 
    0x0C60C, 0x0D785, 0x0E51E, 0x0F497, 0x08028, 0x091A1, 0x0A33A, 0x0B2B3, 
    0x04A44, 0x05BCD, 0x06956, 0x078DF, 0x00C60, 0x01DE9, 0x02F72, 0x03EFB, 
    0x0D68D, 0x0C704, 0x0F59F, 0x0E416, 0x090A9, 0x08120, 0x0B3BB, 0x0A232, 
    0x05AC5, 0x04B4C, 0x079D7, 0x0685E, 0x01CE1, 0x00D68, 0x03FF3, 0x02E7A, 
    0x0E70E, 0x0F687, 0x0C41C, 0x0D595, 0x0A12A, 0x0B0A3, 0x08238, 0x093B1, 
    0x06B46, 0x07ACF, 0x04854, 0x059DD, 0x02D62, 0x03CEB, 0x00E70, 0x01FF9, 
    0x0F78F, 0x0E606, 0x0D49D, 0x0C514, 0x0B1AB, 0x0A022, 0x092B9, 0x08330, 
    0x07BC7, 0x06A4E, 0x058D5, 0x0495C, 0x03DE3, 0x02C6A, 0x01EF1, 0x00F78
  };

  // Initial CRC value is 0x0FFFF.
  WORD result = 0xFFFF;
  while (len--)
    result = (result >> 8) ^ crcLookupTable[(result ^ *data++) & 0xFF];

  // Make this CRC match the one's complement that is sent in the packet.
  return ~result;
}

SendPacket::SendPacket(BYTE type, BYTE length)
  : Packet(type, length) 
{
  m_state = TRANSMIT_NEEDED;
  m_retries = 0;
  m_event = NULL;
  m_reply = NULL;
  m_next = NULL;
}

SendPacket::~SendPacket()
{
#if _DEBUG
  switch (m_state) {
  case TRANSMITTING:
  case AWAITING_ACK:
    // Only happens if device closed right after a command.
    {
      char dbuf[128];
      sprintf(dbuf, "PKTLCD: SendPacket in unexpected state %d for delete.\n", 
              m_state);
      OutputDebugString(dbuf);
    }
    break;
  }
#endif
  if (NULL != m_event)
    CloseHandle(m_event);
  if (NULL != m_reply)
    delete m_reply;
}
