/* User interface. 
$Header$
*/

#include "stdafx.h"
#include "plugin.h"
#include "resource.h"

struct FieldEntry {
  const char *name;
  ID3_FrameID frame;
  ID3_FieldID field;
  ID3_FieldID keyfld;
} FieldEntries[] = {
//{ /* AENC */ "Audio encryption", ID3FID_AUDIOCRYPTO, },
//{ /* APIC */ "Attached picture", ID3FID_PICTURE, },
  { /* COMM */ "Comments", ID3FID_COMMENT, ID3FN_TEXT, ID3FN_DESCRIPTION },
  { /* COMR */ "Commercial", ID3FID_COMMERCIAL, },
//{ /* ENCR */ "Encryption method registration", ID3FID_CRYPTOREG, },
//{ /* EQUA */ "Equalization", ID3FID_EQUALIZATION, },
//{ /* ETCO */ "Event timing codes", ID3FID_EVENTTIMING, },
//{ /* GEOB */ "General encapsulated object", ID3FID_GENERALOBJECT, },
//{ /* GRID */ "Group identification registration", ID3FID_GROUPINGREG, },
  { /* IPLS */ "Involved people", ID3FID_INVOLVEDPEOPLE, ID3FN_TEXT },
//{ /* LINK */ "Linked information", ID3FID_LINKEDINFO, },
//{ /* MCDI */ "Music CD identifier", ID3FID_CDID, },
//{ /* MLLT */ "MPEG location lookup table", ID3FID_MPEGLOOKUP, },
//{ /* OWNE */ "Ownership frame", ID3FID_OWNERSHIP, },
//{ /* PRIV */ "Private frame", ID3FID_PRIVATE, },
//{ /* PCNT */ "Play counter", ID3FID_PLAYCOUNTER, },
//{ /* POPM */ "Popularimeter", ID3FID_POPULARIMETER, },
//{ /* POSS */ "Position synchronisation frame", ID3FID_POSITIONSYNC, },
//{ /* RBUF */ "Recommended buffer size", ID3FID_BUFFERSIZE, },
//{ /* RVAD */ "Relative volume adjustment", ID3FID_VOLUMEADJ, },
//{ /* RVRB */ "Reverb", ID3FID_REVERB, },
//{ /* SYLT */ "Synchronized lyric/text", ID3FID_SYNCEDLYRICS, },
//{ /* SYTC */ "Synchronized tempo codes", ID3FID_SYNCEDTEMPO, },
  { /* TALB */ "Album title", ID3FID_ALBUM, ID3FN_TEXT },
  { /* TBPM */ "BPM", ID3FID_BPM, ID3FN_TEXT },
  { /* TCOM */ "Composer", ID3FID_COMPOSER, ID3FN_TEXT },
  { /* TCON */ "Content type", ID3FID_CONTENTTYPE, ID3FN_TEXT },
  { /* TCOP */ "Copyright", ID3FID_COPYRIGHT, ID3FN_TEXT },
  { /* TDAT */ "Date", ID3FID_DATE, ID3FN_TEXT },
  { /* TDLY */ "Playlist delay", ID3FID_PLAYLISTDELAY, ID3FN_TEXT },
  { /* TENC */ "Encoded by", ID3FID_ENCODEDBY, ID3FN_TEXT },
  { /* TEXT */ "Lyricist", ID3FID_LYRICIST, ID3FN_TEXT },
  { /* TFLT */ "File type", ID3FID_FILETYPE, ID3FN_TEXT },
  { /* TIME */ "Time", ID3FID_TIME, ID3FN_TEXT },
  { /* TIT1 */ "Group description", ID3FID_CONTENTGROUP, ID3FN_TEXT },
  { /* TIT2 */ "Title description", ID3FID_TITLE, ID3FN_TEXT },
  { /* TIT3 */ "Subtitle description", ID3FID_SUBTITLE, ID3FN_TEXT },
  { /* TKEY */ "Initial key", ID3FID_INITIALKEY, ID3FN_TEXT },
  { /* TLAN */ "Language(s)", ID3FID_LANGUAGE, ID3FN_TEXT },
  { /* TLEN */ "Length", ID3FID_SONGLEN, ID3FN_TEXT },
  { /* TMED */ "Media type", ID3FID_MEDIATYPE, ID3FN_TEXT },
  { /* TOAL */ "Original title", ID3FID_ORIGALBUM, ID3FN_TEXT },
  { /* TOFN */ "Original filename", ID3FID_ORIGFILENAME, ID3FN_TEXT },
  { /* TOLY */ "Original lyricist", ID3FID_ORIGLYRICIST, ID3FN_TEXT },
  { /* TOPE */ "Original artist", ID3FID_ORIGARTIST, ID3FN_TEXT },
  { /* TORY */ "Original release year", ID3FID_ORIGYEAR, ID3FN_TEXT },
  { /* TOWN */ "File owner", ID3FID_FILEOWNER, ID3FN_TEXT },
  { /* TPE1 */ "Lead performer", ID3FID_LEADARTIST, ID3FN_TEXT },
  { /* TPE2 */ "Band", ID3FID_BAND, ID3FN_TEXT },
  { /* TPE3 */ "Conductor", ID3FID_CONDUCTOR, ID3FN_TEXT },
  { /* TPE4 */ "Remixed by", ID3FID_MIXARTIST, ID3FN_TEXT },
  { /* TPOS */ "Part of a set", ID3FID_PARTINSET, ID3FN_TEXT },
  { /* TPUB */ "Publisher", ID3FID_PUBLISHER, ID3FN_TEXT },
  { /* TRCK */ "Track number", ID3FID_TRACKNUM, ID3FN_TEXT },
  { /* TRDA */ "Recording dates", ID3FID_RECORDINGDATES, ID3FN_TEXT },
  { /* TRSN */ "Internet radio name", ID3FID_NETRADIOSTATION, ID3FN_TEXT },
  { /* TRSO */ "Internet radio owner", ID3FID_NETRADIOOWNER, ID3FN_TEXT },
  { /* TSIZ */ "Size", ID3FID_SIZE, ID3FN_TEXT },
  { /* TSRC */ "ISRC", ID3FID_ISRC, ID3FN_TEXT },
  { /* TSSE */ "Settings used for encoding", ID3FID_ENCODERSETTINGS, ID3FN_TEXT },
  { /* TXXX */ "User defined text", ID3FID_USERTEXT, ID3FN_TEXT, ID3FN_DESCRIPTION },
  { /* TYER */ "Year", ID3FID_YEAR, ID3FN_TEXT },
//{ /* UFID */ "Unique file identifier", ID3FID_UNIQUEFILEID, },
  { /* USER */ "Terms of use", ID3FID_TERMSOFUSE, },
  { /* USLT */ "Unsynchronized lyrics", ID3FID_UNSYNCEDLYRICS, ID3FN_TEXT, ID3FN_DESCRIPTION },
  { /* WCOM */ "Commercial information", ID3FID_WWWCOMMERCIALINFO, ID3FN_URL },
  { /* WCOP */ "Copyright webpage", ID3FID_WWWCOPYRIGHT, ID3FN_URL },
  { /* WOAF */ "Audio file webpage", ID3FID_WWWAUDIOFILE, ID3FN_URL },
  { /* WOAR */ "Artist webpage", ID3FID_WWWARTIST, ID3FN_URL },
  { /* WOAS */ "Audio source webpage", ID3FID_WWWAUDIOSOURCE, ID3FN_URL },
  { /* WORS */ "Internet radio homepage", ID3FID_WWWRADIOPAGE, ID3FN_URL },
  { /* WPAY */ "Payment webpage", ID3FID_WWWPAYMENT, ID3FN_URL },
  { /* WPUB */ "Publisher webpage", ID3FID_WWWPUBLISHER, ID3FN_URL },
  { /* WXXX */ "User defined webpage", ID3FID_WWWUSER, ID3FN_URL, ID3FN_DESCRIPTION },
//{ /*      */ "Encrypted meta frame (id3v2.2.x)", ID3FID_METACRYPTO, },
//{ /*      */ "Compressed meta frame (id3v2.2.1)", ID3FID_METACOMPRESSION, },
};

