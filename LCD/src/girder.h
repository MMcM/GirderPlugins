/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.0 Plugin                                                                  */
/*  User interface                                                                     */
/*                                                                                     */
/*  Copyright 2000 (c) Ron Bessems                                                     */
/*  GNU General Public License                                                         */
/*                                                                                     */
/*                                                                                     */
/***************************************************************************************/

#define PLUGINNUM 118
#define PLUGINNAME "LCD 1.6"
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
typedef PCHAR  (WINAPI *PI18NTranslate)	(PCHAR trans); 
typedef PCHAR  (WINAPI *PSetCommand)	(TCommand * command); 
typedef PCHAR  (WINAPI *PTargetEnum)	(TCommand * command, PTargetCallBack back); 
typedef void   (WINAPI *PReallocPCHAR)	(PCHAR * old, PCHAR newchar);
typedef void   (WINAPI *PShowOSD)		(int timer);
typedef void   (WINAPI *PHideOSD)		();
typedef _BOOL  (WINAPI *PStartOSDDraw)	(HWND *hw, HDC *h, int user);
typedef void   (WINAPI *PStopOSDDraw)	(HDC h);
typedef int    (WINAPI *PTreePickerShow)(int id);
// v2
typedef int    (WINAPI *PSetGirderReg)	(long val, int reg);
typedef long   (WINAPI *PGetGirderReg)	(int reg);
typedef PCHAR  (WINAPI *PParseRegString)(PCHAR orig);
typedef PCHAR  (WINAPI *PGetLinkName)   (int lvalue);
// v3
typedef _BOOL  (WINAPI *PEventCB)       (PCHAR event_string, int device);
typedef _BOOL  (WINAPI *PRegisterCB)    (int actionplugin, PEventCB callback, PCHAR prefix, int device);
typedef int    (WINAPI *PSetGirderStrReg)(const char *val, int reg);
// Copy the PCHAR immediately!!
typedef PCHAR  (WINAPI *PGetGirderStrReg)(int reg);

/* Old type structure */
typedef struct s_TFunctions {	
			PSetCommand		SetCommand;
			PTargetEnum		TargetEnum;
			PI18NTranslate	I18NTranslate;
			PReallocPCHAR	ReallocPchar;
			PShowOSD		ShowOSD;
			PHideOSD		HideOSD;
			PStartOSDDraw	StartOSDDraw;
			PStopOSDDraw	StopOSDDraw;
			PTreePickerShow	TreePickerShow;
			HWND			TargetHWND;
} TFunctions;

/* I introduced this new type to be able to extend the
   API more easily without breaking compatibility to older
   plugins
*/

/* New Type ! */ 
typedef struct s_TFunctionsEx1 {   /* Version 1 */
	        DWORD           dwSize;
			PSetCommand		SetCommand;
			PTargetEnum		TargetEnum;
			PI18NTranslate	I18NTranslate;
			PReallocPCHAR	ReallocPchar;
			PShowOSD		ShowOSD;
			PHideOSD		HideOSD;
			PStartOSDDraw	StartOSDDraw;
			PStopOSDDraw	StopOSDDraw;
			PTreePickerShow	TreePickerShow;
			HWND			TargetHWND;
} TFunctionsEx1;


typedef struct s_TFunctionsEx2 {	/* Version 2 */
	        DWORD			dwSize;
			PSetGirderReg   SetGirderReg;
			PGetGirderReg   GetGirderReg;
			PParseRegString ParseRegString;
			PGetLinkName	GetLinkName;
			PSetCommand		SetCommand;
			PTargetEnum		TargetEnum;
			PI18NTranslate	I18NTranslate;
			PReallocPCHAR	ReallocPchar;
			PShowOSD		ShowOSD;
			PHideOSD		HideOSD;
			PStartOSDDraw	StartOSDDraw;
			PStopOSDDraw	StopOSDDraw;
			PTreePickerShow	TreePickerShow;
			HWND			TargetHWND;
} TFunctionsEx2;


typedef struct s_TFunctionsEx3 {	/* Version 3 */
	        DWORD			dwSize;
			PSetGirderReg   SetGirderReg;
			PGetGirderReg   GetGirderReg;
			PParseRegString ParseRegString;
			PGetLinkName	GetLinkName;
			PSetCommand		SetCommand;
			PTargetEnum		TargetEnum;
			PI18NTranslate	I18NTranslate;
			PReallocPCHAR	ReallocPchar;
			PShowOSD		ShowOSD;
			PHideOSD		HideOSD;
			PStartOSDDraw	StartOSDDraw;
			PStopOSDDraw	StopOSDDraw;
			PTreePickerShow	TreePickerShow;
			PRegisterCB     RegisterCB;
			HWND			TargetHWND;
} TFunctionsEx3;

typedef struct s_TFunctionsEx4 {	/* Version 4 */
	        DWORD				dwSize;
			PSetGirderReg		SetGirderReg;
			PGetGirderReg		GetGirderReg;
			PParseRegString		ParseRegString;
			PGetLinkName		GetLinkName;
			PSetCommand			SetCommand;
			PTargetEnum			TargetEnum;
			PI18NTranslate		I18NTranslate;
			PReallocPCHAR		ReallocPchar;
			PShowOSD			ShowOSD;
			PHideOSD			HideOSD;
			PStartOSDDraw		StartOSDDraw;
			PStopOSDDraw		StopOSDDraw;
			PTreePickerShow		TreePickerShow;
			PRegisterCB			RegisterCB;
			PSetGirderStrReg	SetGirderStrReg;
			PGetGirderStrReg	GetGirderStrReg;
			HWND				TargetHWND;
} TFunctionsEx4;


#ifdef GIRDER_CPP
	TCommand				*CurCommand;		
	PSetGirderReg			SetGirderReg;
	PGetGirderReg			GetGirderReg;
	PParseRegString			ParseRegString;	
	PGetLinkName			GetLinkName;
	PSetCommand				SetCommand;
	PTargetEnum				TargetEnum;
	PI18NTranslate			I18NTranslate;
	PReallocPCHAR			ReallocPchar;
	PShowOSD				ShowOSD;
	PHideOSD				HideOSD;
	PStartOSDDraw			StartOSDDraw;
	PStopOSDDraw			StopOSDDraw;
	PTreePickerShow			TreePickerShow;
	PRegisterCB				RegisterCB;
	PSetGirderStrReg		SetGirderStrReg;
	PGetGirderStrReg		GetGirderStrReg;
	HWND					TargetHWND;
	HINSTANCE				hInstance;
#endif

#ifndef GIRDER_CPP
	extern TCommand			*CurCommand;		
	extern PSetGirderReg	SetGirderReg;
	extern PGetGirderReg	GetGirderReg;		
	extern PParseRegString	ParseRegString;
	extern PGetLinkName		GetLinkName;
	extern PSetCommand		SetCommand;
	extern PTargetEnum		TargetEnum;
	extern PI18NTranslate	I18NTranslate;
	extern PReallocPCHAR	ReallocPchar;
	extern PShowOSD			ShowOSD;
	extern PHideOSD			HideOSD;
	extern PStartOSDDraw	StartOSDDraw;
	extern PStopOSDDraw		StopOSDDraw;
	extern PTreePickerShow	TreePickerShow;
	extern HWND				TargetHWND;
	extern PRegisterCB		RegisterCB;
	extern PSetGirderStrReg	SetGirderStrReg;
	extern PGetGirderStrReg	GetGirderStrReg;
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
extern void DisplayFilenameRegister(TCommand *command, PCHAR retbuf);
