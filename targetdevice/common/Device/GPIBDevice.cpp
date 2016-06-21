#include "StdAfx.h"
#include "GPIBDevice.h"

typedef void (*MYPROC)(); 
MYPROC ProcAdd;

CString GetGPIBError(int nErr)
{
	switch (nErr)
	{
	case EDVR:
		return _T("System error");
	case ECIC:
		return _T("Function requires GPIB interface to be CIC");
	case ENOL:
		return _T("No Listeners on the GPIB");
	case EADR:
		return _T("GPIB interface not addressed correctly");
	case EARG:
		return _T("Invalid argument to function call");
	case ESAC:
		return _T("GPIB interface not System Controller as required");
	case EABO:
		return _T("I / O operation aborted(timeout)");
	case ENEB:
		return _T("Nonexistent GPIB interface");
	case EDMA:
		return _T("DMA error");
	case EOIP:
		return _T("Asynchronous I / O in progress");
	case ECAP:
		return _T("No capability for operation");
	case EFSO:
		return _T("File system error");
	case EBUS:
		return _T("GPIB bus error");
	case ESRQ:
		return _T("SRQ stuck in ON position");
	case ETAB:
		return _T("Table problem");
	case ELCK:
		return _T("Interface is locked");
	case EARM:
		return _T("ibnotify callback failed to rearm");
	case EHDL:
		return _T("Input handle is invalid");
	case EWIP:
		return _T("Wait in progress on specified input handle");
	case ERST:
		return _T("The event notification was cancelled due to a reset of the interface");
	case EPWR:
		return _T("The interface lost power");
	default:
		return _T("Unknown error");
	};
}

CGPIBDevice::CGPIBDevice(CLogger *p)
{
	SetLogger(p);
	m_strLogHeader = _T(" [GPIB] ");

	m_nDev = -1;

	m_hGPIBDLL = NULL;

	m_Pibsta = NULL;
	m_Piberr = NULL;
	m_Pibcntl = NULL;

	ibdev = NULL;
	ibln = NULL;
	ibwrt = NULL;
	ibrd = NULL;
	ibrsp = NULL;
	ibonl = NULL;
	ibeos = NULL;
	ibeot = NULL;
	ibtmo = NULL;
	ibppc = NULL;
	ibpad = NULL;
	ibsad = NULL;
	ibrpp = NULL;
	ibwait = NULL;
	ibloc = NULL;
	ibpct = NULL;
	ibtrg = NULL;
	ibclr = NULL;
	ibconfig = NULL;

	ibask = NULL;
	ibcac = NULL;
	FindLstn = NULL;
	ibfind = NULL;
	ibsic = NULL;

	Log(_T("GPIB driver created"));
}

CGPIBDevice::~CGPIBDevice(void)
{
	Close();

	if(m_hGPIBDLL) FreeLibrary(m_hGPIBDLL);

	Log(_T("GPIB driver exit"));
}

bool CGPIBDevice::Open(int nBUS, int nAddr, int nSecAddr)
{
	if(m_hGPIBDLL == NULL && !LoadGPIBDriver())
	{
		return false;
	}

	m_nDev = ibdev(nBUS, nAddr, nSecAddr, 13, 1, 0);
	if (m_nDev < 0 || (ibsta & ERR))
	{
		Log(CCommon::FormatString(_T("Failed to open GPIB device at GPIB::%d::%d::%d!"), nBUS, nAddr, nSecAddr));
		return false;
	}

	short n = 0;
	ibln(m_nDev, nAddr, 0, &n);
	if (n <= 0 || (ibsta & ERR))
	{
		Log(_T("No GPIB listener!"));
		return false;
	}
	else
	{
		Log(_T("GPIB device opened!"));
		ibclr(m_nDev);
		return true;
	}
}

bool CGPIBDevice::Close()
{
	if(m_nDev >= 0)
	{
		ibloc(m_nDev);
		ibonl(m_nDev, 0);
		m_nDev = -1;
	}

	Log(_T("GPIB device closed!"));

	return true;
}

bool CGPIBDevice::Read(CString &strRead, int &nLen, int nTimeout)
{
	char *pRead = new char[nLen + 1];
	memset(pRead, 0, sizeof(nLen + 1));

	if(!Read(pRead, nLen, nTimeout))
	{
		return false;
	}

	CCommon::Copy2WideChar(strRead, pRead, strlen(pRead));

	delete[] pRead;

	return true;
}

bool CGPIBDevice::Write(CString strWrite)
{
	int nLen = strWrite.GetLength();
	if(nLen == 0) return true;

	char *pOut = new char[nLen+1];
	memset(pOut, 0, sizeof(nLen + 1));

	CCommon::Copy2MultiByte(strWrite, pOut, nLen + 1);

	bool r = Write(pOut);

	delete[] pOut;
	return r;
}

bool CGPIBDevice::Query(CString strWrite, int &nLen, CString &strRead, int nTimeout)
{
	return Write(strWrite) && Read(strRead, nLen, nTimeout);
}

