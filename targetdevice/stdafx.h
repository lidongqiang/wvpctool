#ifndef _STDAFX_H_
#define _STDAFX_H_
#include <windows.h>
#include <Windowsx.h>
#include "TCHAR.h"
#include <commctrl.h>
#include <string>
#include "commdlg.h"
#ifndef _WIN32_WCE
#pragma comment(lib, "ComCtl32.lib")
#endif
#ifdef UNICODE
    #ifndef TSTRING
    #define TSTRING std::wstring
    #endif
#else
    #ifndef TSTRING
    #define TSTRING std::string
    #endif
#endif //#ifdef UNICODE
#endif