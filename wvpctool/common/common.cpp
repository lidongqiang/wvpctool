#include "stdafx.h"
#include "common.h"
#include <Dbt.h>
#include <Setupapi.h>

#define INTERFACE_NUM 2
const GUID GUID_USB_COM     = {0x4d36e978, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18};
const GUID GUID_GOOGLE_ADB  = {0xF72FE0D4, 0xCBCB, 0x407d, 0x88, 0x14, 0x9E, 0xD6, 0x73, 0xD0, 0xDD, 0x6B};
GUID        g_GUID[INTERFACE_NUM] = {GUID_GOOGLE_ADB, GUID_USB_COM};
HDEVNOTIFY  g_HDEVNOTIFY[INTERFACE_NUM] = {0, 0};

void RegDevNotification(HWND hWnd)
{
    DEV_BROADCAST_DEVICEINTERFACE   NotificationFilter;
    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    for (int i=0;i<INTERFACE_NUM;i++) {
        NotificationFilter.dbcc_classguid = g_GUID[i];
        g_HDEVNOTIFY[i] = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
    }
}

PORTNAME portsname[PORT_PORT_NUM] = 
{
    {_T("Intel Mobile GTI/IPICOM")  ,_T("Intel Mobil GTI/IPICOM")   },
    {_T("Intel Mobile AT Commands") ,_T("Intel Mobil AT Commands")  },
    {_T("Intel Mobile OCT Trace")   ,_T("Intel Mobil OCT Trace")    },
    {_T("IMC Port (USB1)")          ,_T("IMC Port (USB1)")          }

};

std::wstring GetComPortFromFriendlyName(const TCHAR * friendlyName)
{
    int             fnlen;
    std::wstring    strfriendlyName,strPortName;

    fnlen = _tcslen(friendlyName);
    if (0 == fnlen){
        return TEXT("");
    }
    strfriendlyName = friendlyName;
    size_t compos   = strfriendlyName.find_last_of(TEXT("(COM"));
    if(std::string::npos != compos&&compos + 1 < fnlen ) {
        compos += 1;
        size_t endpos   = strfriendlyName.find_last_of(TEXT(")"));
        if(std::string::npos != endpos&&endpos > compos ) {
            strPortName = strfriendlyName.substr(compos,endpos - compos);
            return strPortName;
        }
    }
    return TEXT("");;
}


bool CheckIntelUSBComPort(INTELCOMPORT *ports,CLogger *m_pLog)
{
    DWORD           DataT;
    LPTSTR          buffer      = NULL;
    DWORD           buffersize  = 0;
    bool            bOK         = FALSE;
    HDEVINFO        hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    ports->clear();
    hDevInfo    = SetupDiGetClassDevs(&GUID_USB_COM, NULL, NULL, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) 
        return FALSE;
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
        buffersize  = 0;
		while (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, &DataT, (PBYTE)buffer, buffersize, &buffersize)) {
            DWORD dwLastErr = GetLastError();
			if (dwLastErr == ERROR_INSUFFICIENT_BUFFER) {
				buffer = (LPTSTR)LocalAlloc(LPTR, buffersize);
                if(buffer)
                    memset(buffer,0,buffersize);
			} else {
				break;
			}
		}
        if(NULL == buffer) {
            break;
        }
        for(int i = 0;i < PORT_PORT_NUM;i ++ ) {
            if(_tcsstr(buffer,portsname[i].PortName1) || _tcsstr(buffer,portsname[i].PortName2)) {
                ports->IntelPort[i] = GetComPortFromFriendlyName(buffer);
                break;
            }
        }
        if (buffer) {
            LocalFree(buffer);
            buffer = NULL;
        }
        if((!ports->IntelPort[PORT_GTI].empty())&&(!ports->IntelPort[PORT_AT].empty())) {
            bOK = TRUE;
		    break;
        }

	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return bOK;
}

BYTE hexstr2ushort(const char * str,int len)
{
    BYTE            usRet   = 0;
    int             j;
    int             nlen;
    if(0 > len ) {
        nlen = strlen(str);
    } else {
        nlen = len;
    }
    if( 0 == nlen ) return usRet;
    if (2 < nlen) {
        j = 2;
    }
    for(int i = 0 ; i < nlen ;i ++ ) {
        BYTE val = 0;  
        if(str[i]>='A'&&str[i]<='F') {
            val = (str[i]-'A'+10);
        } else if (str[i]>='a'&&str[i]<='f') {
            val = (str[i]-'a'+10);
        } else if (str[i]>='0'&&str[i]<='9') {
            val = (str[i]-'0');
        }
        usRet = usRet * 16+ val;
    }
    return usRet;
}

