/* Winamp <-> Girder IPC */

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "ipc.h"

/** Really simple shared memory allocation using a fixed number of
 ** fixed size buffers.  Good enough for our purposes.  This is
 ** superior to using WM_COPYDATA because it works with PostMessage,
 ** so events, which are common, don't hang up the sender.  It also
 ** allows for string data in synchronous replies.
 **/

// Amount of data in a single buffer.
#define MAXDATA 512

struct IPCBuffer {
  size_t next;
  size_t length;
  IPC_HANDLE src, dest;
  IPC_OPCODE op;
  int seqno;
  char data[MAXDATA];
};

// Number of buffers.
#define NBUFS 16

struct IPCBase {
  size_t free;
  IPC_HANDLE master;
  IPCBuffer bufs[NBUFS];
};

static HANDLE g_shmemHandle = NULL;
static HANDLE g_shmemMutex = NULL;
static IPCBase *g_shmemBase = NULL;
static IPC_HANDLE g_ipcHandle = NULL;
static BOOL g_open = FALSE;

inline size_t BufferOffset(IPCBuffer *buf)
{
  return (char *)buf - (char *)g_shmemBase;
}

inline IPCBuffer *OffsetBuffer(size_t off)
{
  return (IPCBuffer *)((char *)g_shmemBase + off);
}

inline IPCBuffer *NextBuffer(IPCBuffer *buf)
{
  size_t off = buf->next;
  if (0 == off)
    return NULL;
  else
    return OffsetBuffer(off);
}

IPCBuffer *AllocateBuffer()
{
  size_t off;
  for (int i = 0; i < 4; i++) {
    if (WAIT_OBJECT_0 != WaitForSingleObject(g_shmemMutex, 100))
      return NULL;
    off = g_shmemBase->free;
    if (0 != off)
      break;
    ReleaseMutex(g_shmemMutex);
    // All memory in use; wait a little bit.
    Sleep(25);
  }
  IPCBuffer *result = OffsetBuffer(off);
  g_shmemBase->free = result->next;
  ReleaseMutex(g_shmemMutex);
  return result;
}

// Free (a chain of) buffer(s).
void FreeBuffer(IPCBuffer *buf)
{
  if (WAIT_OBJECT_0 != WaitForSingleObject(g_shmemMutex, 500))
    return;                     // Better to lose memory or wait?
  while (NULL != buf) {
    IPCBuffer *next = NextBuffer(buf);
    buf->next = g_shmemBase->free;
    g_shmemBase->free = BufferOffset(buf);
    buf = next;
  }
  ReleaseMutex(g_shmemMutex);
}

BOOL IPCMessage::IsInit()
{
  return g_open;
}

BOOL IPCMessage::Init(IPC_HANDLE handle, BOOL master)
{
  if (g_open) 
    return TRUE;
  
  if (master)
    g_shmemHandle = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 
                                      0, sizeof(IPCBase),
                                      "Girder Winamp3 Shared Memory");
  else
    g_shmemHandle = OpenFileMapping(FILE_MAP_WRITE, FALSE,
                                    "Girder Winamp3 Shared Memory");
  if (NULL == g_shmemHandle)
    return FALSE;
  
  g_shmemBase = (IPCBase *)MapViewOfFile(g_shmemHandle, FILE_MAP_WRITE, 
                                         0, 0, 0);
  if (NULL == g_shmemBase) {
    Term();
    return FALSE;
  }
  
  if (master)
    g_shmemMutex = CreateMutex(NULL, TRUE,
                               "Girder Winamp3 Mutex");
  else
    g_shmemMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE,
                             "Girder Winamp3 Mutex");
  if (NULL == g_shmemMutex) {
    Term();
    return FALSE;
  }
    
  if (master) {
    g_shmemBase->master = handle;
    g_shmemBase->free = 0;
    for (size_t i = 0; i < NBUFS; i++) {
      IPCBuffer *buf = g_shmemBase->bufs + i;
      buf->next = g_shmemBase->free;
      g_shmemBase->free = BufferOffset(buf);
    }
    ReleaseMutex(g_shmemMutex);
  }

  g_ipcHandle = handle;
  g_open = TRUE;
  return TRUE;
}

void IPCMessage::Term()
{
  g_open = FALSE;
  
  if (NULL != g_shmemBase) {
    if (g_ipcHandle == g_shmemBase->master) {
      g_shmemBase->master = NULL; // No master to communicate with.
      g_shmemBase->free = 0;    // No available buffers.
    }
  }

  if (NULL != g_shmemMutex) {
    CloseHandle(g_shmemMutex);
    g_shmemMutex = NULL;
  }
  if (NULL != g_shmemHandle) {
    CloseHandle(g_shmemHandle);
    g_shmemHandle = NULL;
  }
}

