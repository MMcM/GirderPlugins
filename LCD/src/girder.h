/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.1 Plugin                                                                  */
/*  User interface                                                                     */
/*                                                                                     */
/*  Copyright 2000 (c) Ron Bessems                                                     */
/*  GNU General Public License                                                         */
/*                                                                                     */
/*                                                                                     */
/***************************************************************************************/

#define PLUGINNUM 118
#define PLUGINNAME "LCD 1.8"
#define GIR_ERROR -10;

#define APIEX

#define _BOOL BYTE

typedef struct s_TCommand {
	PCHAR			name;
	_BOOL			enabled;
	PCHAR			irstr;
	int				antirepeat;
	//target
	PCHAR			target_name;
	PCHAR			target_class;
	PCHAR			target_exe;
	PCHAR			ctarget_name;
	PCHAR			ctarget_class;
	_BOOL			submatch;
	_BOOL			topmost;
	BYTE			matchby;
	_BOOL			onematch;
	_BOOL			matchhidden;
	BYTE			statecount;
	BYTE			statebegin;
	_BOOL			stateinvert;
	int				statecurrent;
	// types
	BYTE			treetype;
	BYTE			actiontype;
	BYTE			actionsubtype;
	PCHAR			svalue1;
	PCHAR			svalue2;
	_BOOL			bvalue1;
	_BOOL			bvalue2;
	_BOOL			bvalue3;
	int				ivalue1;
	int				ivalue2;
	int				lvalue1;
	int				lvalue2;
	int				lvalue3;

} TCommand;

typedef  void  (WINAPI *PTargetCallBack)(HWND hw, TCommand *command);
typedef PCHAR  (WINAPI *PSetCommand)	(TCommand * command); 
typedef PCHAR  (WINAPI *PTargetEnum)	(TCommand * command, PTargetCallBack back); 
typedef void   (WINAPI *PReallocPCHAR)	(PCHAR * old, PCHAR newchar);
typedef void   (WINAPI *PShowOSD)		(int timer);
typedef void   (WINAPI *PHideOSD)		();
typedef _BOOL  (WINAPI *PStartOSDDraw)	(HWND *hw, HDC *h, int user);
typedef void   (WINAPI *PStopOSDDraw)	(HDC h);
typedef int    (WINAPI *PTreePickerShow)(int id);
typedef int    (WINAPI *PSetGirderReg)	(long val, int reg);
typedef long   (WINAPI *PGetGirderReg)	(int reg);
typedef _BOOL  (WINAPI *PEventCB)       (PCHAR event_string, int device);
typedef _BOOL  (WINAPI *PRegisterCB)    (int actionplugin, PEventCB callback, PCHAR prefix, int device);
typedef int    (WINAPI *PSetGirderStrRegister)	(PCHAR val, int reg);
typedef _BOOL  (WINAPI *PGetGirderStrRegEx ) ( int reg, PCHAR szstore, int size);
typedef _BOOL  (WINAPI *PGetLinkNameEx     ) ( int lvalue, PCHAR szstore, int size);
typedef _BOOL  (WINAPI *PParseGirderRegEx  ) ( PCHAR orig, PCHAR szstore, int size);
typedef int    (WINAPI *PGetPayload        ) ( PCHAR szstore, int size);
typedef _BOOL  (WINAPI *PI18NTranslateEx   ) ( PCHAR orig, PCHAR szstore, int size);
typedef DWORD   (WINAPI *PGetOSDSettings    ) ( int setting);
typedef _BOOL  (WINAPI *PGetOSDFontName    ) ( PCHAR szstore, int size);

typedef struct s_TFunctionsEx5 {	/* Version 5 */
	      DWORD			dwSize;
			PSetGirderReg   SetGirderReg;
			PGetGirderReg   GetGirderReg;
			PParseGirderRegEx ParseRegStringEx;
			PGetLinkNameEx	GetLinkNameEx;
			PSetCommand		SetCommand;
			PTargetEnum		TargetEnum;
		
			PReallocPCHAR	ReallocPchar;
			PShowOSD		ShowOSD;
			PHideOSD		HideOSD;
			PStartOSDDraw	StartOSDDraw;
			PStopOSDDraw	StopOSDDraw;
			PTreePickerShow	TreePickerShow;
			PRegisterCB     RegisterCB;
         PSetGirderStrRegister SetGirderStrRegister;
         PGetGirderStrRegEx GetGirderStrRegisterEx;
         PGetPayload GetPayload;
         PI18NTranslateEx I18NTranslateEx;
         PGetOSDSettings GetOSDSettings;
         PGetOSDFontName GetOSDFontName;
			HWND			TargetHWND;
} TFunctionsEx5;


#ifdef GIRDER_CPP
	TCommand				*CurCommand;
   TFunctionsEx5     SF;
	HINSTANCE				hInstance;
#endif

#ifndef GIRDER_CPP
	extern TCommand			*CurCommand;		
   extern TFunctionsEx5     SF;
	extern HINSTANCE		hInstance;
#endif


/* Commmon routines */

extern int nDisplayCols, nDisplayRows;
extern LPSTR pDisplayBuf;

enum DisplayValueType {
  valNONE, valSTR, valINT
};

struct DisplayAction
{
  const char *name;
  DisplayValueType valueType;
  void (*function)(TCommand *command, PCHAR retbuf);
};

extern DisplayAction DisplayActions[];
extern void DisplayClose(TCommand *command, PCHAR retbuf);
extern void DisplayClear(TCommand *command, PCHAR retbuf);
extern void DisplayString(TCommand *command, PCHAR retbuf);
extern void DisplayStringRegister(TCommand *command, PCHAR retbuf);
extern void DisplayCurrentTime(TCommand *command, PCHAR retbuf);
extern void DisplayPayload(TCommand *command, PCHAR retbuf);
extern void DisplayFilenamePayload(TCommand *command, PCHAR retbuf);
