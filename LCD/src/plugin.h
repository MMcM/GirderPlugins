/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.2 Plugin                                                                  */
/*  User interface                                                                     */
/*                                                                                     */
/*  Copyright 2000 (c) Ron Bessems                                                     */
/*  GNU General Public License                                                         */
/*                                                                                     */
/*                                                                                     */
/***************************************************************************************/

#define PLUGINNUM 118
#define PLUGINNAME "LCD"
#define PLUGINVERSION "1.10"


#include <girder.h>


#ifdef GIRDER_CPP
p_command CurCommand;
t_functions SF;
HINSTANCE hInstance;
#endif

#ifndef GIRDER_CPP
extern p_command CurCommand;		
extern t_functions SF;
extern HINSTANCE hInstance;
#endif


/* Commmon routines */

extern int nDisplayCols, nDisplayRows;
extern LPSTR pDisplayBuf;

class DisplayCommandState;

enum DisplayValueType {
  valNONE, valSTR, valINT
};

struct DisplayAction
{
  const char *name;
  DisplayValueType valueType;
  void (*function)(DisplayCommandState& state);
};

extern DisplayAction DisplayActions[];
extern void DisplayClose(DisplayCommandState& state);
extern void DisplayClear(DisplayCommandState& state);
extern void DisplayString(DisplayCommandState& state);
extern void DisplayStringRegister(DisplayCommandState& state);
extern void DisplayCurrentTime(DisplayCommandState& state);
extern void DisplayPayload(DisplayCommandState& state);
extern void DisplayFilenamePayload(DisplayCommandState& state);