IPC_HANDLE IPCMessage::GetMaster()
{
  if (g_open)
    return g_shmemBase->master;
  else
    return NULL;
}

void IPCMessage::ctor(IPC_OPCODE op, IPC_HANDLE dest)
{
  IPCBuffer *buf = AllocateBuffer();
  m_head = m_buf = buf;
  m_length = m_cursor = 0;
  if (NULL == buf)
    return;                     // Out of memory (throw exception?)
  buf->next = 0;
  buf->src = g_ipcHandle;
  buf->dest = dest;
  buf->op = op;
  buf->seqno = 1;
}

 IPCMessage::IPCMessage(LPARAM lParam)
{
  IPCBuffer *buf = OffsetBuffer(lParam);
  m_head = m_buf = buf;
  m_length = buf->length;
  m_cursor = 0;
}

 IPCMessage::~IPCMessage()
{
  if (NULL != m_head) {
    FreeBuffer(m_head);
  }
}

IPC_OPCODE IPCMessage::getOpcode()
{
  return m_buf->op;
}

IPC_HANDLE IPCMessage::getSource()
{
  return m_buf->src;
}

IPC_HANDLE IPCMessage::getDestination()
{
  return m_buf->dest;
}

void IPCMessage::addData(IPC_DATATYPE dt, const void *data, size_t len)
{
  size_t nlen = m_length + len + 1;
  if (nlen > MAXDATA) {
    IPCBuffer *next = AllocateBuffer();
    if (NULL == next) return;   // Out of memory.
    m_buf->next = BufferOffset(next);
    next->next = 0;
    next->src = m_buf->src;
    next->dest = m_buf->dest;
    next->op = m_buf->op;
    next->seqno = m_buf->seqno + 1;
    m_buf = next;
    // Don't bother with cursor; same message never used for both read and write.
    m_length = 0;
    if (nlen > MAXDATA) {
      nlen = len = MAXDATA - 2;
      // Can only happen for too long strings, so force premature nul.
      m_buf->data[nlen++] = 0;
    }
  }
  m_buf->data[m_length++] = dt;
  memcpy(m_buf->data + m_length, data, len);
  m_length = nlen;
}

const char *IPCMessage::nextString()
{
  while (m_cursor >= m_length) {
    if (NULL == m_buf) 
      return NULL;
    m_buf = NextBuffer(m_buf);
    m_length = (NULL == m_buf) ? 0 : m_buf->length;
    m_cursor = 0;
  }
  if (IPC_STRING != m_buf->data[m_cursor++])
    return NULL;
  const char *str = m_buf->data + m_cursor;
  m_cursor += strlen(str) + 1;
  return str;
}

BOOL IPCMessage::nextData(IPC_DATATYPE dt, void *data, size_t len)
{
  while (m_cursor + len > m_length) {
    if (NULL == m_buf) 
      return FALSE;
    m_buf = NextBuffer(m_buf);
    m_length = (NULL == m_buf) ? 0 : m_buf->length;
    m_cursor = 0;
  }
  if (dt != m_buf->data[m_cursor++])
    return FALSE;
  memcpy(data, m_buf->data + m_cursor, len);
  m_cursor += len;
  return TRUE;
}

int IPCMessage::nextInt()
{
  int result = 0;
  nextData(IPC_INT, &result, sizeof(int));
  return result;
}

double IPCMessage::nextDouble()
{
  double result = 0;
  nextData(IPC_DOUBLE, &result, sizeof(int));
  return result;
}

void IPCMessage::reuse()
{
  IPCBuffer *extra = NextBuffer(m_head);
  if (NULL != extra) {
    m_head->next = 0;
    FreeBuffer(extra);
  }
  m_buf = m_head;
  m_length = m_cursor = 0;
}

LRESULT IPCMessage::detach()
{
  m_buf->length = m_length;
  size_t result = BufferOffset(m_head);
  m_head = m_buf = NULL;
  return result;
}

BOOL IPCMessage::send(BOOL reply)
{
  IPCBuffer *buf = m_head;
  if ((NULL == buf) || (NULL == buf->dest))
    return FALSE;

  m_head = NULL;

  buf->length = m_length;

  if (!reply) {
    if (!PostMessage(buf->dest, WM_IPC, buf->op, BufferOffset(buf))) {
      m_head = buf;             // Put it back so freed.
      return FALSE;
    }
  }
  else {
    LRESULT r = SendMessage(buf->dest, WM_IPC, buf->op, BufferOffset(buf));
    if (0 == r) {
      // This assumes that a synchronous failure return does not free
      // in the receiver, so that it is freed in the case where the
      // destination doesn't exist or drops the message completely.
      m_head = buf;
      return FALSE;
    }
    else {
      IPCBuffer *buf = OffsetBuffer(r);
      m_head = m_buf = buf;
      m_length = buf->length;
      m_cursor = 0;
    }
  }
  return TRUE;
}
