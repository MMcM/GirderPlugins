/* LcdHype device implementation
$Header$
*/

#include "stdafx.h"
#include <stdlib.h>
#include "win_glyph.h"

HINSTANCE g_hInstance;

#define countof(x) sizeof(x)/sizeof(x[0])
#define MAX_RESPIXELS (320*240)

typedef __int8 TArrayType[MAX_RESPIXELS];

struct TSymbolInfo {
  __int8 SymbolData[20];
  };

struct TCharData {
  __int8 position;
  __int8 data[8][8];
  };

struct TDLLInfo {
  char IDArray[256];
  __int8 SupportGfxLCD;
  __int8 SupportTxtLCD;
  __int8 SupportLightSlider; 
  __int8 SupportContrastSlider; 
  __int8 SupportOutports;	  
  __int8 CCharWidth;
  __int8 CCharHeight;
  __int8 FontPitch;
};


typedef (__stdcall  *TDLL_GetInfo) (TDLLInfo* P_TDLLInfo);
typedef __int8 (__stdcall  *TLCD_IsReadyToReceive) ();
typedef __int8 (__stdcall  *TLCD_GetCGRAMChar) (__int8 position);
typedef (__stdcall  *TLCD_SetCGRAMChar) (TCharData* cdata);
typedef (__stdcall  *TLCD_Customize) (TSymbolInfo* SymData);
typedef (__stdcall  *TLCD_SendToController) (__int8 value);
typedef (__stdcall  *TLCD_SendToMemory) (__int8 value);
typedef (__stdcall  *TLCD_Init) ();
typedef (__stdcall  *TLCD_CleanUp) ();
typedef (__stdcall  *TLCD_LCD_ConfigDialog) ();
typedef (__stdcall  *TLCD_SendToGfxMemory) (TArrayType* Pixels, __int16 X1,__int16 Y1,__int16 X2,__int16 Y2,__int8 inverted);
typedef (__stdcall  *TLCD_SetOutputAddress) (__int32 x,__int32 y);
typedef (__stdcall  *TLCD_SetIOPropertys) (char *port,__int32 Exectime,__int32 ExectimeGfx,
                                           __int32 x,__int32 y,__int32 gx,__int32 gy,__int8 LightOn,
                                           __int8 LightSliderValue,__int8 ContrastOn,__int8 ContrastSliderValue,
										   __int32 Outports,__int8 UnderlineMode,__int8 UnderlineOutput);

struct TDLLInstance {
  int refcnt;
  HINSTANCE h;
  TDLLInfo DLLInfo;
  TDLL_GetInfo DLL_GetInfo;
  TLCD_SetIOPropertys LCD_SetIOPropertys;
  TLCD_Init LCD_Init;
  TLCD_CleanUp LCD_CleanUp;
  TLCD_LCD_ConfigDialog LCD_ConfigDialog;
  TLCD_SetOutputAddress LCD_SetOutputAddress;
  TLCD_SendToMemory LCD_SendToMemory;
  TLCD_Customize LCD_Customize;
  TLCD_SendToGfxMemory LCD_SendToGfxMemory;
  TLCD_SendToController LCD_SendToController;
  TLCD_SetCGRAMChar LCD_SetCGRAMChar;
  TLCD_GetCGRAMChar LCD_GetCGRAMChar;
  TLCD_IsReadyToReceive LCD_IsReadyToReceive;
};

struct DeviceEntry {
  const char *devtype;
  int cols, rows;
  const char *devdll;
} DeviceEntries[] = {
  { "LC798125664", 256, 64 , "hd61830.dll" },
  { "LC798124128", 240, 128, "hd61830.dll" },
  { "LC798124064", 240, 64 , "hd61830.dll" },
  { "T6963C24128", 240, 128, "t6963c.dll" },
  { "T6963C24064", 240, 64 , "t6963c.dll" },
  { "T6963C12128", 128, 128, "t6963c.dll" },
  { "T6963C12864", 128, 64 , "t6963c.dll" },
  { "KS010812864", 128, 64 , "ks0108_128x64.dll" },
  { "GU700011216", 112, 16 , "noritake_gu7000.dll" },
  { "GU700012632", 126, 32 , "noritake_gu7000.dll" },
  { "GU700014032", 140, 32 , "noritake_gu7000.dll" },
  { "SED15309632", 96,  32 , "sed1530.dll" },
  { "SED152012232",122, 32 , "sed1520.dll" },
  { "SED133x32240",320, 240, "sed133x.dll" },
  { "SED133x24128",240, 128, "sed133x.dll" },
};

