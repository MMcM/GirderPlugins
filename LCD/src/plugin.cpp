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

#define WIN32_LEAN_AND_MEAN
#define GIRDER_CPP

#include <windows.h>
#include "plugin.h"
#include "ui.h"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>

#include "lcdriver.h"


int nDisplayCols = 0, nDisplayRows = 0;
LPSTR pDisplayBuf = NULL;
BOOL bDisplayOpen = FALSE;

LPSTR pMarquee = NULL;
int nMarqueeRow = 0, nMarqueePos = 0, nMarqueeLen = 0;
BOOL bMarqueeSimulated = FALSE;
int nMarqueePixelWidth = 0, nMarqueeSpeed = 0;
UINT idMarqueeTimer = 0;

class DisplayCommandState
{
public:
  p_command m_command;
  PCHAR m_eventstring;
  void *m_payload;
  int m_len;
  PCHAR m_status;
  int m_statuslen;

  DisplayCommandState(p_command command, PCHAR eventstring, 
                      void *payload, int len, 
                      PCHAR status, int statuslen)
    : m_command(command), m_eventstring(eventstring), 
      m_payload(payload), m_len(len),
      m_status(status), m_statuslen(statuslen) 
  {
    EnterCriticalSection(&m_command->critical_section);    
  }

  ~DisplayCommandState() 
  {
    LeaveCriticalSection(&m_command->critical_section);
  }
  
  void SetStatus(LPCSTR status)
  {
    strncpy(m_status, status, m_statuslen);
  }

  PCHAR GetPayloadString(int index);
};

PCHAR DisplayCommandState::GetPayloadString(int index)
{
  PCHAR pld = (PCHAR)m_payload;
  int len = m_len;
  if (len-- <= 0) 
    return NULL;
  int nstr = *pld++;
  if ((index < 1) || (index > nstr))
    return NULL;
  while (--index > 0) {
    while (TRUE) {
      if (len-- <= 0)
        return NULL;
      if ('\0' == *pld++)
        break;
    }
  }
  return pld;
}

BOOL DisplayOpen(DisplayCommandState& state)
{
  if (!bDisplayOpen) {
    if (LCD_RESULT_NOERROR != lcdRequest()) {
      state.SetStatus("Could not open LCD.");
      return FALSE; 
    }

    nDisplayCols = lcdGetWidth();
    nDisplayRows = lcdGetHeight();
    pDisplayBuf = (LPSTR)malloc(nDisplayCols * nDisplayRows);
    memset(pDisplayBuf, ' ', nDisplayCols * nDisplayRows);

    lcdSetCursor(0);              // Hide cursor
    lcdSetWrapMode(0, 0);         // Turn off wrap mode
    lcdClearDisplay();            // Clear

    bDisplayOpen = TRUE;
  }
  return bDisplayOpen;
}

void DisplayClose()
{
  if (bDisplayOpen) {
    if (NULL != pMarquee) {
      if (bMarqueeSimulated) {
        KillTimer(NULL, idMarqueeTimer);
        idMarqueeTimer = 0;
      }
      else
        lcdDisableMarquee();
      free(pMarquee);
      pMarquee = NULL;
    }

    free(pDisplayBuf);
    pDisplayBuf = NULL;

    lcdFree();

    bDisplayOpen = FALSE;
  }
}

void DisplayClose(DisplayCommandState& state)
{
  DisplayClose();
  state.SetStatus("Display closed.");
}

void DisplayClear(DisplayCommandState& state)
{
  if (!bDisplayOpen && !DisplayOpen(state)) 
    return;
  if (NULL != pMarquee) {
    if (bMarqueeSimulated) {
      KillTimer(NULL, idMarqueeTimer);
      idMarqueeTimer = 0;
    }
    else
      lcdDisableMarquee();
    free(pMarquee);
    pMarquee = NULL;
  }
  lcdClearDisplay();
  memset(pDisplayBuf, ' ', nDisplayCols * nDisplayRows);
  state.SetStatus("Display cleared.");
}

void DisplayInternal(int row, int col, LPCSTR str, int length)
{
  lcdSetCursorPos(col, row);
  lcdSendString(str, length);

#ifdef _DEBUG
  {
    char dbuf[1024];
    sprintf(dbuf, "LCD: @%d,%d: '", row, col);
    char *ep = dbuf + strlen(dbuf);
    size_t nb = length;
    if (nb > sizeof(dbuf) - (ep - dbuf) - 3)
      nb = sizeof(dbuf) - (ep - dbuf) - 3;
    memcpy(ep, str, nb);
    strcpy(ep + nb, "'\n");
    OutputDebugString(dbuf);
  }
#endif
}

void MarqueeTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
  LPSTR bp = pDisplayBuf + (nMarqueeRow * nDisplayCols);
  memmove(bp, bp+1, nDisplayCols-1);
  bp[nDisplayCols-1] = pMarquee[nMarqueePos++];
  DisplayInternal(nMarqueeRow, 0, bp, nDisplayCols);
  if (nMarqueePos >= nMarqueeLen)
    nMarqueePos = 0;
}

