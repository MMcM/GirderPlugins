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

#define PLUGINNUM 123
#define PLUGINNAME "SetWaveDev"
#define PLUGINVERSION "1.2"

#include <girder.h>

#ifdef GIRDER_CPP
	p_command				  CurCommand;
   s_functions            SF;
	HINSTANCE			     hInstance;
#endif

#ifndef GIRDER_CPP
	extern p_command		  CurCommand;		
   extern s_functions     SF;
	extern HINSTANCE		  hInstance;
#endif


/* Commmon routines */

extern PCHAR DecodeKeyValue(PCHAR value, PHKEY phkey);
