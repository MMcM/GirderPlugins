/* Crystalfontz command packet device implementation 
$Header$
*/

class ReceivePacket;
class SendPacket;

class CrystalfontzPacketLCD : public DisplayDevice
{
public:
  CrystalfontzPacketLCD(DisplayDeviceFactory *factory, LPCSTR devtype,
                        int cols, int rows, BOOL newCommands);
  CrystalfontzPacketLCD(const CrystalfontzPacketLCD& other);
  ~CrystalfontzPacketLCD();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual BOOL DeviceHasContrast();
  virtual BOOL DeviceHasBrightness();
  virtual BOOL DeviceHasBacklight();
  virtual BOOL DeviceHasKeypad();
  virtual void DeviceSerialInputThread();
  virtual BOOL DeviceEnableInput();
  virtual void DeviceDisableInput();

protected:
  void Receive(ReceivePacket *pkt);
  void SendOneWay(SendPacket *pkt);
  ReceivePacket *Send(SendPacket *pkt);

  BOOL m_hasSendData;
  BOOL m_inputEnabled;

  CRITICAL_SECTION m_inputCS;
  SendPacket *m_sendHead, *m_sendTail;
  HANDLE m_sendEvent;
};