void DisplayCommon(DisplayCommandState& state, LPCSTR str)
{
  if (!DisplayOpen(state)) 
    return;
  state.SetStatus(str);

  int row, col;
  row = state.m_command->ivalue1 % nDisplayRows;

  BOOL bDrawingMarquee = FALSE;
  if (state.m_command->ivalue2 < 0) {
    int length = strlen(str);
    if (length > nDisplayCols) {
      // Marquee mode and text wide enough to need it.
      if (NULL != pMarquee) {
        if (!strcmp(str, pMarquee) && (row == nMarqueeRow))
          return;               // Same as presently active.
        if (bMarqueeSimulated) {
          KillTimer(NULL, idMarqueeTimer);
          idMarqueeTimer = 0;
        }
        else
          lcdDisableMarquee();
        if (row != nMarqueeRow) {
          // Only one row allowed to do a marquee at a time; display
          // what fits of the old row in ordinary mode.
          memcpy(pDisplayBuf + (nMarqueeRow * nDisplayCols), pMarquee, nDisplayCols);
          DisplayInternal(nMarqueeRow, 0, pMarquee, nDisplayCols);
        }
        free(pMarquee);
        pMarquee = NULL;
      }
      if (nMarqueePixelWidth == 0) {
        // For CrystalFontz, LCDriver mistakenly sends the speed value
        // straight to the \022 command, as though the device command
        // units were msec.  Actually, they are 1/96sec.  So, 16 is
        // the right value for one char/sec.
        nMarqueePixelWidth = 1; // One pixel.
        nMarqueeSpeed = 16;     // One character / second.

        HKEY hkey;
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
                                        "Software\\Girder3\\SoftPlugins\\LCD", 
                                        &hkey)) {
          BYTE buf[32];
          DWORD dwType, dwLen;
          dwLen = sizeof(buf);
          if (ERROR_SUCCESS == RegQueryValueEx(hkey, "MarqueeSimulated", NULL,
                                               &dwType, buf, &dwLen)) {
            switch (dwType) {
            case REG_DWORD:
              bMarqueeSimulated = !!*(DWORD*)&buf;
              break;
            case REG_SZ:
              bMarqueeSimulated = !_stricmp((LPCSTR)buf, "True");
              break;
            }
          }
          if (ERROR_SUCCESS == RegQueryValueEx(hkey, "MarqueePixelWidth", NULL,
                                               &dwType, buf, &dwLen)) {
            switch (dwType) {
            case REG_DWORD:
              nMarqueePixelWidth = (int)*(DWORD*)&buf;
              break;
            case REG_SZ:
              nMarqueePixelWidth = atoi((LPCSTR)buf);
              break;
            }
          }
          if (ERROR_SUCCESS == RegQueryValueEx(hkey, "MarqueeSpeed", NULL,
                                               &dwType, buf, &dwLen)) {
            switch (dwType) {
            case REG_DWORD:
              nMarqueeSpeed = (int)*(DWORD*)&buf;
              break;
            case REG_SZ:
              nMarqueeSpeed = atoi((LPCSTR)buf);
              break;
            }
          }
        }
      }
      if (bMarqueeSimulated || 
          (LCD_RESULT_NOERROR == lcdEnableMarquee(str, length, row, 
                                                  nMarqueePixelWidth, nMarqueeSpeed))) {
        pMarquee = (LPSTR)malloc(length + 1);
        memcpy(pMarquee, str, length + 1);
        memset(pDisplayBuf + (row * nDisplayCols), 0xFE, nDisplayCols);
#ifdef _DEBUG
        {
          char dbuf[1024];
          sprintf(dbuf, "LCD: @%d<<<: '", row);
          char *ep = dbuf + strlen(dbuf);
          size_t nb = length;
          if (nb > sizeof(dbuf) - (ep - dbuf) - 3)
            nb = sizeof(dbuf) - (ep - dbuf) - 3;
          memcpy(ep, str, nb);
          strcpy(ep + nb, "'\n");
          OutputDebugString(dbuf);
        }
#endif
        nMarqueeRow = row;
        if (!bMarqueeSimulated)
          return;
        nMarqueePos = nDisplayCols;
        nMarqueeLen = length;
        idMarqueeTimer = SetTimer(NULL, 0, 
                                  // Keep time units consistent for now.
                                  (nMarqueeSpeed * 125) / (nMarqueePixelWidth * 2), 
                                  MarqueeTimer);
        bDrawingMarquee = (0 != idMarqueeTimer);
      }
    }
    // Fits as normal rest of line (or simulated or error enabling marquee).
    col = 0;
  }
  else
    col = state.m_command->ivalue2 % nDisplayCols;

  if (!bDrawingMarquee && (NULL != pMarquee) && (row == nMarqueeRow)) {
    lcdDisableMarquee();
    // Display what fits of scrolling in ordinary mode.
    memcpy(pDisplayBuf + (row * nDisplayCols), pMarquee, nDisplayCols);
    DisplayInternal(row, 0, pMarquee, nDisplayCols);
    free(pMarquee);
    pMarquee = NULL;
  }

  int width = nDisplayCols - col;
  if ((state.m_command->ivalue3 > 0) && (state.m_command->ivalue3 < width))
    width = state.m_command->ivalue3;
  LPSTR start = NULL, end;
  LPSTR bp = pDisplayBuf + (row * nDisplayCols) + col;
  while (width-- > 0) {
    char ch = *str;
    if ('\0' == ch)
      ch = ' ';
    else
      str++;
    if (ch != *bp) {
      if (NULL == start)
        start = bp;
      end = bp;
      *bp = ch;
    }
    else if (NULL == start)
      col++;
    bp++;
  }
  if (NULL == start) 
    return;    // No change

  end++;
  DisplayInternal(row, col, start, (end - start));
}

