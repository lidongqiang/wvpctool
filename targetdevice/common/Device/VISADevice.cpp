#include "StdAfx.h"
#include "VISADevice.h"
#include <string>

void trim(std::string& input)
{
	std::string whitespace = " \r\n\t";
	int start_pos = 0;
	int end_pos = 0;
	for (unsigned int i = 0; i < input.size(); ++i)
	{
		if (whitespace.find(input[i]) != std::string::npos)
		{
			start_pos++;
		}
		else
		{
			break;
		}
	}
	for (int i = input.size() - 1; i >= 0; --i)
	{
		if (whitespace.find(input[i]) != std::string::npos)
		{
			end_pos++;
		}
		else
		{
			break;
		}
	}

	input = input.substr(start_pos, input.size() - start_pos - end_pos);
}

CVISADevice::CVISADevice(CLogger *p)
{
	m_hVISADLL = NULL;

	viOpen = NULL;
	viOpenDefaultRM = NULL;
	viSetAttribute = NULL;
	viGetAttribute = NULL;
	viClear = NULL;
	viWrite = NULL;
	viRead = NULL;
	viClose = NULL;
	viFindRsrc = NULL;
	viFindNext = NULL;
	viSetBuf = NULL;

	m_VisaRM = NULL;
	m_VisaInst = NULL;

	SetLogger(p);

	m_strLogHeader = _T("[VISA] ");

	Log(_T("VISA driver created"));
}

CVISADevice::~CVISADevice(void)
{
	Close();

	if (m_hVISADLL) FreeLibrary(m_hVISADLL);

	Log(_T("VISA driver exit"));
}

bool CVISADevice::Open(LPCTSTR strDevice)
{
	if (_tcslen(strDevice) == 0)
	{
		Log(CCommon::FormatString(_T("No VISA resource name")));
		return false;
	}

	if (m_hVISADLL == NULL && !LoadVisa32DLL())
	{
		return false;
	}

	Close();

	ViStatus status = viOpenDefaultRM(&m_VisaRM);
	if (status < VI_SUCCESS)
	{
		Log(CCommon::FormatString(_T("Open VISA resource manager fail, error code:%d"), status));
		return false;
	}

	int nBuffLen = _tcslen(strDevice) + 1;
	char *p = new char[nBuffLen];
	memset(p, 0, nBuffLen);

	CCommon::Copy2MultiByte(strDevice, p, nBuffLen);

	status = viOpen(m_VisaRM, p, VI_NULL, VI_NULL, &m_VisaInst);

	delete[] p; p = NULL;

	if (status < VI_SUCCESS)
	{
		Log(CCommon::FormatString(_T("Failed to open VISA device <%s>, error code:%d"), strDevice, status));
		return false;
	}

	Log(CCommon::FormatString(_T("VISA device <%s> opened"), strDevice));
	return true;
}

bool CVISADevice::Close()
{
	if (m_VisaInst != NULL) viClose(m_VisaInst);
	if (m_VisaRM != NULL) viClose(m_VisaRM);

	m_VisaInst = NULL;
	m_VisaRM = NULL;

	Log(_T("VISA device closed!"));

	return true;
}

bool CVISADevice::Read(CString &strRead, int &nLen, int nTimeout)
{
	if (nLen <= 0) return false;

	strRead.Empty();

	memset(m_Buffer, 0, sizeof(m_Buffer));

	bool bRes = Read(m_Buffer, nLen, nTimeout);
	if (bRes && nLen > 0) CCommon::Copy2WideChar(strRead, m_Buffer, nLen);

	return bRes;
}

bool CVISADevice::Write(CString strWrite)
{
	strWrite.Trim();
	if (strWrite.GetLength() == 0) return false;

	CCommon::Copy2MultiByte(strWrite, m_Buffer, strWrite.GetLength() + 1);

	return Write(m_Buffer);
}

bool CVISADevice::Query(CString strWrite, int &nLen, CString &strRead, int nTimeout)
{
	if (!Write(strWrite)) return false;
	return Read(strRead, nLen, nTimeout);
}

