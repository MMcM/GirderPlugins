
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <malloc.h>

#include "girder.h"

class CSVParser
{
public:
  CSVParser() {
    m_file = NULL;
    m_buf = NULL;
  }
  ~CSVParser() {
    if (NULL != m_file)
      fclose(m_file);
    if (NULL != m_buf)
      free(m_buf);
  }

  BOOL Open(LPCSTR file);
  BOOL NextLine();
  LPCSTR NextField();

protected:
  char *m_buf;
  size_t m_buflen;
  FILE *m_file;
  LPSTR m_fp;
};

BOOL CSVParser::Open(LPCSTR file)
{
  m_buflen = 1024;
  m_buf = (LPSTR)malloc(m_buflen);
  if (NULL == m_buf)
    return FALSE;

  m_file = fopen(file, "r");
  return (NULL != m_file);
}

BOOL CSVParser::NextLine()
{
  while (TRUE) {
    // Read next file line.
    size_t llen = 0;
    while (TRUE) {
      if (NULL == fgets(m_buf+llen, m_buflen-llen, m_file))
        break;
      llen += strlen(m_buf+llen);
      if (0 == llen) break;
      else if ('\n' == m_buf[llen-1]) {
        m_buf[--llen] = '\0';
        break;
      }
      else if (llen + 128 > m_buflen) {
        m_buflen = m_buflen + 128;
        m_buf = (LPSTR)realloc(m_buf, m_buflen);
        if (NULL == m_buf) return FALSE;
      }
    }
    if (llen > 0) {
      m_fp = m_buf;
      return TRUE;
    }
    else if (feof(m_file))
      return FALSE;
    // else ignore blank line
  }
}

LPCSTR CSVParser::NextField()
{
  if ('\0' == *m_fp)
    return NULL;

  if ('"' == *m_fp) {
    // TODO: Something about however double quotes are escaped.
    LPSTR result = ++m_fp;
    m_fp = strchr(m_fp, '"');
    if (NULL == m_fp)
      return NULL;
    *m_fp++ = '\0';
    if ('\0' == *m_fp)
      return result;
    else if (',' != *m_fp)
      return NULL;
    *m_fp++ = '\0';
    return result;
  }
  else {
    LPSTR result = m_fp;
    m_fp = strchr(m_fp, ',');
    if (NULL != m_fp)
      *m_fp++ = '\0';
    else
      m_fp = result + strlen(result);
    return result;
  }
}

BOOL LookupDVDTitle(DWORD volser, LPSTR title, size_t tsize)
{
  HKEY hkey;
  if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, 
                                  "Software\\Girder3\\HardPlugins\\DVDSpy", 
                                  &hkey))
    return FALSE;
  
  char file[MAX_PATH];
  DWORD dtype, len;
  len = sizeof(file);
  LONG ok = RegQueryValueEx(hkey, "DVDTitles", NULL, &dtype, (LPBYTE)file, &len);
  RegCloseKey(hkey);
  if ((ERROR_SUCCESS != ok) || (REG_SZ != dtype))
    return FALSE;

  CSVParser parser;
  if (!parser.Open(file))
    return FALSE;
  if (!parser.NextLine())
    return FALSE;
  size_t field, fSerNo, fTitle;
  field = fSerNo = fTitle = 0;
  LPCSTR fstr;
  while (NULL != (fstr = parser.NextField())) {
    field++;
    if (!_stricmp(fstr, "Title"))
      fTitle = field;
    else if (!_stricmp(fstr, "Serial Number"))
      fSerNo = field;
  }
  if (!fSerNo || !fTitle)
    return FALSE;
  
  char key[9];
  sprintf(key, "%X", volser);
  while (parser.NextLine()) {
    field = 0;
    LPCSTR recTitle = NULL;
    BOOL recMatch = FALSE;
    while (NULL != (fstr = parser.NextField())) {
      field++;
      if (field == fSerNo) {
        if (!_stricmp(fstr, key)) {
          if (NULL != recTitle) {
            strncpy(title, recTitle, tsize);
            return TRUE;
          }
          else
            recMatch = TRUE;
        }
        else if (fSerNo < fTitle)
          break;
      }
      else if (field == fTitle) {
        if (recMatch) {
          strncpy(title, fstr, tsize);
          return TRUE;
        }
        recTitle = fstr;
      }
    }
  }
  return FALSE;
}

// Try various strategies to find the title of this disc.
void GetDVDTitle(LPCSTR disc, LPSTR title, size_t tsize, LPDWORD volser)
{
  GetVolumeInformation(disc, title, tsize, volser, NULL, NULL, NULL, 0);

  if (LookupDVDTitle(*volser, title, tsize))
    return;

  {
    char dfile[MAX_PATH], dtitle[256];
    strcpy(dfile, disc);
    strcat(dfile, "disc.id");
    if (GetPrivateProfileString("PCFriendly", "name", "", dtitle, sizeof(dtitle), dfile))
      strncpy(title, dtitle, tsize);
  }

}