#define countof(x) sizeof(x)/sizeof(x[0])

/* Local variables */
p_command g_pCommand = NULL;
HWND g_hCommandDialog = NULL;
HANDLE g_hCommandThread = NULL;

static void AdjustVisibility(HWND hwnd)
{
  BOOL bHasKey = FALSE;
  
  HWND hwndList = GetDlgItem(hwnd, IDC_FRAME);
  int idx = SendMessage(hwndList, CB_GETCURSEL, 0, 0);
  if (CB_ERR != idx) {
    FieldEntry *fe = (FieldEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    if (NULL != fe)
      bHasKey = (0 != fe->keyfld);
  }
  
  ShowWindow(GetDlgItem(hwnd, IDC_KEYL), (bHasKey) ? SW_SHOW : SW_HIDE);
  ShowWindow(GetDlgItem(hwnd, IDC_KEY), (bHasKey) ? SW_SHOW : SW_HIDE);
}

static void LoadUISettings(HWND hwnd)
{
  if (NULL == g_pCommand) {
    EnableWindow(GetDlgItem(hwnd, IDC_VAR), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_FILE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_FRAME), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_KEY), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
    return;
  }

  EnterCriticalSection(&g_pCommand->critical_section);

  LPCSTR var = g_pCommand->svalue1;
  if ('\0' == *var) var = "var";
  LPCSTR file = g_pCommand->svalue2;
  if ('\0' == *file) file = "[pld1]";
  SetWindowText(GetDlgItem(hwnd, IDC_VAR), var);
  SetWindowText(GetDlgItem(hwnd, IDC_FILE), file);
  SetWindowText(GetDlgItem(hwnd, IDC_KEY), g_pCommand->svalue3);

  HWND hwndList = GetDlgItem(hwnd, IDC_FRAME);
  int nf = SendMessage(hwndList, CB_GETCOUNT, 0, 0);
  int selidx = 0;
  for (int idx = 0; idx < nf; idx++) {
    FieldEntry *fe = (FieldEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    if ((fe->frame == g_pCommand->ivalue1) &&
        (fe->field == g_pCommand->ivalue2) &&
        (fe->keyfld == g_pCommand->ivalue3)) {
      selidx = idx;
      break;
    }
  }
  SendMessage(hwndList, CB_SETCURSEL, selidx, 0);
             
  EnableWindow(GetDlgItem(hwnd, IDC_VAR), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_FILE), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_FRAME), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_KEY), TRUE);
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  
  LeaveCriticalSection(&g_pCommand->critical_section);

  AdjustVisibility(hwnd);
}

