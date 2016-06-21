
// WvProjectBill.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "./settings/SettingBase.h"
#include "tinyxml.h"
#include "debug.h"
// CWvProjectBillApp:
// See WvProjectBill.cpp for the implementation of this class
//

class CWvProjectBillApp : public CWinAppEx
{
public:
	CWvProjectBillApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWvProjectBillApp theApp;