void DisplayString(DisplayCommandState& state)
{
  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  DisplayCommon(state, buf);
}

void DisplayVariable(DisplayCommandState& state)
{
  char buf[1024];
  SF.get_string_var(state.m_command->svalue1, buf, sizeof(buf));
  DisplayCommon(state, buf);
}

void DisplayFilename(DisplayCommandState& state)
{
  char buf[1024];
  SF.get_string_var(state.m_command->svalue1, buf, sizeof(buf));
  PCHAR sp = strrchr(buf, '\\');
  if (NULL != sp)
    sp++;
  else if (NULL != strstr(buf, "://")) // A URL
    sp = strrchr(buf, '/') + 1;
  else
    sp = buf;
  PCHAR ep = strrchr(sp, '.');
  if (NULL != ep)
    *ep = '\0';
  DisplayCommon(state, sp);
}

void DisplayCurrentTime(DisplayCommandState& state)
{
  const char *fmt = state.m_command->svalue1;
  if ((NULL == fmt) || ('\0' == *fmt))
    fmt = "%H:%M:%S";

  time_t ltime;
  struct tm *now;
  char buf[128];
  
  time(&ltime);
  now = localtime(&ltime);
  strftime(buf, sizeof(buf), fmt, now);
  DisplayCommon(state, buf);
}

extern "C" int WINAPI gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions p)
{

  p_functions h = (p_functions )	p;
  if ( h->size != sizeof( s_functions ) )
    {
      return GIR_FALSE;
    }
		
  memcpy((void *)&SF, p, sizeof ( s_functions ));



  return GIR_TRUE;

}



extern "C" int WINAPI gir_close()
{
  DisplayClose();

  Close_Config();

  return GIR_TRUE;

}

/* This function will be called when an action should be executed!
  
  In: TCommand *command: The details of the command to be executed
  Out: PCHAR ReturnBuf: an reallocated buffer with 1000 bytes of space

  */
extern "C" int WINAPI gir_event(p_command command, PCHAR eventstring, void *payload, int len, PCHAR status, int statuslen)
{
  DisplayCommandState state(command, eventstring,
                            payload, len,
                            status, statuslen);
  DisplayAction *action = FindDisplayAction(command);
  if (NULL != action)
    (*action->function)(state);
  return retContinue;
}

/* When the user selects a different command in the treeview the
   plugin will be notified of this event via this function

   In : TCommand *command: the details of the new plugin
*/

extern "C" void WINAPI gir_command_changed(p_command  command)
{

  if ( command == NULL ) 
    {
      Enable_Config(FALSE);
    }
  else
    {
      Enable_Config(TRUE);
      CurCommand = command;	
      Update_Config();
    }

}


/* You should open a configuration dialog for the plugin
   when this function gets called.
  
  In : TCommand *command: the details of the new plugin
*/
extern "C" void WINAPI gir_command_gui()
{

  Show_Config();

}




extern "C" int WINAPI gir_requested_api(int maxapi)
{
  return 1;
}

extern "C" int WINAPI gir_devicenum()
{
  return PLUGINNUM;
}

extern "C" void WINAPI gir_description(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, "LCD display.  Written by Mike McMahon (MMcM).", Length);
}

extern "C" void WINAPI gir_name(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINNAME, Length);
}

extern "C" void WINAPI gir_version(PCHAR Buffer, BYTE Length)
{
  strncpy(Buffer, PLUGINVERSION, Length);
}



/* Called by windows */
BOOL WINAPI DllMain( HANDLE hModule, DWORD fdwreason,  LPVOID lpReserved )
{
  UNREFERENCED_PARAMETER(lpReserved);
  
  switch(fdwreason) {
  case DLL_PROCESS_ATTACH:
    hInstance=HINSTANCE(hModule);
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