bool CVISADevice::Read(char *strRead, int &nLen, int nTimeout)
{
	if (strRead == NULL || nLen <= 0)
	{
		Log(_T("Invalid VISA read input parameter"));
		return false;
	}

	int nBufLen = nLen;
	ViUInt32 retCnt = 0;

	ViStatus status = viRead(m_VisaInst, (ViPBuf)strRead, nLen - 1, &retCnt);

	if (status < VI_SUCCESS)
	{
		viClear(m_VisaInst);
		Log(CCommon::FormatString(_T("VISA read fail, error code:%d"), status));
		return false;
	}
	/*
	if (retCnt <= 0)
	{
		Log(_T("VISA Read nothing!"));
		return false;
	}
	*/
	std::string s;
	s.assign(strRead);
	trim(s);

	strncpy_s(strRead, nBufLen, s.c_str(), s.length());
	Log(m_strLogHeader + _T(" [PC<-TS] ") + CCommon::ToWideChar(strRead));

	return true;
}

bool CVISADevice::Write(const char *strWrite)
{
	if (strWrite == NULL)
	{
		Log(_T("Invalid VISA write input parameter"));
		return false;
	}

	if (strlen(strWrite) == 0) return true;

	std::string s;
	s.assign(strWrite);
	s.append("\n");

	ViStatus status = viWrite(m_VisaInst, (ViByte*)s.c_str(), s.length(), VI_NULL);

	Log(m_strLogHeader + _T(" [PC->TS] ") + CCommon::ToWideChar(strWrite));

	if (status < VI_SUCCESS)
	{
		viClear(m_VisaInst);
		Log(CCommon::FormatString(_T("VISA write fail, error code:%d"), status));
		return false;
	}

	return true;
}

bool CVISADevice::Query(const char *strWrite, int &nLen, char *sRead, int nTimeout)
{
	if (!Write(strWrite)) return false;
	return Read(sRead, nLen, nTimeout);
}

bool CVISADevice::ScanDevice(CArray<IO_ADDRESS> &addresses)
{
	IO_ADDRESS addrnode;
	addresses.RemoveAll();

	if (m_hVISADLL == NULL && !LoadVisa32DLL())
	{
		return false;
	}

	Close();

	Log(_T("VISA device scanning start!"));

	ViStatus status = viOpenDefaultRM(&m_VisaRM);
	if (status < VI_SUCCESS)
	{
		Log(CCommon::FormatString(_T("Open VISA resource manager fail, error code:%d"), status));
		return false;
	}
	/*
	status = viSetAttribute(m_VisaRM, VI_ATTR_TMO_VALUE, 1000);
	if (status < VI_SUCCESS)
	{
		Log(CCommon::FormatString(_T("Failed to set VISA timeout, error code:%d"), status));
		return false;
	}
	*/
	std::string visaExp = "?*(TCPIP|USB|RSNRP|GPIB)?*(INSTR|SOCKET)";

	ViFindList fList;
	ViUInt32   numInstrs;
	ViChar     desc[VI_FIND_BUFLEN];

	status = viFindRsrc(m_VisaRM, (ViChar*)visaExp.c_str(), &fList, &numInstrs, desc);
	if (status < VI_SUCCESS)
	{
		Log(CCommon::FormatString(_T("Failed to find VISA resource, error code:%d"), status));
		return false;
	}

	int nCount = 0;
	while (numInstrs--)
	{
		if (!strstr(desc, "GPIB") && !strstr(desc, "TCPIP")) continue;

		Log(_T("Find device: ") + CCommon::ToWideChar(desc));

		status = viOpen(m_VisaRM, desc, VI_NULL, VI_NULL, &m_VisaInst);
		if (status < VI_SUCCESS)
		{
			Log(CCommon::FormatString(_T("Failed to open device: %s, error code:%d"), CCommon::ToWideChar(desc), status));

			viFindNext(fList, desc);
			continue;
		}

		memset(&addrnode, 0, sizeof(IO_ADDRESS));
		addrnode.eType = DRV_VISA;

		strcpy_s(addrnode.sConnString, desc);

		//addrnode.bPrimary = (nCount == 0 || addresses[nCount - 1].nPrimAddr != addrnode.nPrimAddr);

		//if (!addrnode.bPrimary)
		//	strcpy_s(addrnode.sDevIdentity, addresses[nCount - 1].sDevIdentity);
		//else
		{
			int nReadLen = MAX_DEV_IDN_LEN;
			if (!Query("*IDN?", nReadLen, addrnode.sDevIdentity, 1000))
			{
				Log(CCommon::FormatString(_T("Failed to query identity of  device: %s"), CCommon::ToWideChar(desc)));

				viClose(m_VisaInst);
				m_VisaInst = NULL;

				continue;
			}
		}

		addresses.Add(addrnode);
		nCount++;

		viClose(m_VisaInst);
		m_VisaInst = NULL;

		viFindNext(fList, desc);
	}

	viClose(m_VisaRM);
	m_VisaRM = NULL;
	m_VisaInst = NULL;

	return true;
}

