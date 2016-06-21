#include "stdafx.h"
#include <Setupapi.h>
#include "EnumSerial.h"
#define LDEGMSGW(x) 
EnumSerial::EnumSerial(void)
{
    //Initializing list
    InitializeListHead(&serList);
}
EnumSerial::~EnumSerial(void)
{
    //Empty list
    RemoveAll(&serList);
}
void EnumSerial::RemoveAll(ListEntry *list)
{
    //Empty list
#if defined(_DEBUG)
    int         i = 0;
#endif
    ListEntry   *node;
    SSerInfo    *pInfo;
    for(node = serList.Flink;node != &serList ;){
        pInfo   = CONTAINING_RECORD(node, SSerInfo, Link);
        node    = node->Flink;
        RemoveEntryList(&pInfo->Link);
        delete  pInfo;
#if defined(_DEBUG)
        i ++;
#endif
    }
#if defined(_DEBUG)
    //LDEGMSGW((CLogger::DEBUG_INFO,TEXT("@@RemoveAll():%d\r\n"),i));
#endif
    
}
int EnumSerial::FindStrW(const int StartPos,const TCHAR * TSourceStr,const int iBytesLength,const TCHAR* TStrToFind,const int CharStrLength)
{
    BOOL bResult = TRUE;
    for (int i = StartPos; i< iBytesLength - CharStrLength + 1; i++ )
    {
        bResult = TRUE;
        for (int j = 0; j < CharStrLength; j++)
        {
            if (TSourceStr[i + j] != TStrToFind[j] )
            {
                bResult = FALSE;
                break;
            }
        }
        if ( bResult )
        {
            return i;
        }
    }
    return -1;
}

