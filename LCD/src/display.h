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

  LPCBYTE GetBits() const {
    return m_bits;
  }
  void SetBits(LPCBYTE bits) {
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
  
  int FindCustomCharacter(const CustomCharacter& cust, int ncust);
  int AllocateCustomCharacter(const CustomCharacter& cust, int ncust, 
                              LPCBYTE characterMap);
  void UseCustomCharacter(int index);

protected:
  LPBYTE m_bytes;
  int m_rows, m_cols;
  BYTE m_space;

#define MAXCUSTCHARS 8
  CustomCharacter m_customCharacters[MAXCUSTCHARS];
  DWORD m_customLastUse[MAXCUSTCHARS];
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

class InputMapEntry;

class LCD_API InputMap
{
public:
  InputMap() {
    m_passUnknownInput = FALSE;
    m_entries = NULL;
  }
  InputMap(const InputMap& other) {
    Copy(other);
  }
  ~InputMap() {
    Clear();
  }
  InputMap& operator=(const InputMap& other) {
    Clear();
    Copy(other);
    return *this;
  }

  LPCSTR Get(LPCSTR input) const;
  void Put(LPCSTR input, LPCSTR event);
  void Clear();

  BOOL GetPassUnknownInput() const {
    return m_passUnknownInput;
  }
  void SetPassUnknownInput(BOOL passUnknownInput) {
    m_passUnknownInput = passUnknownInput;
  }

  BOOL Enum(PVOID& state, LPCSTR& input, LPCSTR& event) const;

  void LoadFromString(LPCSTR defn);

  void LoadFromRegistry(HKEY hkey);
  void SaveToRegistry(HKEY hkey);

protected:
  void Copy(const InputMap& other);

  BOOL m_passUnknownInput;
  InputMapEntry *m_entries;
};

class DisplayDeviceFactory;

class LCD_API DisplayDevice
{
public:
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

  virtual DisplayDevice *Duplicate() const = 0;
  virtual ~DisplayDevice();

  LPCSTR GetName() const {
    return m_name;
  }
  void SetName(LPCSTR name);
  const DisplayDeviceFactory *GetFactory() const {
    return m_factory;
  }
  LPCSTR GetDeviceType() const {
    return m_devtype;
  }
  DisplayDevice *GetNext() const {
    return m_next;
  }

  BOOL IsDefault() const {
    return m_default;
  }
  void SetDefault(BOOL defval) {
    m_default = defval;
  }
  BOOL IsEnabled() const {
    return m_enabled;
  }
  void SetEnabled(BOOL enabled) {
    m_enabled = enabled;
  }

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
  InputMap& GetInputMap() {
    return m_inputMap;
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
  BOOL HasKeypadLegends() {
    return DeviceHasKeypadLegends();
  }
  LPCSTR *GetKeypadButtonChoices() {
    return DeviceGetKeypadButtonChoices();
  }
  LPCSTR *GetKeypadLegendChoices() {
    return DeviceGetKeypadLegendChoices();
  }
  void SetKeypadLegend(LPCSTR button, LPCSTR legend) {
    DeviceSetKeypadLegend(button, legend);
  }

  int GetGPOs() {
    return DeviceGetGPOs();
  }
  void SetGPO(int gpo, BOOL on) {
    DeviceSetGPO(gpo, on);
  }

  HKEY GetSettingsKey();
  void LoadSettings(HKEY hkey);
  void SaveSettings(HKEY hkey);

  // Drawing.
  void Display(int row, int col, int width, LPCSTR str);
  void DisplayCharacter(int row, int col, char ch);
  void DisplayCustomCharacter(int row, int col, const CustomCharacter& cust);
  void Clear();
  PVOID Save();
  void Restore(PVOID state);
  void ResetInputMap();
  void Test();

protected:
  DisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype);
  DisplayDevice(const DisplayDevice& other);

  // Override these for each device.
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length) = 0;
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust) = 0;
  virtual int DeviceNCustomCharacters();
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
  virtual void DeviceSerialInputThread();
  virtual BOOL DeviceEnableInput();
  virtual void DeviceDisableInput();
  virtual void DeviceInput(BYTE b);
  virtual BOOL DeviceHasKeypadLegends();
  virtual LPCSTR *DeviceGetKeypadButtonChoices();
  virtual LPCSTR *DeviceGetKeypadLegendChoices();
  virtual void DeviceSetKeypadLegend(LPCSTR button, LPCSTR legend);
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

  void MapInput(LPCSTR input);

  BOOL OpenSerial(BOOL asynch = FALSE);
  BOOL WriteSerial(LPBYTE data, size_t len);
  BOOL EnableSerialInput();
  void DisableSerialInput();
  void CloseSerial();
  friend DWORD WINAPI SerialInputThread(LPVOID lpParam);

  friend class DisplayDeviceList;

  // Source.
  DisplayDeviceFactory *m_factory;
  LPSTR m_devtype;
  DisplayDevice *m_next;
  LPSTR m_name;
  BOOL m_default, m_enabled;

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

  InputMap m_inputMap;
  BOOL m_enableInput;
  HANDLE m_inputThread, m_inputStopEvent, m_outputEvent;
};

