#ifndef _ENUMSERIAL_H_
#define _ENUMSERIAL_H_
#include "list.h"
struct SSerInfo {
    ListEntry   Link;
    TCHAR       strDevPath[260];          // Device path for use with CreateFile()
    TCHAR       strPortName[260];         // Simple name (i.e. COM1)
    TCHAR       strFriendlyName[260];     // Full name to be displayed to a user
    BOOL        bUsbDevice;               // Provided through a USB connection?
    TCHAR       strPortDesc[260];         // friendly name without the COMx
};
class EnumSerial
{
private:
    ListEntry               serList;
    void EnumPortsWdm       (ListEntry *);
    void RemoveAll          (ListEntry *);
    void EnumPortsWNt4      (ListEntry *);
    void EnumPortsW9x       (ListEntry *);
    void SearchPnpKeyW9x    (HKEY hkPnp, BOOL bUsbDevice,ListEntry *);
    int  FindStrW           (const int StartPos,const TCHAR * TSourceStr,const int iBytesLength,const TCHAR* TStrToFind,const int CharStrLength);
public:
	EnumSerial              (void);
	~EnumSerial             (void);
    ListEntry * EnumSerialPorts(BOOL bIgnoreBusyPorts=TRUE);	
};









#endif
