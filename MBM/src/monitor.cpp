/* Monitor thread */

#include "stdafx.h"
#include "plugin.h"

/* MBM shared memory */

// enums, but make sure the size is right.
typedef unsigned char BusType;
const BusType ISA = 0;
const BusType SMBus = 1;
const BusType VIA686Bus = 2;
const BusType DirectIO = 3;

typedef unsigned char SMBType;
const SMBType smtSMBIntel = 0;
const SMBType smtSMBAMD = 1;
const SMBType smtSMBALi = 2;
const SMBType smtSMBNForce = 3;
const SMBType smtSMBSIS = 4;

// enum Sensor Types
typedef unsigned char SensorType;
const SensorType stUnknown = 0;
const SensorType stTemperature = 1;
const SensorType stVoltage = 2;
const SensorType stFan = 3;
const SensorType stMhz = 4;
const SensorType stPercentage = 5;

struct SharedIndex {
  SensorType iType;             // type of sensor
  int Count;                    // number of sensor for that type
};

struct SharedSensor {
  SensorType ssType;            // type of sensor
  char ssName[12];              // name of sensor
  char sspadding1[3];           // padding of 3 byte
  double ssCurrent;             // current value
  double ssLow;                 // lowest readout
  double ssHigh;                // highest readout
  long ssCount;                 // total number of readout
  char sspadding2[4];           // padding of 4 byte
  long double ssTotal;          // total amout of all readouts
  char sspadding3[6];           // padding of 6 byte
  double ssAlarm1;              // temp & fan: high alarm; voltage: % off;
  double ssAlarm2;              // temp: low alarm
};

struct SharedInfo {
  WORD siSMB_Base;              // SMBus base address
  BusType siSMB_Type;           // SMBus/Isa bus used to access chip
  SMBType siSMB_Code;           // SMBus sub type, Intel, AMD or ALi
  BYTE siSMB_Addr;              // Address of sensor chip on SMBus
  char siSMB_Name[41];          // Nice name for SMBus
  WORD siISA_Base;              // ISA base address of sensor chip on ISA
  int siChipType;               // Chip nr, connects with Chipinfo.ini
  BYTE siVoltageSubType;        // Subvoltage option selected
};

struct SharedData {
  double sdVersion;             // version number (example: 51090)
  SharedIndex sdIndex[10];      // Sensor index
  SharedSensor sdSensor[100];   // sensor info
  SharedInfo sdInfo;            // misc. info
  char sdStart[41];             // start time
  char sdCurrent[41];           // current time
  char sdPath[256];             // MBM path
};

HANDLE g_hSD = NULL;
SharedData *g_pSD = NULL;
BOOL g_bEnabled[NSENSORS];
double g_Values[NSENSORS];
long g_nInterval;

HANDLE g_hMonitorThread = NULL;
HANDLE g_hMonitorEvent = NULL;

