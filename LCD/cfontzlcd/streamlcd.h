/* Crystalfontz command stream device implementation 
$Header$
*/

class CrystalfontzStreamLCD : public DisplayDevice
{
public:
  CrystalfontzStreamLCD(LPCSTR devname, int cols, int rows, BOOL backlight);
  ~CrystalfontzStreamLCD();
  
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceSetMarquee();
  virtual void DeviceClearMarquee();
  virtual BOOL DeviceHasContrast();
  virtual BOOL DeviceHasBrightness();
  virtual BOOL DeviceHasBacklight();

protected:
  BOOL m_backlight;
};