bool CGPIBDevice::Read(char *strRead, int &nLen, int nTimeout)
{
	if (strRead == NULL || nLen <= 0)
	{
		Log(_T("Invalid GPIB read input parameter"));
		return false;
	}

	memset(strRead, 0, nLen);

	ibrd(m_nDev, strRead, nLen - 1);

	if (ibsta & ERR)
	{
		Log(_T("GPIB read error: ") + GetGPIBError(iberr));
		return false;
	}

	Log(m_strLogHeader + _T("[PC<-TS] ") + CCommon::ToWideChar(strRead));
	return true;
}

bool CGPIBDevice::Write(const char *strWrite)
{
	if (strWrite == NULL)
	{
		Log(_T("Invalid GPIB write input parameter"));
		return false;
	}

	if (strlen(strWrite) == 0) return true;

	ibwrt(m_nDev, (char*)strWrite, strlen(strWrite));

	Log(m_strLogHeader + _T(" [PC->TS] ") + CCommon::ToWideChar(strWrite));

	if (ibsta & ERR)
	{
		Log(_T("GPIB write error: ") + GetGPIBError(iberr));
		return false;
	}

	return true;
}

bool CGPIBDevice::Query(const char *sWrite, int &nLen, char *sRead, int nTimeout)
{
	return Write(sWrite) && Read(sRead, nLen, nTimeout);
}

bool CGPIBDevice::Read(byte *strRead, int &nLen, int nTimeout)
{
	return false;
}

bool CGPIBDevice::Write(const byte *strWrite)
{
	return false;
}

bool CGPIBDevice::Query(const byte *strWrite, int &nLen, byte *sRead, int nTimeout)
{
	return false;
}

bool CGPIBDevice::LoadGPIBDriver()
{
	CString strGPIBVersion;
	if (!CCommon::GetGPIBVersion(strGPIBVersion))
	{
		Log(_T("Failed to get GPIB version infomation, so GPIB driver is not available!"));
		return false;
	}

	Log(strGPIBVersion);

	CString dllname,dllpathname;

	dllpathname = _T("gpib-32.DLL");
	m_hGPIBDLL = LoadLibrary(dllpathname);//加载system32里系统默认的DLL

	if(m_hGPIBDLL == NULL)
	{
		Log(_T("Failed to load gpib-32.dll!"));
		Log(_T("Error message:"));
		Log(CCommon::GetSystemErrorMessage());

		return false;
	}

	ibdev =(ibdev_fun)GetProcAddress(m_hGPIBDLL,"ibdev");
	ibwrt=(ibwrt_fun)GetProcAddress(m_hGPIBDLL,"ibwrt");
	ibrd =(ibrd_fun)GetProcAddress(m_hGPIBDLL,"ibrd");
	ibonl=(ibonl_fun)GetProcAddress(m_hGPIBDLL,"ibonl");	
	ibclr=(ibclr_fun)GetProcAddress(m_hGPIBDLL,"ibclr");
	ibconfig=(ibconfig_fun)GetProcAddress(m_hGPIBDLL,"ibconfig");
	ibln=(ibln_fun)GetProcAddress(m_hGPIBDLL,"ibln"); 
	ibrsp=(ibrsp_fun)GetProcAddress(m_hGPIBDLL,"ibrsp");
	ibeos=(ibeos_fun)GetProcAddress(m_hGPIBDLL,"ibeos"); 
	ibeot=(ibeot_fun)GetProcAddress(m_hGPIBDLL,"ibeot"); 
	ibtmo=(ibtmo_fun)GetProcAddress(m_hGPIBDLL,"ibtmo"); 
	ibppc=(ibppc_fun)GetProcAddress(m_hGPIBDLL,"ibppc");
	ibpad=(ibpad_fun)GetProcAddress(m_hGPIBDLL,"ibpad");
	ibsad=(ibsad_fun)GetProcAddress(m_hGPIBDLL,"ibsad");
	ibrpp=(ibrpp_fun)GetProcAddress(m_hGPIBDLL,"ibrpp"); 
	ibwait=(ibwait_fun)GetProcAddress(m_hGPIBDLL,"ibwait");
	ibloc=(ibloc_fun)GetProcAddress(m_hGPIBDLL,"ibloc"); 
	ibpct=(ibpct_fun)GetProcAddress(m_hGPIBDLL,"ibpct"); 
	ibtrg=(ibtrg_fun)GetProcAddress(m_hGPIBDLL,"ibtrg"); 

	ibask = (ibask_fun)GetProcAddress(m_hGPIBDLL, "ibask");
	ibcac = (ibcac_fun)GetProcAddress(m_hGPIBDLL, "ibcac");
	FindLstn = (FindLstn_fun)GetProcAddress(m_hGPIBDLL, "FindLstn");
	ibfind = (ibfind_fun)GetProcAddress(m_hGPIBDLL, "ibfindA");
	ibsic = (ibsic_fun)GetProcAddress(m_hGPIBDLL, "ibsic");

	ProcAdd   = (MYPROC) GetProcAddress(m_hGPIBDLL, (LPCSTR)"user_ibsta"); 
	m_Pibsta = (int*)ProcAdd ;

	ProcAdd   = (MYPROC) GetProcAddress(m_hGPIBDLL, (LPCSTR)"user_iberr"); 
	m_Piberr = (int*)ProcAdd ;

	ProcAdd   = (MYPROC) GetProcAddress(m_hGPIBDLL, (LPCSTR)"user_ibcntl"); 
	m_Pibcntl = (long*)ProcAdd ;

	if (
		(m_Pibsta == NULL)  ||	(m_Piberr == NULL)  ||	(m_Pibcntl == NULL) ||	(ibtrg == NULL )	||
		(ibpct == NULL )	||	(ibloc == NULL )	||	(ibwait == NULL )	||	(ibrpp == NULL )	||
		(ibsad == NULL )	||	(ibpad == NULL )	||	(ibppc == NULL )	||	(ibtmo == NULL )	||
		(ibeot == NULL )	||	(ibeos == NULL )	||	(ibrsp == NULL )	||	(ibln == NULL )		||
		(ibconfig == NULL)	||	(ibclr == NULL)		||	(ibdev == NULL)		||	(ibonl == NULL)		||
		(ibrd == NULL)		||	(ibwrt == NULL)		||	(ibask == NULL)		||	(ibcac == NULL)		||
		(FindLstn == NULL)	||	(ibfind == NULL)	||	(ibsic == NULL)
		)
	{
		FreeLibrary (m_hGPIBDLL);
		m_hGPIBDLL = NULL;

		Log(_T("Failed to get function entry point from gpib-32.dll!"));

		return false;
	}
	else
	{
		Log(_T("Init GPIB driver OK!"));
		return true;
	}
}