class LCDHypeDisplay : public DisplayDevice
{
public:
  LCDHypeDisplay(DisplayDeviceFactory *factory, LPCSTR devtype);
  LCDHypeDisplay(const LCDHypeDisplay& other);
  ~LCDHypeDisplay();
  
  DisplayDevice *Duplicate() const;
  virtual void DeviceDisplay(int row, int col, LPCBYTE str, int length);
  virtual BOOL DeviceOpen();
  virtual void DeviceClose();
  virtual void DeviceClear();
  virtual BOOL DeviceHasSetSize();
  virtual void DeviceLoadSettings(HKEY hkey);
  virtual void DeviceSaveSettings(HKEY hkey);
  virtual void DeviceDefineCustomCharacter(int index, const CustomCharacter& cust);

protected:
  void LCD_Font();
  void Load_Export();

  TDLLInstance *m_dll;

  char port[8];
  unsigned char* grbuff;
  int  height,width;
  int  fontHeight,fontWidth;
  HFONT font;
  int m_fontSize,m_fontSizeW;
  char m_fontName[LF_FACESIZE], m_fontStyle[LF_FACESIZE];
};

LCDHypeDisplay::LCDHypeDisplay(DisplayDeviceFactory *factory, LPCSTR devtype)
  : DisplayDevice(factory, devtype)
{
  m_dll = new TDLLInstance;
  m_dll->refcnt = 1;
  m_dll->h = NULL;

  char cDllPath[MAX_PATH];
  GetModuleFileName(NULL, cDllPath, sizeof(cDllPath));

  for (int i = 0; i < countof(DeviceEntries); i++) {
    DeviceEntry *entry = DeviceEntries + i;
    if (!strcmp(devtype, entry->devtype)) {
      height = entry->rows;
      width  = entry->cols;

      if(strrchr(cDllPath,'\\')) {
        strcpy(strrchr(cDllPath,'\\')+1, entry->devdll);}
      else {
        strcpy(cDllPath, entry->devdll);}

      m_cols=0;
      m_rows=0;
      m_portType=portNONE;
      m_dll->h = LoadLibrary(cDllPath);
      if ( m_dll->h != NULL ) {
        m_dll->DLL_GetInfo = (TDLL_GetInfo)GetProcAddress(m_dll->h, "DLL_GetInfo");
        if (m_dll->DLL_GetInfo == NULL ) {
          FreeLibrary(m_dll->h);
          m_dll->h=NULL;}
        else {
          m_portType = portPARALLEL;
          strcpy(m_port, "LPT1");
          m_dll->DLL_GetInfo(&m_dll->DLLInfo);
          if (m_dll->DLLInfo.SupportTxtLCD) {
            m_cols=width/m_dll->DLLInfo.FontPitch;
            m_rows=height/m_dll->DLLInfo.CCharHeight;
          }
          Load_Export();}
      }  
      break;
    }
  }

  m_fontSize=10;
  m_fontSizeW=0;
  strncpy(m_fontName, "Lucida Console", sizeof(m_fontName));
  strncpy(m_fontStyle, "Regular", sizeof(m_fontStyle));
}

