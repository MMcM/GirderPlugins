// RealSpy.h -- header for plug-in object.

class CRealSpy : public IRMAPlugin,
                 public IRMAPlayerCreationSink
{
public:
  CRealSpy();
  ~CRealSpy();

  // IUnknown
  STDMETHOD(QueryInterface)(THIS_ REFIID ID, void** ppInterfaceObj);
  STDMETHOD_(UINT32, AddRef)(THIS);
  STDMETHOD_(UINT32, Release)(THIS);

  // IRMAPlugin
  STDMETHOD(GetPluginInfo)(THIS_
                           REF(BOOL) bLoadMultiple,
                           REF(const char*) pDescription,
                           REF(const char*) pCopyright,
                           REF(const char*) pMoreInfoURL,
                           REF(UINT32) versionNumber);
  STDMETHOD(InitPlugin)(THIS_ IUnknown* pRMACore);

  // IRMAPlayerCreationSink
  STDMETHOD(PlayerCreated)(THIS_ IRMAPlayer* pPlayer);
  STDMETHOD(PlayerClosed)(THIS_ IRMAPlayer* pPlayer);

private:
  INT32 m_refCount;
};

extern void DVDSpyEvent(LPCSTR szEvent, LPCSTR szPayload = NULL);
