/* Crystalfontz command stream device implementation 
$Header$
*/

class CrystalfontzStreamLCD : public DisplayDevice
{
public:
  CrystalfontzStreamLCD(DisplayDeviceFactory *factory, LPCSTR devtype,
                       int cols, int rows, BOOL backlight);
  CrystalfontzStreamLCD(const CrystalfontzStreamLCD& other);
  ~CrystalfontzStreamLCD();
  
  DisplayDevice *Duplicate() const;
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

#if 0
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);
#endif

protected:
  BOOL m_backlight;
  Delay m_bootDelay;
};
