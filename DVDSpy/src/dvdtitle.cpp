/* Compute DVD title */

#include "stdafx.h"
#include "plugin.h"

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
  if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, 
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

// Get DVD attributes.  Get video attributes from VTSTT_VOBS of first
// title, which is most likely to represent the bulk of the content.
BOOL GetDVDAttribs(LPCSTR disc, 
                   LPCSTR& aspect, LPCSTR& display, LPCSTR& standard, 
                   LPSTR regions)
{
  char fname[MAX_PATH];
  FILE *file;

  strcpy(fname, disc);
  strcat(fname, "video_ts\\vts_01_0.ifo");

  file = fopen(fname, "rb");
  if (NULL == file)
    return FALSE;

  if (fseek(file, VTSTT_VOBS_ATTRIB_OFFSET, SEEK_SET)) {
    fclose(file);
    return FALSE;
  }

  char v_attribs;

  if (fread(&v_attribs, 1, sizeof(v_attribs), file) != sizeof(v_attribs)) {
    fclose(file);
    return FALSE;
  }

  fclose(file);

  switch (v_attribs & VIDEO_ATTRIB_ASPECT_MASK) {
  case VIDEO_ATTRIB_ASPECT_4X3:
    aspect = "4:3";
    break;
  case VIDEO_ATTRIB_ASPECT_16X9:
    aspect = "16:9";
    break;
  default:
    aspect = "unknown";
    break;
  }

  if (v_attribs & VIDEO_ATTRIB_DISPLAY_PANSCAN) {
    if (v_attribs & VIDEO_ATTRIB_DISPLAY_LETTERBOX) {
      display = "none";
    }
    else {
      display = "letterbox";
    }
  }
  else {
    if (v_attribs & VIDEO_ATTRIB_DISPLAY_LETTERBOX) {
      display = "pan-scan";
    }
    else {
      display = "both";
    }
  }

  if (v_attribs & VIDEO_ATTRIB_STANDARD_PAL) {
    standard = "PAL";
  }
  else {
    standard = "NTSC";
  }

  strcpy(fname, disc);
  strcat(fname, "video_ts\\video_ts.ifo");

  file = fopen(fname, "rb");
  if (NULL == file)
    return FALSE;

  if (fseek(file, VMG_CATEGORY_REGION_MASK_OFFSET, SEEK_SET)) {
    fclose(file);
    return FALSE;
  }

  char r_mask;

  if (fread(&r_mask, 1, sizeof(r_mask), file) != sizeof(r_mask)) {
    fclose(file);
    return FALSE;
  }

  fclose(file);
  
  for (int i = 1; i <= 8; i++) {
    if ((r_mask & (1 << (i-1))) == 0)
      *regions++ = '0' + i;
  }
  *regions = '\0';

  return TRUE;
}

// Get 64-bit disc id.  This is computed from the contents (file dates
// and lengths, perhaps).  COM must have been initialized before
// calling this.  Hopefully that won't be a problem on older operating
// systems.
void GetDVDDiscID(LPCSTR disc, ULONGLONG *discid)
{
  HRESULT hr;

  IDvdInfo2 *dvdInfo = NULL;
  hr = CoCreateInstance(CLSID_DVDNavigator, NULL, CLSCTX_ALL, 
                        IID_IDvdInfo2, (void**)&dvdInfo);
  if (FAILED(hr)) return;

  OLECHAR olevol[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, disc, -1, 
                      olevol, sizeof(olevol) / sizeof(OLECHAR));
  wcscat(olevol, OLESTR("\\video_ts"));

  hr = dvdInfo->GetDiscID(olevol, discid);

  dvdInfo->Release();
}