ListEntry *EnumSerial::EnumSerialPorts(BOOL bIgnoreBusyPorts)
{
    OSVERSIONINFO   vi;
    int             startdex;
    int             enddex;
    ListEntry       *node;
    SSerInfo        *pInfo;
    RemoveAll(&serList);
    vi.dwOSVersionInfoSize  = sizeof(vi);
    if (!::GetVersionEx(&vi)) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("@@GetVersionEx:ERROR(err=%d)\r\n"),GetLastError()));
        return &serList;
    }
    // Handle windows 9x and NT4 specially
    if (vi.dwMajorVersion < 5) {
        if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT){
            EnumPortsWNt4(&serList);
        } else {
            EnumPortsW9x(&serList);
        }
    } else {
        // Win2k and later support a standard API for
        // enumerating hardware devices.
        EnumPortsWdm(&serList);
    }
    for (node = serList.Flink; node != &serList; node = node->Flink)
	{
        pInfo = CONTAINING_RECORD(node, SSerInfo, Link);
		if (bIgnoreBusyPorts) {
			// Only display ports that can be opened for read/write
			HANDLE hCom = CreateFile(pInfo->strDevPath,
				GENERIC_READ | GENERIC_WRITE,
				0,    /* comm devices must be opened w/exclusive-access */
				NULL, /* no security attrs */
				OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
				0,    /* not overlapped I/O */
				NULL  /* hTemplate must be NULL for comm devices */
				);
			if (hCom == INVALID_HANDLE_VALUE) {
				// It can't be opened; remove it.
                RemoveEntryList(&pInfo->Link);
				delete pInfo;       
				continue;
			} else {
				// It can be opened! Close it and add it to the list
				::CloseHandle(hCom);
			}
		}
		// Come up with a name for the device.
		// If there is no friendly name, use the port name.
        
		if (0 == wcslen(pInfo->strFriendlyName)){
            wsprintf(pInfo->strFriendlyName,pInfo->strPortName);
        }

		// If there is no description, try to make one up from
		// the friendly name.
		if (0 == wcslen(pInfo->strPortDesc)) {
			// If the port name is of the form "ACME Port (COM3)"
			// then strip off the " (COM3)"
            wsprintf(pInfo->strPortDesc,pInfo->strFriendlyName);
            startdex    = FindStrW(0,pInfo->strPortDesc,wcslen(pInfo->strPortDesc),TEXT(" ("),wcslen(TEXT(" (")));
            enddex      = FindStrW(0,pInfo->strPortDesc,wcslen(pInfo->strPortDesc),TEXT(")"),wcslen(TEXT(")")));
			if (startdex > 0 && enddex == (wcslen(pInfo->strPortDesc) -1 )){
                memset(pInfo->strPortDesc,0,sizeof(pInfo->strPortDesc));
                memcpy(pInfo->strPortDesc,pInfo->strFriendlyName,startdex*sizeof(TCHAR));
            }
		}
	}
    //if(!IsListEmpty(&serList)) {
        return &serList;
    //} else {
    //    return NULL;
    //}
}
void EnumSerial::EnumPortsWdm(ListEntry *list)
{
    SSerInfo    *psi;
    HDEVINFO    hDevInfo        = INVALID_HANDLE_VALUE;
    GUID        *guidDev        = (GUID*) &GUID_CLASS_COMPORT;
    BOOL        bOk             = TRUE;
    DWORD       dwDetDataSize;
    DWORD       err;
    TCHAR       fname[256];
    TCHAR       strDevPath[256];
    TCHAR       desc[256];
    TCHAR       locinfo[256];
    BOOL        bSuccess;
    DWORD       ii;
    BOOL        bUsbDevice;
    SP_DEVICE_INTERFACE_DATA        ifcData;
    SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData;
    SP_DEVINFO_DATA                 devdata;
    hDevInfo = SetupDiGetClassDevs( guidDev,
                                    NULL,
                                    NULL,
                                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
                                    );
    if(INVALID_HANDLE_VALUE == hDevInfo){
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("@@SetupDiGetClassDevs():ERROR(err=%d)\r\n"),GetLastError()));
        return ;
    }
    dwDetDataSize       =   sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
    pDetData            = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[dwDetDataSize];
    ifcData.cbSize      = sizeof(SP_DEVICE_INTERFACE_DATA);
    pDetData->cbSize    = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    for ( ii = 0; bOk; ii++) {
        bOk = SetupDiEnumDeviceInterfaces(hDevInfo,NULL, guidDev, ii, &ifcData);
        if (bOk) {
            // Got a device. Get the details.
            devdata.cbSize = sizeof(SP_DEVINFO_DATA);
            bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo,&ifcData, pDetData, dwDetDataSize, NULL, &devdata);
            if (bOk) {
                wsprintf(strDevPath,pDetData->DevicePath);
                bSuccess    = SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,(PBYTE)fname, sizeof(fname), NULL);
                bSuccess    = bSuccess && SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,(PBYTE)desc, sizeof(desc), NULL);
                bUsbDevice  = FALSE;
                if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_LOCATION_INFORMATION, NULL,(PBYTE)locinfo, sizeof(locinfo), NULL)) {
                    bUsbDevice = (_tcsncmp(locinfo, TEXT("USB"), 3)==0);
                }
                if (bSuccess) {
                    psi = new SSerInfo();
                    wsprintf(psi->strDevPath,strDevPath);
                    //si.strDevPath       = strDevPath;
                    wsprintf(psi->strFriendlyName,fname);
                    //si.strFriendlyName  = fname;
                    wsprintf(psi->strPortDesc,desc);
                    //si.strPortDesc      = desc;
                    wsprintf(psi->strDevPath,strDevPath);
                    psi->bUsbDevice       = bUsbDevice;
                    //asi.Add(si);
                    InsertTailList(list,&psi->Link);
                }
            } else {
                LDEGMSGW((CLogger::DEBUG_INFO,TEXT("@@SetupDiGetDeviceInterfaceDetail():ERROR(err=%d)\r\n"),GetLastError()));
            }
        } else {
            err = GetLastError();
            if (err != ERROR_NO_MORE_ITEMS) {
                LDEGMSGW((CLogger::DEBUG_INFO,TEXT("err != ERROR_NO_MORE_ITEMS\r\n")));
            }
        }
    }
    if (NULL != pDetData ) {
        delete[] pDetData;
    }
    if (INVALID_HANDLE_VALUE != hDevInfo){ 
        // must delete the returned device information set when it is no longer needed by calling
        SetupDiDestroyDeviceInfoList(hDevInfo);
        hDevInfo    = INVALID_HANDLE_VALUE;
    }
}

