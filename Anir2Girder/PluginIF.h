/*********************************************************************
 *  @file   : PluginIF.h
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : Girder API
 *
 *********************************************************************
 * $Id$
 ********************************************************************/

/* 
   
   Girder 3.2+ API Header
   May 4th 2002
   Copyright Ron Bessems


   When contributing plugins to Girder please consider releasing the sourcecode
   to the public or to Ron. In the past we have had some binary only plugins 
   that needed modification but we where unable to contact the author thus loosing
   valuable work. When releasing the code to Ron it is possible to do this under
   a NDA.

   Girder developers homepage

   http://www.girder.nl/developer.php
   
   Questions go to 
   
   http://www.girder.nl/phpBB2/

   or directly to Ron at,

   developer@girder.nl

*/


#ifndef __PLUGINIF_H_
#define __PLUGINIF_H_


// return values of the gir_event function
#define retContinue       0   // Girder will continue working normally
#define retExplicitReturn 1   // Girder will stop after this command in the current Multigroup but continues on elsewhere.
#define retStopProcessing 2   // Girder will stop processing this event completely.

// The OSD settings for use with get_osd_settings
#define osdFGColor             1
#define osdFGDColor            2
#define osdBGColor             3
#define osdWidth               4
#define osdHeight              5
#define osdCaption             6
#define osdBorder              7
#define osdTransparent         8
#define osdFontsize            9
#define osdFontWeight          10
#define osdFontItalic          11
#define osdFontUnderline       12
#define osdFontStrikeout       13
#define osdFontWeight          10
#define osdFontItalic          11
#define osdFontUnderline       12
#define osdFontStrikeout       13
#define osdLeft                14
#define osdTop                 15
#define osdCenter              16
#define osdMonitor             17

// generic return values
#define GIR_TRUE               1 
#define GIR_FALSE              0
#define GIR_ASYNC_LEARN        2

// gir_info messages
#define GIRINFO_QUERYUNLOAD    1
/* 
   If you return GIR_FALSE Girder will query the plugin again 500 milliseconds later. If you
   return GIR_TRUE girder will not ask again and unload the plugin as soon as all other plugins
   agreed to unload.

   The wparam in QUERY_UNLOAD contains the number of requests left before Girder closes
   the plugin anyway no matter what you return.

*/
#define GIRINFO_POWERBROADCAST 2
/* this passes the wparam and lparam directly from the WM_POWERBROADCAST, 
   to confirm the message pass GIR_TRUE.
*/

/* Currently not yet supported */
#define GIRINFO_WINLOGON       3

/* Wparam for GIRINFO_WINLOGON can be : */
#define WL_LOGON               0
#define WL_LOGOFF              1
#define WL_STARTUP             2
#define WL_SHUTDOWN            3
#define WL_STARTSCREENSAVER    4
#define WL_STOPSCREENSAVER     5
#define WL_STARTSHELL          6
#define WL_LOCK                7
#define WL_UNLOCK              8

/* This message is sent when the girder GUI window closes,
   you could close the plugin windows if needed */
#define GIRINFO_MAINWIN_CLOSE  4
/* This message is sent when the girder GUI window opens */
#define GIRINFO_MAINWIN_OPEN   5


//! The command structure that girder passes to the plugin.
typedef struct s_command 
{
   CRITICAL_SECTION  critical_section;
   PCHAR             name;
   int               actiontype;
   int               actionsubtype;
   PCHAR             svalue1;
   PCHAR             svalue2;
   PCHAR             svalue3;
   int               bvalue1;
   int               bvalue2;
   int               bvalue3;
   int               ivalue1;
   int               ivalue2;
   int               ivalue3;
   int               lvalue1;
   int               lvalue2;
   int               lvalue3;
   void              *binary;
   int               size;
} t_command, *p_command;


// Girder exports a few functions here are the required typedefs
typedef void   (WINAPI *t_target_callback)   (HWND hw, p_command command);
typedef void   (WINAPI *t_set_command)       (p_command command); 
typedef void   (WINAPI *t_target_enum)       (int id, t_target_callback callback); 
typedef void   (WINAPI *t_realloc_pchar)     (PCHAR * old, PCHAR newchar);
typedef void   (WINAPI *t_show_osd)          (int timer);
typedef void   (WINAPI *t_hide_osd)          ();
typedef int    (WINAPI *t_start_osd_draw)    (HWND *hw, HDC *h, int user);
typedef void   (WINAPI *t_stop_osd_draw)     (HDC h);
typedef int    (WINAPI *t_treepicker_show)   (HWND window, int id);
typedef int    (WINAPI *t_event_cb)          (PCHAR event_string, int device, void *payload, int len);
typedef int    (WINAPI *t_register_cb)       (int actionplugin, t_event_cb callback, PCHAR prefix, int device);
typedef int    (WINAPI *t_get_link_name)     (int lvalue, PCHAR szstore, int size);
typedef int    (WINAPI *t_parse_girder_reg)  (PCHAR orig, PCHAR szstore, int size);
typedef int    (WINAPI *t_i18n_translate)    (PCHAR orig, PCHAR szstore, int size);
typedef DWORD  (WINAPI *t_get_osd_settings)  (int setting);
typedef int    (WINAPI *t_get_osd_fontname)  (PCHAR szstore, int size);
typedef int    (WINAPI *t_run_parser)        (PCHAR str, int *error_value);
typedef int    (WINAPI *t_get_int_var)       (PCHAR name);
typedef double (WINAPI *t_get_double_var)    (PCHAR name);
typedef int    (WINAPI *t_get_string_var)    (PCHAR name, PCHAR szstore, int size);
typedef int    (WINAPI *t_set_int_var)       (PCHAR name, int value);
typedef int    (WINAPI *t_set_double_var)    (PCHAR name, double value);
typedef int    (WINAPI *t_set_string_var)    (PCHAR name, PCHAR value);
typedef int    (WINAPI *t_delete_var)        (PCHAR name);
typedef void * (WINAPI *t_gir_malloc)        (int size);
typedef void   (WINAPI *t_gir_free)          (void *data);
typedef int    (WINAPI *t_send_event)        (const char * eventstring, void *payload, int len, int device);
typedef int    (WINAPI *t_trigger_command)   (int command_id);


//! this is the version 1 export function structure as passed to gir_open
typedef struct s_functions
{
   DWORD                size;
   t_parse_girder_reg   parse_reg_string;
   t_get_link_name      get_link_name;
   t_set_command        set_command;
   t_target_enum        target_enum;		
   t_realloc_pchar      realloc_pchar;
   t_show_osd           show_osd;
   t_hide_osd           hide_osd;
   t_start_osd_draw     start_osd_draw;
   t_stop_osd_draw      stop_osd_draw;
   t_treepicker_show    treepicker_show;
   t_register_cb        register_cb;
   t_i18n_translate     i18n_translate;
   t_get_osd_settings   get_osd_settings;
   t_get_osd_fontname   get_osd_font_name;
   t_gir_malloc         gir_malloc;
   t_gir_free           gir_free;         
   t_get_int_var        get_int_var;
   t_get_double_var     get_double_var;
   t_get_string_var     get_string_var;
   t_set_int_var        set_int_var;
   t_set_double_var     set_double_var;
   t_set_string_var     set_string_var;
   t_delete_var         delete_var;
   t_run_parser         run_parser;
   t_send_event         send_event;
   t_trigger_command    trigger_command;
   HWND                 parent_hwnd;
} t_functions, *p_functions;



#endif __PLUGINIF_H_