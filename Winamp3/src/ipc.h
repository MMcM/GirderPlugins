/* Winamp <-> Girder IPC */

typedef HWND IPC_HANDLE;

const UINT WM_IPC = WM_USER+123;

enum IPC_DATATYPE {
  IPC_STRING = 1,
  IPC_INT,
  IPC_DOUBLE
};

enum IPC_OPCODE {
  // Commands (asynchronous) Girder -> Winamp
  IPC_C_START = 1,              // ()
  IPC_C_STOP,                   // ()
  ICP_C_PAUSE,                  // ()
  IPC_C_RESUME,                 // ()
  IPC_C_PAUSE_TOGGLE,           // ()
  IPC_C_SEEK_ABSOLUTE,          // (ms)
  IPC_C_SEEK_RELATIVE,          // (ms)
  IPC_C_NEXT,                   // ()
  IPC_C_PREV,                   // ()
  IPC_C_SET_NEXT,               // (playstring)
  IPC_C_TRACK,                  // (pos)
  
  // Requests (synchronous) Girder -> Winamp -> Girder
  IPC_R_STATE = 100,            // () -> (state)
  IPC_R_PLAYSTRING,             // () -> (playstring)
  IPC_R_METADATA,               // (tag, file) -> (value)
  IPC_R_TRACK,                  // () -> (pos, length)
  IPC_R_POSITION,               // () -> (elapsed, duration)
  IPC_R_ITEM,                   // (pos) -> (playstring)
  
  // Events (asynchronous) Winamp -> Girder
  IPC_E_HELLO = 1000,           // (prefix)
  IPC_E_GOODBYE,                // ()
  IPC_E_START,                  // (pos, length)
  IPC_E_STOP,                   // ()
  IPC_E_PAUSE,                  // (elapsed, duration)
  IPC_E_RESUME,                 // (elapsed, duration)
  IPC_E_SEEK,                   // (elapsed, duration)
  IPC_E_PLAYING,                // (elapsed, duration)
  IPC_E_PLAYSTRING,             // (playstring, {tag, value}...)
};

struct IPCBuffer;

class IPCMessage {
public:
  static BOOL IsInit();
  static BOOL Init(IPC_HANDLE handle, BOOL master = FALSE);
  static void Term();
  static IPC_HANDLE GetMaster();

  IPCMessage(IPC_OPCODE op) {
    ctor(op, GetMaster());
  }
  IPCMessage(IPC_OPCODE op, IPC_HANDLE dest) {
    ctor(op, dest);
  }
  IPCMessage(LPARAM lParam);
  ~IPCMessage();

  IPC_OPCODE getOpcode();
  IPC_HANDLE getSource();
  IPC_HANDLE getDestination();

  void addData(IPC_DATATYPE dt, const void *data, size_t len);

  void addString(const char *str) {
    addData(IPC_STRING, str, strlen(str)+1);
  }
  void addInt(int i) {
    addData(IPC_INT, &i, sizeof(int));
  }
  void addDouble(double d) {
    addData(IPC_DOUBLE, &d, sizeof(double));
  }

  const char *nextString();
  BOOL nextData(IPC_DATATYPE dt, void *data, size_t len);
  int nextInt();
  double nextDouble();
  
  IPCMessage& operator<<(const char *str) {
    addString(str);
    return *this;
  }
  IPCMessage& operator<<(int i) {
    addInt(i);
    return *this;
  }
  IPCMessage& operator<<(double d) {
    addDouble(d);
    return *this;
  }
  IPCMessage& operator>>(const char *&str) {
    str = nextString();
    return *this;
  }
  IPCMessage& operator>>(int& i) {
    i = nextInt();
    return *this;
  }
  IPCMessage& operator>>(double d) {
    d = nextDouble();
    return *this;
  }

  void reuse();
  LRESULT detach();
  BOOL send(BOOL reply = false);

private:
  void ctor(IPC_OPCODE op, IPC_HANDLE dest);

  IPCBuffer *m_head, *m_buf;
  size_t m_length, m_cursor;
};
