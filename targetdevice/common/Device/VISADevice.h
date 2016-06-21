#pragma once

#include "baseiodevice.h"
#include "visa.h"
#include "visatype.h"

class CVISADevice : public CBaseIODevice
{
public:
	CVISADevice(CLogger *p = NULL);
	~CVISADevice(void);

	virtual bool Open(LPCTSTR strDevice);
	virtual bool Open(int, int, int nSecAddr = 0){ return false; };

	virtual bool Close();

	virtual bool Read(CString &strRead, int &nLen, int nTimeout);
	virtual bool Write(CString strWrite);
	virtual bool Query(CString strWrite, int &nLen, CString &strRead, int nTimeout);

	virtual bool Read(char *strRead, int &nLen, int nTimeout);
	virtual bool Write(const char *strWrite);
	virtual bool Query(const char *strWrite, int &nLen, char *sRead, int nTimeout);

	virtual bool Read(byte *strRead, int &nLen, int nTimeout){ return false; };
	virtual bool Write(const byte *strWrite){ return false; };
	virtual bool Query(const byte *strWrite, int &nLen, byte *sRead, int nTimeout){ return false; };

	virtual bool ScanDevice(CArray<IO_ADDRESS> &addresses);

private:

	HMODULE m_hVISADLL;
	bool LoadVisa32DLL();

	ViStatus(__stdcall *viOpen)(ViSession sesn, ViRsrc name, ViAccessMode mode, ViUInt32 timeout, ViPSession vi);
	ViStatus(__stdcall *viOpenDefaultRM)(ViPSession vi);
	ViStatus(__stdcall *viSetAttribute)(ViObject vi, ViAttr attrName, ViAttrState attrValue);
	ViStatus(__stdcall *viGetAttribute)(ViObject vi, ViAttr attrName, void _VI_PTR attrValue);
	ViStatus(__stdcall *viClear)(ViSession vi);
	ViStatus(__stdcall *viWrite)(ViSession vi, ViBuf  buf, ViUInt32 cnt, ViPUInt32 retCnt);
	ViStatus(__stdcall *viRead)(ViSession vi, ViPBuf buf, ViUInt32 cnt, ViPUInt32 retCnt);
	ViStatus(__stdcall *viClose)(ViObject vi);
	ViStatus(__stdcall *viFindRsrc)(ViSession sesn, ViString expr, ViPFindList vi, ViPUInt32 retCnt, ViChar _VI_FAR desc[]);
	ViStatus(__stdcall *viFindNext)(ViFindList vi, ViChar _VI_FAR desc[]);
	ViStatus(__stdcall *viSetBuf)(ViSession vi, ViUInt16 mask, ViUInt32 size);

	char m_Buffer[8192];

	ViSession m_VisaRM;
	ViSession m_VisaInst;

	CString m_strLogHeader;

};

