/* LCD device control */

#include "stdafx.h"
#include "plugin.h"

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
  PCHAR m_status;
  int m_statuslen;

  DisplayCommandState(p_command command,
                      PCHAR status, int statuslen)
    : m_command(command),
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
};

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

void DisplayUpdate(int row, int col, int width, LPCSTR str)
{
  if ((row < 0) || (row >= nDisplayRows))
    return;

  BOOL bDrawingMarquee = FALSE;
  if (col < 0) {
    int length;
    LPCSTR endl = strchr(str, '\n');
    if (NULL == endl)
      length = strlen(str);
    else {
      length = endl - str;
      while ((length > 0) &&
             ('\r' == str[length-1]))
        length--;
    }
    if (length > nDisplayCols) {
      // Marquee mode and text wide enough to need it.
      if (NULL != pMarquee) {
        if ((row == nMarqueeRow) &&
            (length == nMarqueeLen) && 
            !memcmp(str, pMarquee, length))
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
        memcpy(pMarquee, str, length);
        pMarquee[length] = '\0';
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
  else if (col >= nDisplayCols)
    return;

  if (!bDrawingMarquee && (NULL != pMarquee) && (row == nMarqueeRow)) {
    if (bMarqueeSimulated) {
      KillTimer(NULL, idMarqueeTimer);
      idMarqueeTimer = 0;
    }
    else
      lcdDisableMarquee();
    // Display what fits of scrolling in ordinary mode.
    memcpy(pDisplayBuf + (row * nDisplayCols), pMarquee, nDisplayCols);
    DisplayInternal(row, 0, pMarquee, nDisplayCols);
    free(pMarquee);
    pMarquee = NULL;
  }

  int ncols = nDisplayCols - col;
  if ((width <= 0) || (width > ncols))
    width = ncols;
  LPSTR start = NULL, end;
  LPSTR bp = pDisplayBuf + (row * nDisplayCols) + col;
  while (width-- > 0) {
    char ch = *str;
    if (('\0' == ch) || ('\n' == ch) || ('\r' == ch))
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

void DisplayCommon(DisplayCommandState& state, LPCSTR str)
{
  if (!DisplayOpen(state)) 
    return;
  DisplayUpdate(state.m_command->ivalue1, state.m_command->ivalue2, 
                state.m_command->ivalue3, str);
  state.SetStatus(str);
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

void DisplayScreen(DisplayCommandState& state)
{
  if (!DisplayOpen(state)) 
    return;

  char buf[1024];
  SF.parse_reg_string(state.m_command->svalue1, buf, sizeof(buf));
  for (int pass = 0; pass <= 1; pass++) { // Do marquee after others.
    PCHAR pval = buf;
    for (int i = 0; i < nDisplayRows; i++) {
      if (!(state.m_command->ivalue1 & (1 << i))) { // Enabled
        if (pass == !!(state.m_command->ivalue2 & (1 << i))) { // Marquee
          DisplayUpdate(i, (pass) ? -1 : 0, -1, pval);
        }
      }
      PCHAR next = strchr(pval, '\n');
      if (NULL != next)
        pval = next + 1;
      else
        pval += strlen(pval);
    }
  }
  state.SetStatus("LCD screen");
}

void DisplayCommand(p_command command,
                    PCHAR status, int statuslen)
{
  DisplayCommandState state(command,
                            status, statuslen);
  DisplayAction *action = FindDisplayAction(command);
  if (NULL != action)
    (*action->function)(state);
}
