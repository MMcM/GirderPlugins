// PlayerSpy.h -- header for advice sink object.

const size_t NPROPNAMES = 2;

class CPlayerSpy : public IRMAClientAdviseSink
{
public:
  CPlayerSpy();
  ~CPlayerSpy();

  void Init(IRMAPlayer* pPlayer);

  // IUnknown
  STDMETHOD(QueryInterface)(THIS_ REFIID ID, void** ppInterfaceObj);
  STDMETHOD_(UINT32, AddRef)(THIS);
  STDMETHOD_(UINT32, Release)(THIS);

  // IRMAClientAdviseSink
  STDMETHOD(OnPosLength)(THIS_ UINT32 ulPosition, UINT32 ulLength);
  STDMETHOD(OnPresentationOpened)(THIS);
  STDMETHOD(OnPresentationClosed)(THIS);
  STDMETHOD(OnStatisticsChanged)(THIS);
  STDMETHOD(OnPreSeek)(THIS_ UINT32 ulOldTime, UINT32 ulNewTime);
  STDMETHOD(OnPostSeek)(THIS_ UINT32 ulOldTime, UINT32 ulNewTime);
  STDMETHOD(OnStop)(THIS);
  STDMETHOD(OnPause)(THIS_ UINT32 ulTime);
  STDMETHOD(OnBegin)(THIS_ UINT32 ulTime);
  STDMETHOD(OnBuffering)(THIS_ UINT32 ulFlags, UINT16 unPercentComplete);
  STDMETHOD(OnContacting)(THIS_ const char* pHostName);

protected:
  void TimeEvent(LPCSTR szEvent, UINT32 ulTime);

private:
  INT32 m_refCount;
  IRMAPlayer* m_pPlayer;
  IRMAPNRegistry* m_pRegistry;
  UINT32 m_nPropIDs[NPROPNAMES];
  UINT32 m_ulPosition, m_ulLength;
  UINT16 m_unBuffering;
};
