#define PLUGINNUM 15
#define PLUGINNAME "DVDSpy 1.13"
#define GIR_ERROR -10;

#define _BOOL BYTE

typedef PCHAR (__stdcall  *TranslateFunct) (PCHAR eng);

typedef void   (WINAPI *PShowOSD)      (int timer);
typedef void   (WINAPI *PHideOSD)      ();
typedef _BOOL  (WINAPI *PStartOSDDraw)    (HWND *hw, HDC *h, int user);
typedef void   (WINAPI *PStopOSDDraw)    (HDC h);
typedef int    (WINAPI *PSetGirderReg)    (long val, int reg);
typedef long   (WINAPI *PGetGirderReg)    (int reg);
typedef PCHAR  (WINAPI *PParseRegString)  (PCHAR orig);
typedef int    (WINAPI *PSetGirderStrReg)  (const char *val, int reg);
// Copy the PCHAR immediately!!
typedef PCHAR  (WINAPI *PGetGirderStrReg)  (int reg);

// addition support functions
typedef struct s_HFunctions1 {  // Hardware plugin support functions version 1
  DWORD        dwSize;        // size of structure
  PSetGirderReg    SetGirderReg;    // Set numeric register
  PGetGirderReg    GetGirderReg;    // get numeric register
  PSetGirderStrReg  SetGirderStrReg;  // set string register
  PGetGirderStrReg  GetGirderStrReg;  // get string register ! COPY immediately after return !
  PParseRegString    ParseRegString;    // parse the string ! COPY immediately after return !
  PShowOSD      ShowOSD;      // currently UNUSED!!
  PHideOSD      HideOSD;      // currently UNUSED!!
  PStartOSDDraw    StartOSDDraw;    // currently UNUSED!!
  PStopOSDDraw    StopOSDDraw;    // currently UNUSED!!
} sHFunctions1;

#ifdef GIRDER_CPP

HINSTANCE hInstance;

TranslateFunct I18NTranslate;

sHFunctions1 HFunctions;

#else

extern HINSTANCE hInstance;

extern TranslateFunct I18NTranslate;

extern sHFunctions1 HFunctions;

#endif

#ifdef UI_CPP

HWND hConfigDialog;
HWND hLearnDialog;

#else

extern HWND hConfigDialog;
extern HWND hLearnDialog;

#endif

extern void GirderEvent(PCHAR event, PCHAR sarg = NULL, long iarg = 0);
extern void GetDVDTitle(LPCSTR disc, LPSTR title, size_t tsize, LPDWORD volser);
