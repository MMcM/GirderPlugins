/* Display device interface 
$Header$
*/

typedef const BYTE *LPCBYTE;

#ifdef LCD_EXPORTS
#define LCD_API __declspec(dllexport)
#else
#define LCD_API __declspec(dllimport)
#endif
#define LCD_DECL __cdecl

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

class LCD_API DisplayBuffer
{
public:
  DisplayBuffer(int rows, int cols, BYTE space);
  DisplayBuffer(const DisplayBuffer& other); 
  ~DisplayBuffer();

  void Clear() {
    memset(m_bytes, m_space, m_rows * m_cols);
  }

  LPBYTE GetBuffer(int row, int col) {
    return m_bytes + (row * m_cols) + col;
  }

  const CustomCharacter& GetCustomCharacter(int index) const {
    return m_customCharacters[index];
  }
  BOOL IsCustomCharacterUsed(int index) {
    return (0 != m_customLastUse[index]);
  }
  
  int FindCustomCharacter(const CustomCharacter& cust);
  int AllocateCustomCharacter(const CustomCharacter& cust, LPCBYTE characterMap);
  void ClearUnusedCustomCharacters(LPCBYTE characterMap);
  void UseCustomCharacter(int index);

protected:
  LPBYTE m_bytes;
  int m_rows, m_cols;
  BYTE m_space;

#define NCUSTCHARS 8
  CustomCharacter m_customCharacters[NCUSTCHARS];
  DWORD m_customLastUse[NCUSTCHARS];
};

class LCD_API Marquee
{
public:
  Marquee(int row, LPCSTR str, int length, LPCBYTE characterMap);
  Marquee(const Marquee& other);
  ~Marquee();

  int operator==(const Marquee& other) const;

  int GetRow() const {
    return m_row;
  }

  int GetLength() const {
    return m_len;
  }

  LPCBYTE GetBytes() const {
    return m_bytes;
  }
  
  int GetPosition() const {
    return m_pos;
  }
  void SetPosition(int pos) {
    m_pos = pos;
  }

  int NextPosition();

protected:
  LPBYTE m_bytes;
  int m_row, m_len, m_pos;
};

class LCD_API DisplayDevice
{
public:
  static HKEY LCD_DECL GetSettingsKey();
  static BOOL LCD_DECL GetSettingString(HKEY hkey, LPCSTR valkey,
                                        LPSTR value, size_t vallen);
  static BOOL LCD_DECL GetSettingInt(HKEY hkey, LPCSTR valkey, int& value);
  static BOOL LCD_DECL GetSettingBool(HKEY hkey, LPCSTR valkey, BOOL& defval);
  static BOOL LCD_DECL GetSettingBinary(HKEY hkey, LPCSTR valkey,
                                        LPBYTE value, size_t vallen);
  static void LCD_DECL SetSettingString(HKEY hkey, LPCSTR valkey, LPCSTR value);
  static void LCD_DECL SetSettingInt(HKEY hkey, LPCSTR valkey, int value);
  static void LCD_DECL SetSettingBool(HKEY hkey, LPCSTR valkey, BOOL value);
  static void LCD_DECL SetSettingBinary(HKEY hkey, LPCSTR valkey,
                                        LPCBYTE value, size_t vallen);

  // Create device based on registry settings.
  static BOOL LCD_DECL Create(DisplayDevice*& device, HMODULE& devlib,
                              HWND parent = NULL, 
                              LPCSTR lib = NULL, LPCSTR dev = NULL);
  static void LCD_DECL Destroy(DisplayDevice*& device, HMODULE& devlib);
  static void LCD_DECL Take(DisplayDevice*& fromDevice, HMODULE& fromDevlib,
                            DisplayDevice*& toDevice, HMODULE& toDevlib);
  virtual ~DisplayDevice();

  BOOL IsOpen() const {
    return m_open;
  }
  BOOL Open();
  void Close();

  int GetWidth() const {
    return m_cols;
  }
  int GetHeight() const {
    return m_rows;
  }
  BOOL HasSetSize() {
    return DeviceHasSetSize();
  }
  void SetSize(int width, int height);

  enum PortType_t { portNONE, portSERIAL, portPARALLEL };
  PortType_t GetPortType() const {
    return m_portType;
  }
  LPCSTR GetPort() const {
    return m_port;
  }
  void SetPort(LPCSTR port) {
    strncpy(m_port, port, sizeof(m_port));
  }
  int GetPortSpeed() const {
    return m_portSpeed;
  }
  void SetPortSpeed(int speed) {
    m_portSpeed = speed;
  }

  BOOL HasContrast() {
    return DeviceHasContrast();
  }
  int GetContrast() const {
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
  int GetBrightness() const {
    return m_brightness;
  }
  void SetBrightness(int percent) {
    m_brightness = percent;
  }

  BOOL HasKeypad() {
    return DeviceHasKeypad();
  }
  BOOL GetEnableInput() const {
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
  PVOID Save();
  void Restore(PVOID state);
  void Test();

protected:
  typedef DisplayDevice *(LCD_DECL *CreateFun_t)(HWND parent, LPCSTR devname);
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
  void DefineCustomCharacterInternal(int index, const CustomCharacter& cust);
  void SetSimulatedMarquee();
  void ClearSimulatedMarquee();
  void StepSimulatedMarquee();
  void SetMarqueeInternal(Marquee *marquee);
  void CheckMarqueeOverlap(int row);
  friend void WINAPI MarqueeTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

  BOOL OpenSerial(BOOL asynch = FALSE);
  BOOL WriteSerial(LPBYTE data, size_t len);
  BOOL EnableSerialInput();
  void DisableSerialInput();
  void CloseSerial();
  friend DWORD WINAPI SerialInputThread(LPVOID lpParam);

  // State.
  BYTE m_characterMap[256];

  DisplayBuffer *m_buffer;
  BOOL m_open;
  int m_cols, m_rows;

  Marquee *m_marquee;
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
LCD_API void LCD_DECL DisplayWin32Error(HWND parent, DWORD dwErr);
LCD_API void LCD_DECL DisplaySendEvent(LPCSTR event);
}