bool CVISADevice::LoadVisa32DLL()
{
	CString strVISAVersion;
	if (!CCommon::GetVISAVersion(strVISAVersion))
	{
		Log(_T("Failed to get VISA version infomation, so VISA driver is not available!"));
		return false;
	}

	Log(strVISAVersion);

	m_hVISADLL = LoadLibrary(_T("visa32.dll"));//加载system32里系统默认的DLL

	if (m_hVISADLL == NULL)
	{
		Log(_T("Failed to load visa32.dll!"));
		Log(_T("Error message:"));
		Log(CCommon::GetSystemErrorMessage());

		return false;
	}

	viOpen = (ViStatus(__stdcall *)(ViSession sesn, ViRsrc name, ViAccessMode mode, ViUInt32 timeout, ViPSession vi))GetProcAddress(m_hVISADLL, "viOpen");
	viOpenDefaultRM = (ViStatus(__stdcall *)(ViPSession vi))GetProcAddress(m_hVISADLL, "viOpenDefaultRM");
	viSetAttribute = (ViStatus(__stdcall *)(ViObject vi, ViAttr attrName, ViAttrState attrValue))GetProcAddress(m_hVISADLL, "viSetAttribute");
	viGetAttribute = (ViStatus(__stdcall *)(ViObject vi, ViAttr attrName, void _VI_PTR attrValue))GetProcAddress(m_hVISADLL, "viGetAttribute");
	viClear = (ViStatus(__stdcall *)(ViSession vi))GetProcAddress(m_hVISADLL, "viClear");
	viWrite = (ViStatus(__stdcall *)(ViSession vi, ViBuf  buf, ViUInt32 cnt, ViPUInt32 retCnt))GetProcAddress(m_hVISADLL, "viWrite");
	viRead = (ViStatus(__stdcall *)(ViSession vi, ViPBuf buf, ViUInt32 cnt, ViPUInt32 retCnt))GetProcAddress(m_hVISADLL, "viRead");
	viClose = (ViStatus(__stdcall *)(ViObject vi))GetProcAddress(m_hVISADLL, "viClose");
	viFindRsrc = (ViStatus(__stdcall *)(ViSession sesn, ViString expr, ViPFindList vi, ViPUInt32 retCnt, ViChar _VI_FAR desc[]))GetProcAddress(m_hVISADLL, "viFindRsrc");
	viFindNext = (ViStatus(__stdcall *)(ViFindList vi, ViChar _VI_FAR desc[]))GetProcAddress(m_hVISADLL, "viFindNext");
	viSetBuf = (ViStatus(__stdcall *)(ViSession vi, ViUInt16 mask, ViUInt32 size))GetProcAddress(m_hVISADLL, "viSetBuf");

	if (
		(viOpen == NULL) || (viOpenDefaultRM == NULL) || (viSetAttribute == NULL) || (viGetAttribute == NULL) ||
		(viClear == NULL) || (viWrite == NULL) || (viRead == NULL) || (viClose == NULL) ||
		(viFindRsrc == NULL) || (viFindNext == NULL) || (viSetBuf == NULL)
		)
	{
		FreeLibrary(m_hVISADLL);
		m_hVISADLL = NULL;

		Log(_T("Failed to get function entry point from visa32.dll!"));
		return false;
	}
	else
	{
		Log(_T("Init VISA driver OK!"));
		return true;
	}
}