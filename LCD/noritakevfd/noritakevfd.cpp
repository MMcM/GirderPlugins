/* Noritake VFD device implementation 
$Header$
*/

struct DeviceEntry {
  const char *devtype;
  int cols;
  int rows;
} DeviceEntries[] = {
  { "CU20025SCPB", 20, 2 },
  { "CU20045SCPB", 20, 4 },
  { "CU40026SCPB", 40, 2 },
};

#define countof(x) sizeof(x)/sizeof(x[0])

class NoritakeVFDDisplay : public DisplayDevice
{
public:
  NoritakeVFDDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  NoritakeVFDDisplay(const NoritakeVFDDisplay& other);
  ~NoritakeVFDDisplay();
  
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);
  virtual BOOL DeviceHasBrightness();
  virtual void DeviceWriteRaw(LPBYTE data, DWORD len);

  virtual BOOL Write(LPBYTE data, DWORD len) = 0;

protected:
  Delay m_extraDelay;
};

NoritakeVFDDisplay::NoritakeVFDDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  for (int i = 0; i < countof(DeviceEntries); i++) {
    DeviceEntry *entry = DeviceEntries + i;
    if (!strcmp(devtype, entry->devtype)) {
      m_cols = entry->cols;
      m_rows = entry->rows;
      break;
    }
  }
  m_extraDelay = .001;
}

NoritakeVFDDisplay::NoritakeVFDDisplay(const NoritakeVFDDisplay& other)
  : DisplayDevice(other)
{
}

NoritakeVFDDisplay::~NoritakeVFDDisplay()
{
}

#define ESC 0x1B

BOOL NoritakeVFDDisplay::DeviceOpen()
{
  BYTE buf[128];
  int nb = 0;

  m_extraDelay.Wait();

  buf[nb++] = ESC;
  buf[nb++] = 'I';              // Initialize display 'I' 0x49 (clear and cursor on at top left)
  Write(buf, nb);

  m_extraDelay.Wait();
  m_extraDelay.Wait();

  nb = 0;
  buf[nb++] = 0x16;             // Cursor Off
  buf[nb++] = 0x0E;             // Clear Display
  buf[nb++] = 0x11;             // Normal Mode
  buf[nb++] = ESC;
  buf[nb++] = 'L';              // Luminance Control
  buf[nb++] = (m_brightness * 0xFF) / 100;
  Write(buf, nb);

  return TRUE;
}

void NoritakeVFDDisplay::DeviceClose()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x0E;             // Clear Display
  buf[nb++] = 0x16;             // Cursor Off
  buf[nb++] = ESC;              // Esc
  buf[nb++] = 'H';              // 'H' Cursor move 0x48
  buf[nb++] = 0x00;             // Return home
  Write(buf, nb);
}

void NoritakeVFDDisplay::DeviceClear()
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = 0x0E;             // Clear Display
  buf[nb++] = ESC;              // Esc
  buf[nb++] = 'H';              // 'H' Cursor move 0x48
  buf[nb++] = 0x00;             // Return home
  Write(buf, nb);
  m_extraDelay.Wait();
}

void NoritakeVFDDisplay::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = ESC;
  buf[nb++] = 'H';              // Position Cursor
  buf[nb++] = (row * m_cols) + col;
  for (int i = 0; i < length; i++) {
    BYTE b = str[i];
    buf[nb++] = b;
  }
  Write(buf, nb);
}

void NoritakeVFDDisplay::DeviceDefineCustomCharacter(int index, 
                                                     const CustomCharacter& cust)
{
  BYTE buf[128];
  int nb = 0;
  buf[nb++] = ESC;
  buf[nb++] = 'C';              // Set User Font
  buf[nb++] = index;
  // LSB on left; densely packed.
  memset(buf+nb, 0, 5);
  for (int i = 0; i < 7; i++) {
    BYTE b = cust.GetBits()[i];
    for (int j = 0; j < 5; j++) {
      if ((b >> (4-j)) & 1) {
        int bit = i*5+j;
        buf[nb + (bit / 8)] |= (1 << (bit % 8));
      }
    }
  }
  nb += 5;
  WriteSerial(buf, nb);
}

BOOL NoritakeVFDDisplay::DeviceHasBrightness()
{
  return TRUE;
}

void NoritakeVFDDisplay::DeviceWriteRaw(LPBYTE data, DWORD len)
{
  Write(data, len);
}