void EnumSerial::EnumPortsWNt4      (ListEntry * list)
{
    // NT4's driver model is totally different, and not that
	// many people use NT4 anymore. Just try all the COM ports
	// between 1 and 16
    SSerInfo    *psi;
    TCHAR       szPort[32];
    int         ii;
	for ( ii=1; ii<= 16; ii++) {
        psi = new SSerInfo();
        wsprintf(szPort,TEXT("COM%d"),ii);
        wsprintf(psi->strDevPath,TEXT("\\\\.\\%s"),szPort);
        wsprintf(psi->strPortName,TEXT("%s"),szPort);
        InsertTailList(list,&psi->Link);
	}

}
void EnumSerial::EnumPortsW9x(ListEntry * list)
{
    HKEY    hkEnum          = NULL;
	HKEY    hkSubSubEnum    = NULL;
    //HKEY    hkSubSubEnum    = NULL;
    BOOL    bMatch;
    BOOL    bUsbDevice;
    DWORD   dwSubSubEnumIndex;
    TCHAR   szSubEnum[128];
    TCHAR   szSubSubEnum[128];
    DWORD   dwSubEnumIndex  = 0;
    HKEY    hkSubEnum       = NULL;
    DWORD   dwSize          = sizeof(szSubEnum);
    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("++EnumPortsW9x()\r\n")));
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Enum"), 0, KEY_READ,&hkEnum) == ERROR_SUCCESS){
        while (RegEnumKeyEx(hkEnum, dwSubEnumIndex++, szSubEnum, &dwSize,NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("@@RegEnumSubKeyEx():%s\r\n"),szSubEnum));
            if (RegOpenKeyEx(hkEnum, szSubEnum, 0, KEY_READ,&hkSubEnum) == ERROR_SUCCESS){
                // Enumerate the subkeys of HKLM\Enum\*\, looking for keys
    			// named *PNP0500 and *PNP0501 (or anything in USBPORTS)
                bUsbDevice          = ( 0 == _tcscmp(szSubEnum,TEXT("USBPORTS")));
                dwSubSubEnumIndex   = 0;
                dwSize              = sizeof(szSubSubEnum); // set the buffer size
    			while (RegEnumKeyEx(hkSubEnum, dwSubSubEnumIndex++, szSubSubEnum,&dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("@@RegEnumSubSubKeyEx():%s\r\n"),szSubSubEnum));
                    bMatch = ( 0 == _tcscmp(szSubSubEnum,TEXT("*PNP0500")) || 0 == _tcscmp(szSubSubEnum,TEXT("*PNP0501"))||bUsbDevice);
    				if (bMatch) {
                        if (RegOpenKeyEx(hkSubEnum, szSubSubEnum, 0, KEY_READ,&hkSubSubEnum) == ERROR_SUCCESS){
        					SearchPnpKeyW9x(hkSubSubEnum, bUsbDevice, list);
        					RegCloseKey(hkSubSubEnum);
                        }
    				}
                    hkSubSubEnum = NULL;
    				dwSize = sizeof(szSubSubEnum);  // restore the buffer size
    			}
    			RegCloseKey(hkSubEnum);
            }
            hkSubEnum       = NULL;
            dwSize          = sizeof(szSubEnum); // restore the buffer size
		}
        RegCloseKey(hkEnum);
    } else {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("@@RegOpenKeyEx():FAILED\r\n")));
    }
    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("--EnumPortsW9x()\r\n")));
}
void EnumSerial::SearchPnpKeyW9x    (HKEY hkPnp, BOOL bUsbDevice,ListEntry *List)
{
    // Enumerate the subkeys of the given PNP key, looking for values with
    // the name "PORTNAME"
    // First, open HKLM\Enum
    SSerInfo    *pInfo          = NULL;
    ListEntry   *node           = NULL;
    TCHAR       szSubPnp[256];
    TCHAR       szValue[256];
    TCHAR       szPath[256];
    TCHAR       strPortName[256];
    TCHAR       strFriendlyName[256];
    BOOL        bDup;
    DWORD       dwSubPnpIndex   = 0;
    SSerInfo    *psi            = NULL;
    DWORD       dwSize          = sizeof(szSubPnp);
    HKEY        hkSubPnp        = NULL;
    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("++SearchPnpKeyW9x()\r\n")));
    // Enumerate the subkeys of HKLM\Enum\*\PNP050[01]
	while (RegEnumKeyEx(hkPnp, dwSubPnpIndex++, szSubPnp, &dwSize,NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        if (RegOpenKeyEx(hkPnp, szSubPnp, 0, KEY_READ,&hkSubPnp) == ERROR_SUCCESS){
			// Look for the PORTNAME value
			dwSize = sizeof(szValue);
			if (RegQueryValueEx(hkSubPnp, TEXT("PORTNAME"), NULL, NULL, (BYTE*)szValue, &dwSize) == ERROR_SUCCESS){
                wsprintf(strPortName,szValue);
				// Got the portname value. Look for a friendly name.
                dwSize = sizeof(szValue);
                if (RegQueryValueEx(hkSubPnp, TEXT("FRIENDLYNAME"), NULL, NULL, (BYTE*)szValue,&dwSize) == ERROR_SUCCESS)
                    wsprintf(strFriendlyName,szValue);
                //strFriendlyName = szValue;
				//Prepare an entry for the output array.
                psi         = new SSerInfo();
                wsprintf(szPath,TEXT("\\\\.\\%s"),strPortName);
                wsprintf(psi->strDevPath,szPath);
                wsprintf(psi->strPortName,strPortName);
                wsprintf(psi->strFriendlyName,strFriendlyName);
				psi->bUsbDevice = bUsbDevice;
				// Overwrite duplicates.
                bDup        = FALSE;
                for(node = List->Flink;node != List ;){
                    pInfo   = CONTAINING_RECORD(node, SSerInfo, Link);
                    node    = node->Flink;
                    if(0 == _tcscmp(pInfo->strPortName,psi->strPortName) ){
						bDup = TRUE;
                    }
                }
				if (bDup) {
                    delete psi;
                } else {
				    // Add an entry to the array
                    InsertTailList(List,&psi->Link);
                }
                psi = NULL;
			}
			RegCloseKey(hkSubPnp);
        }
		hkSubPnp = NULL;
		dwSize = sizeof(szSubPnp);  // restore the buffer size
    }
    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("--SearchPnpKeyW9x()\r\n")));

}

