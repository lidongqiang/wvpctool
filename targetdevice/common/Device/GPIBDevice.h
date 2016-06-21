#pragma once

#include "baseiodevice.h"
#include "ni488.h"

#define MAX_GPIB_CARD 5
#define MAX_GPIB_PRIM_ADDR 32
#define MAX_ADDRESSES 65

class CGPIBDevice : public CBaseIODevice
{
public:

	CGPIBDevice(CLogger *p = NULL);
	virtual ~CGPIBDevice(void);

	virtual bool Open(LPCTSTR strDevice){ return false; };
	virtual bool Open(int, int, int nSecAddr = 0);

	virtual bool Close();

	virtual bool Read(CString &strRead, int &nLen, int nTimeout);
	virtual bool Write(CString strWrite);
	virtual bool Query(CString strWrite, int &nLen, CString &strRead, int nTimeout);

	virtual bool Read(char *strRead, int &nLen, int nTimeout);
	virtual bool Write(const char *strWrite);
	virtual bool Query(const char *strWrite, int &nLen, char *sRead, int nTimeout);

	virtual bool Read(byte *strRead, int &nLen, int nTimeout);
	virtual bool Write(const byte *strWrite);
	virtual bool Query(const byte *strWrite, int &nLen, byte *sRead, int nTimeout);

	virtual bool ScanDevice(CArray<IO_ADDRESS> &addresses);

protected:

	bool LoadGPIBDriver();

	int	m_nDev;

	HMODULE m_hGPIBDLL;

	int  *m_Pibsta;
	int  *m_Piberr;
	long *m_Pibcntl;

	ibdev_fun ibdev;
	ibln_fun  ibln;
	ibwrt_fun ibwrt;
	ibrd_fun ibrd;
	ibrsp_fun ibrsp;
	ibonl_fun ibonl;
	ibeos_fun ibeos;
	ibeot_fun ibeot;
	ibtmo_fun ibtmo;
	ibppc_fun ibppc;
	ibpad_fun ibpad;
	ibsad_fun ibsad;
	ibrpp_fun ibrpp;
	ibwait_fun ibwait;
	ibloc_fun ibloc;
	ibpct_fun ibpct;
	ibtrg_fun ibtrg;
	ibclr_fun ibclr; 
	ibconfig_fun ibconfig;

	ibask_fun ibask;
	ibcac_fun ibcac;
	FindLstn_fun FindLstn;
	ibfind_fun ibfind;
	ibsic_fun ibsic;

	CString m_strLogHeader;
};