
#ifdef DISPLAYSPYHOOK_EXPORTS
#define DISPLAYSPYHOOK_API __declspec(dllexport) __stdcall
#else
#define DISPLAYSPYHOOK_API __declspec(dllimport) __stdcall
#endif

#define WM_NEWDISPLAY WM_USER+10

extern "C" {
void DISPLAYSPYHOOK_API DS_StartHook(DWORD dwThreadId);
void DISPLAYSPYHOOK_API DS_EndHook(DWORD dwThreadId);
size_t DISPLAYSPYHOOK_API DS_GetMatchCount();
size_t DISPLAYSPYHOOK_API DS_GetMatchIndexCount(size_t nCurrent);
void DISPLAYSPYHOOK_API DS_GetMatchName(size_t nCurrent, char *szBuf, size_t nSize);
size_t DISPLAYSPYHOOK_API DS_GetMatchIndex(LPCSTR szName);
void DISPLAYSPYHOOK_API DS_EnableMatch(size_t nIndex, BOOL bEnabled);
BOOL DISPLAYSPYHOOK_API DS_GetNext(size_t *nMatch, size_t *nIndex,
                                   char *szBuf, size_t nSize);
void DISPLAYSPYHOOK_API DS_Reset();
}
