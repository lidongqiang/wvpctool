
// wvpctool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "./settings/SettingBase.h"
#include "..\targetdevice\common\target.h"
#include ".\common\common.h"
#include "tinyxml.h"
// CwvpctoolApp:
// See wvpctool.cpp for the implementation of this class
//

#define WM_SAVE_CONFIG          WM_USER+1
#define WM_UPDATE_MSG           WM_USER+2
#define WM_DROPFILES_MSG        WM_USER+3
#define WM_ASKBUY               WM_USER+4
#define WM_UPDATE_FILELIST      WM_USER+5
class CwvpctoolApp : public CWinAppEx
{
public:
	CwvpctoolApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CwvpctoolApp theApp;
