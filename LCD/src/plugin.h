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
#define PLUGINVERSION "1.12"


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
  valNONE, valSTR, valINT, valVAR
};

struct DisplayAction
{
  const char *key;
  const char *name;
  DisplayValueType valueType;
  void (*function)(DisplayCommandState& state);
};

extern DisplayAction *FindDisplayAction(p_command command);

extern void DisplayString(DisplayCommandState& state);
extern void DisplayVariable(DisplayCommandState& state);
extern void DisplayCurrentTime(DisplayCommandState& state);
extern void DisplayFilename(DisplayCommandState& state);
extern void DisplayClose(DisplayCommandState& state);
extern void DisplayClear(DisplayCommandState& state);
