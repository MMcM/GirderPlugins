/* Display device interface 
$Header$
*/

typedef const BYTE *LPCBYTE;

#ifdef LCD_EXPORTS
#define LCD_API __declspec(dllexport)
#else
#define LCD_API __declspec(dllimport)
#endif

class LCD_API CustomCharacter
{
public:
  CustomCharacter() {
    memset(m_bits, 0, sizeof(m_bits));
  }
  CustomCharacter(LPCSTR defn) {
    LoadFromString(defn);
  }
  CustomCharacter(const CustomCharacter& other) {
    memcpy(m_bits, other.m_bits, sizeof(m_bits));
  }
  ~CustomCharacter() {
  }

  CustomCharacter& operator=(const CustomCharacter& other) {
    memcpy(m_bits, other.m_bits, sizeof(m_bits));
    return *this;
  }
  int operator==(const CustomCharacter& other) {
    return !memcmp(m_bits, other.m_bits, sizeof(m_bits));
  }

  const BYTE* GetBits() const {
    return m_bits;
  }
  void SetBits(LPBYTE bits) {
    memcpy(m_bits, bits, sizeof(m_bits));
  }

  void LoadFromString(LPCSTR defn);

protected:
  // Same order as most devices: top to bottom, MSB on left.
#define NCUSTCOLS 5  
#define NCUSTROWS 8
  BYTE m_bits[NCUSTROWS];
};

class LCD_API DisplayDevice
{
public:
  static HKEY GetSettingsKey();
  static BOOL GetSettingString(HKEY hkey, LPCSTR valkey,
                               LPSTR value, size_t vallen);
  static BOOL GetSettingInt(HKEY hkey, LPCSTR valkey, int& value);
  static BOOL GetSettingBool(HKEY hkey, LPCSTR valkey, BOOL& defval);
  static BOOL GetSettingBinary(HKEY hkey, LPCSTR valkey,
                               LPBYTE value, size_t vallen);
  static void SetSettingString(HKEY hkey, LPCSTR valkey, LPCSTR value);
  static void SetSettingInt(HKEY hkey, LPCSTR valkey, int value);
  static void SetSettingBool(HKEY hkey, LPCSTR valkey, BOOL value);
  static void SetSettingBinary(HKEY hkey, LPCSTR valkey,
                               LPCBYTE value, size_t vallen);

  // Create device based on registry settings.
  static DisplayDevice *Create(HWND parent = NULL, LPCSTR lib = NULL, LPCSTR dev = NULL);
  virtual ~DisplayDevice();

  BOOL IsOpen() {
    return m_open;
  }
  BOOL Open();
  void Close();

  int GetWidth() {
    return m_cols;
  }
  int GetHeight() {
    return m_rows;
  }
  BOOL HasSetSize() {
    return DeviceHasSetSize();
  }
  void SetSize(int width, int height);

  enum PortType_t { portNONE, portSERIAL, portPARALLEL };
  PortType_t GetPortType() {
    return m_portType;
  }
  LPCSTR GetPort() {
    return m_port;
  }
  void SetPort(LPCSTR port) {
    strncpy(m_port, port, sizeof(m_port));
  }
  int GetPortSpeed() {
    return m_portSpeed;
  }
  void SetPortSpeed(int speed) {
    m_portSpeed = speed;
  }

  BOOL HasContrast() {
    return DeviceHasContrast();
  }
  int GetContrast() {
    return m_contrast;
  }
  void SetContrast(int percent) {
    m_contrast = percent;
  }

  BOOL HasBrightness() {
    return DeviceHasBrightness();
  }
  BOOL HasBacklight() {
    return DeviceHasBacklight();
  }
  int GetBrightness() {
    return m_brightness;
  }
  void SetBrightness(int percent) {
    m_brightness = percent;
  }

  BOOL HasKeypad() {
    return DeviceHasKeypad();
  }
  BOOL GetEnableInput() {
    return m_enableInput;
  }
  void SetEnableInput(BOOL enable) {
    m_enableInput = enable;
  }
  BOOL EnableInput() {
    return DeviceEnableInput();
  }
  void DisableInput() {
    DeviceDisableInput();
  }

  int GetGPOs() {
    return DeviceGetGPOs();
  }
  void SetGPO(int gpo, BOOL on) {
    DeviceSetGPO(gpo, on);
  }

  void LoadSettings(HKEY hkey);
  void SaveSettings(HKEY hkey);

  // Drawing.
  void Display(int row, int col, int width, LPCSTR str);
  void DisplayCharacter(int row, int col, char ch);
  void DisplayCustomCharacter(int row, int col, const CustomCharacter& cust);
  void Clear();
  void Test();

protected:
  typedef DisplayDevice *(*CreateFun_t)(HWND parent, LPCSTR devname);
  DisplayDevice();

  // Override these for each device.
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length) = 0;
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust) = 0;
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceSetMarquee();
  virtual void DeviceClearMarquee();
  virtual BOOL DeviceHasSetSize();
  virtual BOOL DeviceHasContrast();
  virtual BOOL DeviceHasBrightness();
  virtual BOOL DeviceHasBacklight();
  virtual BOOL DeviceHasKeypad();
  virtual BOOL DeviceEnableInput();
  virtual void DeviceDisableInput();
  virtual void DeviceInput(BYTE b);
  virtual int DeviceGetGPOs();
  virtual void DeviceSetGPO(int gpo, BOOL on);
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);

  // These can be used by device implementations.
  void DisplayInternal(int row, int col, LPCBYTE str, int length);
  int DefineCustomCharacter(const CustomCharacter& cust);
  void SetSimulatedMarquee();
  void ClearSimulatedMarquee();
  void StepSimulatedMarquee();
  friend void WINAPI MarqueeTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

  BOOL OpenSerial(BOOL asynch = FALSE);
  BOOL WriteSerial(LPBYTE data, size_t len);
  BOOL EnableSerialInput();
  void DisableSerialInput();
  void CloseSerial();
  friend DWORD WINAPI SerialInputThread(LPVOID lpParam);

  // State.
  BYTE m_characterMap[256];

  BOOL m_open;
  int m_cols, m_rows;
  LPBYTE m_buffer;

#define NCUSTCHARS 8
  CustomCharacter m_customCharacters[NCUSTCHARS];
  DWORD m_customLastUse[NCUSTCHARS];

  LPBYTE m_marquee;
  int m_marqueeRow, m_marqueeLen, m_marqueePos;
  int m_marqueePixelWidth, m_marqueeSpeed;
  UINT m_marqueeTimer;

  PortType_t m_portType;
  char m_port[8];
  int m_portSpeed;
  BOOL m_portRTS, m_portDTR;
  HANDLE m_portHandle;

  int m_contrast, m_brightness;

  BOOL m_enableInput;
  HANDLE m_inputThread, m_inputStopEvent, m_outputEvent;
};

extern "C" {
LCD_API void DisplayWin32Error(HWND parent, DWORD dwErr);
LCD_API void DisplaySendEvent(LPCSTR event);
}
