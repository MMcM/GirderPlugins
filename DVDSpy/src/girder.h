#define PLUGINNUM 15
#define PLUGINNAME "DVDSpy 1.21"
#define GIR_ERROR -10;

#define _BOOL BYTE

typedef void   (WINAPI *PShowOSD)      (int timer);
typedef void   (WINAPI *PHideOSD)      ();
typedef _BOOL  (WINAPI *PStartOSDDraw)    (HWND *hw, HDC *h, int user);
typedef void   (WINAPI *PStopOSDDraw)    (HDC h);
typedef int    (WINAPI *PSetGirderReg)    (long val, int reg);
typedef long   (WINAPI *PGetGirderReg)    (int reg);
typedef int    (WINAPI *PSetGirderStrReg)  (const char *val, int reg);
typedef _BOOL  (WINAPI *PGetGirderStrRegEx ) ( int reg, PCHAR szstore, int size);
typedef _BOOL  (WINAPI *PParseGirderRegEx  ) ( PCHAR orig, PCHAR szstore, int size);
typedef _BOOL  (WINAPI *PI18NTranslateEx   ) ( PCHAR orig, PCHAR szstore, int size);


// addition support functions
typedef struct s_HFunctions2 {  // Hardware plugin support functions version 1
  DWORD dwSize;                 // size of structure
  PSetGirderReg SetGirderReg;   // Set numeric register
  PGetGirderReg GetGirderReg;   // get numeric register
  PSetGirderStrReg SetGirderStrReg; // set string register
  PGetGirderStrRegEx GetGirderStrReg;  
  PParseGirderRegEx ParseRegString;    
  PShowOSD ShowOSD;             // currently UNUSED!!
  PHideOSD HideOSD;             // currently UNUSED!!
  PStartOSDDraw StartOSDDraw;   // currently UNUSED!!
  PStopOSDDraw StopOSDDraw;     // currently UNUSED!!
  PI18NTranslateEx I18NTranslateEx;
} sHFunctions2;

#ifdef GIRDER_CPP

HINSTANCE hInstance;

sHFunctions2 HFunctions;

#else

extern HINSTANCE hInstance;

extern sHFunctions2 HFunctions;

#endif

#ifdef UI_CPP

HWND hConfigDialog;
HWND hLearnDialog;

#else

extern HWND hConfigDialog;
extern HWND hLearnDialog;

#endif

extern void GirderEvent(PCHAR event, PCHAR payload = NULL, size_t pllen = 0);
extern void GetDVDTitle(LPCSTR disc, LPSTR title, size_t tsize, LPDWORD volser);
