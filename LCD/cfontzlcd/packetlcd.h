/* Crystalfontz command packet device implementation 
$Header$
*/

class ReceivePacket;
class SendPacket;

class CrystalfontzPacketLCD : public DisplayDevice
{
public:
  enum { OLD_CMDS, NEW_CMDS, LEGENDS };
  CrystalfontzPacketLCD(DisplayDeviceFactory *factory, LPCSTR devtype,
                        int cols, int rows, int devmode);
  CrystalfontzPacketLCD(const CrystalfontzPacketLCD& other);
  ~CrystalfontzPacketLCD();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual int DeviceNCustomCharacters();
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
  virtual BOOL DeviceHasKeypadLegends();
  virtual LPCSTR *DeviceGetKeypadButtonChoices();
  virtual LPCSTR *DeviceGetKeypadLegendChoices();
  virtual void DeviceSetKeypadLegend(LPCSTR button, LPCSTR legend);
  virtual int DeviceGetNFans();
  virtual void DeviceSetFanPower(int fan, double power);
  virtual IntervalMode DeviceHasFanInterval();
  virtual void DeviceCheckFans();
  virtual BOOL DeviceHasSensors();
  virtual IntervalMode DeviceHasSensorInterval();
  virtual void DeviceDetectSensors(LPCSTR prefix);

protected:
  void Receive(ReceivePacket *pkt);
  void SendOneWay(SendPacket *pkt);
  ReceivePacket *Send(SendPacket *pkt);

  void UpdateKeypadLegends();
  void MatchSensors(LPCSTR createPrefix);

  BOOL m_hasSendData, m_hasKeypadLegends, m_hasFansAndSensors;

#define NFANS 4
  FanMonitor *m_fansIndexed[NFANS];
#define NSENSORS 32
  DOWSensor *m_sensorsIndexed[NSENSORS];

  CRITICAL_SECTION m_inputCS;
  SendPacket *m_sendHead, *m_sendTail;
  HANDLE m_sendEvent;
};