BOOL OpenSharedMemory()
{
  if (NULL != g_pSD)
    return TRUE;

  g_hSD = OpenFileMapping(FILE_MAP_READ, FALSE, "$M$B$M$5$S$D$");
  if (NULL == g_hSD)
    return FALSE;

  g_pSD = (SharedData *)MapViewOfFile(g_hSD, FILE_MAP_READ, 0, 0, 0);
  if (NULL == g_pSD)
    return FALSE;
  
  memset(&g_bEnabled, 0, NSENSORS * sizeof(BOOL));
  
  char iniPath[MAX_PATH];
  strcpy(iniPath, g_pSD->sdPath);
  strcat(iniPath, "\\Data\\MBM 5.ini");

  // Refresh time in ms.
  g_nInterval = GetPrivateProfileInt("General", "Interval Time", 1000, iniPath) * 10;
  
  int ntemp = 0, nvolt = 0, nfan = 0;
  for (int i = 0; i < NSENSORS; i++) {
    const char *section;
    char key[64], *ep;
    BOOL checkSensor = FALSE;
    switch (g_pSD->sdSensor[i].ssType) {
    case stTemperature:
      section = "Temperature";
      sprintf(key, "T%d", ++ntemp);
      checkSensor = TRUE;
      break;
    case stVoltage:
      section = "Voltage";
      sprintf(key, "V%d", ++nvolt);
      checkSensor = TRUE;
      break;
    case stFan:
      section = "Fan";
      sprintf(key, "F%d", ++nfan);
      checkSensor = TRUE;
      break;
    case stMhz:
      section = "CPU";
      strcpy(key, "CPU");
      break;
    case stPercentage:
      section = "CPU";
      strcpy(key, "CPU Usage");
      break;
    default:
      // [3rd Party] Pn?
      continue;
    }
    ep = key + strlen(key);
    *ep++ = ' ';
    strcpy(ep, "Display On Dashboard");
    if (0 == GetPrivateProfileInt(section, key, 0, iniPath))
      continue;
    if (checkSensor) {
      strcpy(ep, "Sensor");
      if (0 == GetPrivateProfileInt(section, key, 0, iniPath))
        continue;
    }
    g_bEnabled[i] = TRUE;
  }

  return TRUE;
}

void CloseSharedMemory()
{
  if (NULL != g_pSD) {
    UnmapViewOfFile(g_pSD);
    g_pSD = NULL;
  }
  if (NULL != g_hSD) {
    CloseHandle(g_hSD);
    g_hSD = NULL;
  }
}

DWORD WINAPI MonitorThread(LPVOID lpParam)
{
  HANDLE hEvent = (HANDLE)lpParam;

  // Fill with infinities (NaN would make more sense, but compares strangely).
  for (size_t i = 0; i < NSENSORS; i++) {
    LPBYTE pb = (LPBYTE)&g_Values[i];
    pb[7] = 0x7F;
    pb[6] = 0xF0;
    pb[5] = pb[4] = pb[3] = pb[2] = pb[1] = pb[0] = 0;
  }

  while (TRUE) {
    for (i = 0; i < NSENSORS; i++) {
      SharedSensor *pss = g_pSD->sdSensor + i;
      if (g_bEnabled[i]) {
        double curr = pss->ssCurrent;
        if (g_Values[i] == curr)
          continue;
        g_Values[i] = curr;

        char buf[32];
        buf[0] = 1;
        sprintf(buf + 1, "%g", curr);
        SF.send_event(pss->ssName, buf, strlen(buf) + 1, PLUGINNUM);
      }
    }

    // Check for wakeup from main thread.
    if (WAIT_TIMEOUT != WaitForSingleObject(hEvent, g_nInterval))
      break;
  }
  return 0;
}

BOOL StartMonitor()
{
  if (!OpenSharedMemory())
    return FALSE;

  if (NULL != g_hMonitorThread)
    return TRUE;

  g_hMonitorEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  DWORD dwThreadId;
  g_hMonitorThread = CreateThread(NULL, 0, MonitorThread, g_hMonitorEvent, 
                                  0, &dwThreadId);
  if (NULL == g_hMonitorThread) {
    MessageBox(0, "Cannot create monitor thread.", "Error", MB_OK);
    return FALSE;
  }
  
  return TRUE;
}

void StopMonitor()
{
  if (NULL == g_hMonitorThread)
    return;

  SetEvent(g_hMonitorEvent);
  CloseHandle(g_hMonitorEvent);
  g_hMonitorEvent = NULL;

  WaitForSingleObject(g_hMonitorThread, INFINITE);
  CloseHandle(g_hMonitorThread);
  g_hMonitorThread = NULL;
}

LPCSTR GetEventName(size_t i)
{
  if (!OpenSharedMemory())
    return NULL;

  if (!g_bEnabled[i])
    return NULL;

  return g_pSD->sdSensor[i].ssName;
}