bool CGPIBDevice::ScanDevice(CArray<IO_ADDRESS> &addresses)
{
	IO_ADDRESS addrnode;
	addresses.RemoveAll();
	int res;

	if (m_hGPIBDLL == NULL && !LoadGPIBDriver())
	{
		return false;
	}

	short primary_addrs[MAX_GPIB_PRIM_ADDR];
	short addr[MAX_ADDRESSES];

	for (int nAddr = 0; nAddr < MAX_ADDRESSES; nAddr++)
	{
		addr[nAddr] = (short)0xFFFF;
	}

	for (int nAddr = 0; nAddr < (MAX_GPIB_PRIM_ADDR - 2); nAddr++)
	{
		primary_addrs[nAddr] = (short)(nAddr + 1);
	}

	primary_addrs[MAX_GPIB_PRIM_ADDR - 2] = (short)0xFFFF;

	Log(_T("GPIB device scanning start!"));

	for (int nBus = 0; nBus < MAX_GPIB_CARD; nBus++)
	{
		ibonl(nBus, 1); if (ibsta & ERR) continue;
		ibconfig(nBus, IbcSC, 1); if (ibsta & ERR) continue;
		ibsic(nBus); if (ibsta & ERR) continue;
		ibconfig(nBus, IbcSRE, 1); if (ibsta & ERR) continue;
		ibconfig(nBus, IbcTIMING, 1); if (ibsta & ERR) continue;
		ibask(nBus, IbaPAD, &res); if (ibsta & ERR) continue;
		ibask(nBus, IbaSAD, &res); if (ibsta & ERR) continue;

		FindLstn(nBus, primary_addrs, addr, MAX_ADDRESSES - 1);

		if (ibsta & ERR) continue;

		for (int i = 0; i < (MAX_ADDRESSES - 1); i++)
		{
			if (addr[i] == -1) break;

			memset(&addrnode, 0, sizeof(IO_ADDRESS));

			addrnode.nBus = nBus;
			addrnode.eType = DRV_GPIB;
			addrnode.nPrimAddr = (addr[i] & 0xFF);
			addrnode.nSecAddr = ((addr[i] & 0xFF00) >> 8);

			addrnode.bPrimary = (i == 0 || addresses[i - 1].nPrimAddr != addrnode.nPrimAddr);

			if (addrnode.bPrimary && !Open(nBus, addrnode.nPrimAddr, addrnode.nSecAddr)) continue;

			if (!addrnode.bPrimary)
				strcpy_s(addrnode.sDevIdentity, addresses[i - 1].sDevIdentity);
			else
			{
				int nReadLen = MAX_DEV_IDN_LEN;
				CString strIDN;

				if (!Query("*IDN?", nReadLen, addrnode.sDevIdentity, 1000))
				{
					Log(CCommon::FormatString(_T("Failed to query identity of  device at GPIB::%d::%d::%d"), nBus, addrnode.nPrimAddr, addrnode.nSecAddr));

					Close();
					continue;
				}

				Close();
			}

			addresses.Add(addrnode);
		}
	}

	if (addresses.GetCount() == 0)
	{
		Log(_T("Found no GPIB device!"));
		return false;
	}

	return true;
}