static BOOL SaveUISettings(HWND hwnd)
{
  char buf[1024];
  
  EnterCriticalSection(&g_pCommand->critical_section);
  
  GetWindowText(GetDlgItem(hwnd, IDC_VAR), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue1, buf);

  GetWindowText(GetDlgItem(hwnd, IDC_FILE), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue2, buf);
  
  GetWindowText(GetDlgItem(hwnd, IDC_KEY), buf, sizeof(buf));
  SF.realloc_pchar(&g_pCommand->svalue3, buf);
  
  HWND hwndList = GetDlgItem(hwnd, IDC_FRAME);
  int idx = SendMessage(hwndList, CB_GETCURSEL, 0, 0);
  if (CB_ERR != idx) {
    FieldEntry *fe = (FieldEntry *)SendMessage(hwndList, CB_GETITEMDATA, idx, 0);
    g_pCommand->ivalue1 = fe->frame;
    g_pCommand->ivalue2 = fe->field;
    g_pCommand->ivalue3 = fe->keyfld;
  }

  g_pCommand->actiontype = PLUGINNUM;
  SF.set_command(g_pCommand);
  
  LeaveCriticalSection(&g_pCommand->critical_section);  
  EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
  return TRUE;
}
     
static BOOL CALLBACK CommandDialogProc(HWND hwnd,  UINT uMsg, 
                                       WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      char trans[256];
      	
      g_hCommandDialog = hwnd;

      SendMessage(hwnd, WM_SETICON, ICON_SMALL, 
                  (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(PLUGINICON)));

      SetWindowText(hwnd, PLUGINNAME);
			
      SF.i18n_translate("Ok", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDOK), trans);

      SF.i18n_translate("Cancel", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDCANCEL), trans);

      SF.i18n_translate("Apply", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_APPLY), trans);

      SF.i18n_translate("File:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_FILEL), trans);
	
      SF.i18n_translate("Variable:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_VARL), trans);

      SF.i18n_translate("Frame:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_FRAMEL), trans);

      SF.i18n_translate("Key:", trans, sizeof(trans));
      SetWindowText(GetDlgItem(hwnd, IDC_KEYL), trans);

      HWND hwndList = GetDlgItem(hwnd, IDC_FRAME);
      for (size_t i = 0; i < countof(FieldEntries); i++) {
        int idx = SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)FieldEntries[i].name);
        SendMessage(hwndList, CB_SETITEMDATA, idx, (LPARAM)(FieldEntries + i));
      }

      LoadUISettings(hwnd);

      EnableWindow(GetDlgItem(hwnd, IDC_APPLY), FALSE);
      return 0;
    }

  case WM_DESTROY: 
    PostQuitMessage(0); 
    return 0;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (SaveUISettings(hwnd))
        EndDialog(hwnd, TRUE);
      return 1;

    case IDC_APPLY:
      SaveUISettings(hwnd);
      return 1;

    case IDCANCEL:
      EndDialog(hwnd, FALSE);
      return 1;

    case IDC_FILE:
    case IDC_VAR:
    case IDC_KEY:
      if (EN_CHANGE == HIWORD(wParam)) {
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;

    case IDC_FRAME:
      if (CBN_SELENDOK == HIWORD(wParam)) {
        AdjustVisibility(hwnd);
        EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
      }
      break;
    }
    break;

  case WM_USER+100:
    LoadUISettings(hwnd);
    return 1;
  }

  return 0;
}

static DWORD WINAPI CommandThread(LPVOID lpParam)
{
  BOOL result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, 
                               CommandDialogProc, (LPARAM)lpParam);
  g_hCommandDialog = NULL;
  return 0;
}

void OpenCommandUI()
{
   DWORD dwThreadId;
   if (NULL != g_hCommandDialog) {
     SetForegroundWindow(g_hCommandDialog);
   }
   else {
     g_hCommandThread = CreateThread(NULL, 0, &CommandThread, NULL, 0, &dwThreadId);
     if (NULL == g_hCommandThread)
       MessageBox(NULL, "Cannot create dialog thread.", "Error", MB_OK);
   }
}

void UpdateCommandUI(p_command command)
{
  g_pCommand = command;
  if (NULL != g_hCommandDialog)
    SendMessage(g_hCommandDialog, WM_USER+100, 0, 0);
}

void CloseCommandUI()
{
  if (NULL != g_hCommandDialog) {
    SendMessage(g_hCommandDialog, WM_DESTROY, 0, 0);
    WaitForSingleObject(g_hCommandThread, 4000);
    CloseHandle(g_hCommandThread);
  }
}