class LCD_API DisplayDeviceFactory
{
public:
  LPCSTR GetName() const {
    return m_name;
  }

  DisplayDevice *CreateDisplayDevice(LPCSTR devtype) {
    return (*m_entry)(this, devtype);
  }

  static DisplayDeviceFactory *GetFactory(LPCSTR name);
  static void CloseAll();
  static void InitCS() {
    InitializeCriticalSection(&g_CS);
  }
  static void DeleteCS() {
    DeleteCriticalSection(&g_CS);
  }

protected:
  typedef DisplayDevice *(LCD_DECL *CreateFun_t)(DisplayDeviceFactory *factory, 
                                                 LPCSTR devtype);
  DisplayDeviceFactory(LPCSTR name, HMODULE lib, CreateFun_t entry);
  ~DisplayDeviceFactory();

  static CRITICAL_SECTION g_CS;
  static DisplayDeviceFactory *g_extent;
  DisplayDeviceFactory *m_next;
  LPSTR m_name;
  HMODULE m_lib;
  CreateFun_t m_entry;
};

class LCD_API DisplayDeviceList
{
public:
  DisplayDeviceList() {
    m_head = m_tail = NULL;
    m_loaded = FALSE;
  }
  ~DisplayDeviceList() {
    Clear();
  }
  
  DisplayDevice *GetFirst() {
    return m_head;
  }
  DisplayDevice *GetDefault();
  DisplayDevice *Get(LPCSTR name);
  BOOL Contains(DisplayDevice *dev);
  
  void Clear();

  void Replace(DisplayDevice *odev, DisplayDevice *ndev);
  void SetDefault(DisplayDevice *dev);

  static HKEY LCD_DECL GetSettingsKey();
  void LoadFromRegistry(BOOL all = FALSE);
  void SaveToRegistry();

  PVOID Save();
  void Restore(PVOID state);

protected:
  DisplayDevice *LoadFromRegistry(HKEY hkey);
  void SaveToRegistry(HKEY hkey, DisplayDevice *dev);

  DisplayDevice *m_head, *m_tail;
  BOOL m_loaded;
};

class LCD_API Delay
{
public:
  Delay() {
    m_delay = 0;
    m_method = NONE;
  }
  Delay(double secs) {
    SetDelay(secs);
  }
  Delay(const Delay& other) {
    SetDelay(other.m_delay);
  }
  Delay& operator=(const Delay& other) {
    SetDelay(other.m_delay);
    return *this;
  }
  double operator=(double secs) {
    SetDelay(secs);
    return secs;
  }

  double GetDelay() const { 
    return m_delay;
  }
  void SetDelay(double secs) {
    m_delay = secs;
    m_method = UNKNOWN;
  }

  BOOL LoadSetting(HKEY hkey, LPCSTR valkey);
  void SaveSetting(HKEY hkey, LPCSTR valkey) const;
  
  void Wait() const;

protected:
  double m_delay;
  mutable enum { 
    UNKNOWN, NONE, SLEEP, HIRES_COUNTER 
    // Busy waiting in a loop would be another possibility.
  } m_method;
  mutable union {
    DWORD dwMillis;
    LONGLONG llCount;
  } m_arg;
};

extern "C" {
LCD_API HWND LCD_DECL DisplayWindowParent();
LCD_API void LCD_DECL DisplayWin32Error(DWORD dwErr, HWND parent = DisplayWindowParent());
LCD_API void LCD_DECL DisplaySendEvent(LPCSTR event, LPCSTR payload = NULL);
}