void LCDHypeDisplay::Load_Export()
{
  m_dll->LCD_SetIOPropertys=(TLCD_SetIOPropertys)GetProcAddress(m_dll->h, "LCD_SetIOPropertys");
  m_dll->LCD_Init=(TLCD_Init)GetProcAddress(m_dll->h, "LCD_Init");
  m_dll->LCD_CleanUp=(TLCD_CleanUp)GetProcAddress(m_dll->h, "LCD_CleanUp");
  m_dll->LCD_ConfigDialog=(TLCD_LCD_ConfigDialog)GetProcAddress(m_dll->h, "LCD_ConfigDialog");
  m_dll->LCD_SetOutputAddress=(TLCD_SetOutputAddress)GetProcAddress(m_dll->h, "LCD_SetOutputAddress");
  m_dll->LCD_SendToMemory=(TLCD_SendToMemory)GetProcAddress(m_dll->h, "LCD_SendToMemory");
  m_dll->LCD_Customize=(TLCD_Customize)GetProcAddress(m_dll->h, "LCD_Customize");
  m_dll->LCD_SendToGfxMemory=(TLCD_SendToGfxMemory)GetProcAddress(m_dll->h, "LCD_SendToGfxMemory");
  m_dll->LCD_SendToController=(TLCD_SendToController)GetProcAddress(m_dll->h, "LCD_SendToController");
  m_dll->LCD_SetCGRAMChar=(TLCD_SetCGRAMChar)GetProcAddress(m_dll->h, "LCD_SetCGRAMChar");
  m_dll->LCD_GetCGRAMChar=(TLCD_GetCGRAMChar)GetProcAddress(m_dll->h, "LCD_GetCGRAMChar");
  m_dll->LCD_IsReadyToReceive=(TLCD_IsReadyToReceive)GetProcAddress(m_dll->h, "LCD_IsReadyToReceive");
}

LCDHypeDisplay::LCDHypeDisplay(const LCDHypeDisplay& other)
  : DisplayDevice(other)
{
  m_dll = other.m_dll;
  m_dll->refcnt++;
  m_cols = other.m_cols;
  m_rows = other.m_rows;
  height = other.height;
  width = other.width;
  fontHeight = other.fontHeight;
  fontWidth = other.fontWidth;
  m_fontSize = other.m_fontSize;
  m_fontSizeW = other.m_fontSizeW;
  strncpy(m_fontName, other.m_fontName, sizeof(m_fontName));
  strncpy(m_fontStyle, other.m_fontStyle, sizeof(m_fontStyle));
}


LCDHypeDisplay::~LCDHypeDisplay()
{
  if (--m_dll->refcnt <= 0) {
    if ( m_dll->h != NULL ) { 
      FreeLibrary(m_dll->h);
    }
    delete m_dll;
  }
}

DisplayDevice* LCDHypeDisplay::Duplicate() const
{
  return new LCDHypeDisplay(*this);
}

BOOL LCDHypeDisplay::DeviceHasSetSize()
{
  return FALSE;
}

BOOL LCDHypeDisplay::DeviceOpen()
{
  if ( m_dll->h == NULL ) {
    return FALSE;}

  if (!strcmp(m_port, "LPT1")) {
    strcpy(port, "378");}
  else if (!strcmp(m_port, "LPT2")){
    strcpy(port, "278");}
  else if (!strcmp(m_port, "LPT3")){
    strcpy(port, "3BC");}
  else {
    strcpy(port, m_port);}

  grbuff = new unsigned char[MAX_RESPIXELS];

  m_cols=0;
  m_rows=0;
  if (m_dll->DLLInfo.SupportTxtLCD) {
    m_cols=width/m_dll->DLLInfo.FontPitch;
    m_rows=height/m_dll->DLLInfo.CCharHeight;
  }

  m_dll->LCD_SetIOPropertys(port,1,1,m_cols,m_rows,width,height,false,127,false,127,0,false,false);
  m_dll->LCD_Init();

  DeviceClear();

  return TRUE;
}

void LCDHypeDisplay::DeviceClose()
{
  DeviceClear();
  m_dll->LCD_CleanUp();
  if (font) {
    DeleteObject(font);}
  delete [] grbuff;
}

