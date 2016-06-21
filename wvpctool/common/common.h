#ifndef _COMMON_H_
#define _COMMON_H_
#include <string>
#include "debug.h"
#define WM_UPDATE_COM		(WM_USER+111)
#define WM_COM_ADD		    1
#define WM_COM_RM		    2
#define UPDATE_LIST         3
#define LIST_EMPTY          4
#define UPDATE_TEST_EXIT    5

#define LIST_INFO 0
#define LIST_TIME 1
#define LIST_WARN 2
#define LIST_ERR  3
typedef enum
{
	PORT_GTI = 0,
	PORT_AT,
	PORT_OCT,
	PORT_CORE_DUMP,
    PORT_PORT_NUM
}USB_PORT_TYPE;

typedef struct _PORTNAME_
{
    const TCHAR *PortName1;
    const TCHAR *PortName2;
}PORTNAME;

class INTELCOMPORT
{
public:
    void clear()
    {
        GtiPort.clear();
        AtPort.clear();
        OctPort.clear();
        ImcPort.clear();
        for(int i = 0; i < PORT_PORT_NUM ;i ++) {
            IntelPort[i].clear();

        }
    }
    std::wstring GtiPort;
    std::wstring AtPort;
    std::wstring OctPort;
    std::wstring ImcPort;
    std::wstring IntelPort[PORT_PORT_NUM];
};

void          RegDevNotification(HWND hWnd);
bool          CheckIntelUSBComPort(INTELCOMPORT *ports,CLogger *m_pLog = NULL);
unsigned char hexstr2ushort(const char * str,int len);
#endif