void LCDHypeDisplay::DeviceClear()
{
  LCD_Font();

  for (int i = 0; i < width*height; i++) {
    grbuff[i]=0;}
  m_dll->LCD_SendToGfxMemory((TArrayType*)grbuff,0,0,width-1,height-1,false);
}

void LCDHypeDisplay::LCD_Font()
{
  // Font parameters
  //------------------------
  fontHeight      = m_fontSize;  
  fontWidth       = m_fontSizeW;
  bool bold            = (strnicmp(m_fontStyle,"bold",4)==0) ? true : false;
  bool italic          = false;
  const char* typeFace = m_fontName;

  if (font) {
    DeleteObject(font);}

  font = CreateFont(fontHeight,               // height of font
                    fontWidth,                // average character width
                    0,                        // angle of escapement
                    0,                        // base-line orientation angle
                    bold ? FW_BOLD : FW_NORMAL,       // font weight
                    italic,                   // italic attribute option
                    FALSE,                    // underline attribute option
                    FALSE,                    // strikeout attribute option
                    DEFAULT_CHARSET,          // character set identifier
                    OUT_DEFAULT_PRECIS,       // output precision
                    CLIP_DEFAULT_PRECIS,      // clipping precision
                    DEFAULT_QUALITY,          // output quality
                    FF_DONTCARE|DEFAULT_PITCH,        // pitch and family
                    typeFace);                // typeface name

  TEXTMETRIC tm;
  HDC hDC = GetDC(0);
  if (hDC) {
    HGDIOBJ hOld = SelectObject(hDC, font); 
    GetTextMetrics(hDC, &tm);
    SelectObject(hDC, hOld);
    ReleaseDC(0, hDC);

    fontWidth   = tm.tmAveCharWidth;
    fontHeight  = tm.tmHeight-tm.tmInternalLeading;
    m_cols=width/fontWidth;
    m_rows=height/fontHeight;

  }
}

void LCDHypeDisplay::DeviceDisplay(int row, int col, LPCBYTE str, int length)
{
  char tempstr[50];
  if (font) {
    memcpy(tempstr,str,length);
    tempstr[length]=0;
    for (int i = 0; i < length; i++) {
      // TODO: Use custom bitmaps like SIMLCD.
      if ((tempstr[i]<0x10) && (tempstr[i]>0x00))
	   {tempstr[i]=0x20;}
	}
    work(font,grbuff,width,height,col*fontWidth,(row+1)*fontHeight,tempstr);
    m_dll->LCD_SendToGfxMemory((TArrayType*)grbuff,0,0,width-1,height-1,false);
  }
}

void LCDHypeDisplay::DeviceLoadSettings(HKEY hkey)
{
  GetSettingInt(hkey, "SimLCDFontSize", m_fontSize);
  GetSettingInt(hkey, "SimLCDFontSizeW", m_fontSizeW);
  GetSettingString(hkey, "SimLCDFont", m_fontName, sizeof(m_fontName));
  GetSettingString(hkey, "SimLCDFontStyle", m_fontStyle, sizeof(m_fontStyle));
}

void LCDHypeDisplay::DeviceSaveSettings(HKEY hkey)
{
  SetSettingInt(hkey, "SimLCDFontSize", m_fontSize);
  SetSettingInt(hkey, "SimLCDFontSizeW", m_fontSizeW);
  SetSettingString(hkey, "SimLCDFont", m_fontName);
  SetSettingString(hkey, "SimLCDFontStyle", m_fontStyle);
}

void LCDHypeDisplay::DeviceDefineCustomCharacter(int index, 
                                                 const CustomCharacter& cust)
{
  // TODO: Use custom bitmaps like SIMLCD.
}

extern "C" __declspec(dllexport)
DisplayDevice *CreateDisplayDevice(DisplayDeviceFactory *factory, LPCSTR devtype)
{
  return new LCDHypeDisplay(factory, devtype);
}

/* Called by windows */
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason,  LPVOID lpReserved)
{
  switch(dwReason) {
  case DLL_PROCESS_ATTACH:
    g_hInstance = (HINSTANCE)hModule;
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}
