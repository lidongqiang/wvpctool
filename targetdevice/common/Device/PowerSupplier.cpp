#include "StdAfx.h"
#include "PowerSupplier.h"

CPowerSupplier::CPowerSupplier(void)
{
	m_bKeithley = true;
	m_bDualport = true;
	m_bDVMAvailable = false;
}

CPowerSupplier::~CPowerSupplier(void)
{
	m_GPIBDevice.Close();
}

bool CPowerSupplier::Open(int nGPIBBoard,int nAddr, bool bReset)
{
	if(!m_GPIBDevice.Open(nGPIBBoard,nAddr))
	{
		return false;
	}

	char sRet[200];
	memset(sRet,0,sizeof(sRet));
	if(!m_GPIBDevice.Query("*IDN?",sRet,sizeof(sRet)))
	{
		AfxMessageBox(_T("Failed to init power supplier!"));
		return false;
	}

	CString strPSName(sRet);
	strPSName.MakeLower();
	if(strPSName.Find(_T("keithley")) >= 0)
	{
		m_bKeithley = true;
		m_bDVMAvailable = true;
		m_bDualport = (strPSName.Find(_T("2306")) >= 0) || (strPSName.Find(_T("2308")) >= 0);
	}
	else if(strPSName.Find(_T("agilent")) >= 0 || strPSName.Find(_T("hewlett")) >= 0)
	{
		m_bKeithley = false;
		m_bDualport = (strPSName.Find(_T("66309")) >= 0) || (strPSName.Find(_T("66311")) >= 0) || (strPSName.Find(_T("66319")) >= 0);
		m_bDVMAvailable = m_bDualport;
	}
	else
	{
		AfxMessageBox(_T("Unsupported power supplier!"));
		return false;
	}

	if(bReset)
	{
		char sRet[50];
		m_GPIBDevice.Query("*RST;*OPC?",sRet,sizeof(sRet));
	}

	if(m_bKeithley)
	{
		m_GPIBDevice.Write("SYST:AZER:STAT 0");
		m_GPIBDevice.Write("SENS:NPLC 0.1");
		m_GPIBDevice.Write("SENS:AVER 2");
	}
	else if(m_bDualport)
	{
		m_GPIBDevice.Write("SENS:SWE:TINT 0.001667");
		m_GPIBDevice.Write("SENS:SWE:POIN 2000");
	}

	return true;
}

void CPowerSupplier::Close()
{
	m_GPIBDevice.Close();
	return;
}

bool CPowerSupplier::SetOutputVolt(double fVolt, bool bPort2)
{
	char sCMD[50];

	if (bPort2 && !m_bDualport) return true;

	if(m_bKeithley)
		sprintf_s(sCMD, "SOUR%s:VOLT %4.2f", (bPort2 && m_bDualport) ? _T("2") : _T(""), fVolt);
	else
		sprintf_s(sCMD, "VOLT%s %4.2f", (bPort2 && m_bDualport) ? _T("2") : _T(""), fVolt);

	return m_GPIBDevice.Write(sCMD);
}

bool CPowerSupplier::SetCurrentLimit(double fCurr, bool bPort2)
{
	char sCMD[50];

	if (bPort2 && !m_bDualport) return true;

	if(m_bKeithley)
		sprintf_s(sCMD, "SOUR%s:CURR %4.2f", (bPort2 && m_bDualport) ? _T("2") : _T(""), fCurr);
	else
		sprintf_s(sCMD, "CURR%s %4.2f", (bPort2 && m_bDualport) ? _T("2") : _T(""), fCurr);

	return m_GPIBDevice.Write(sCMD);
}

bool CPowerSupplier::MeasureVolt(double &fVolt, bool bPort2)
{
	char sRet[100],sCMD[50];

	if (bPort2 && !m_bDualport) return false;

	if(m_bKeithley)
		sprintf_s(sCMD, "MEAS%s:VOLT?", (bPort2 && m_bDualport) ? _T("2") : _T(""));
	else
		sprintf_s(sCMD, "MEAS:VOLT%s?", (bPort2 && m_bDualport) ? _T("2") : _T(""));

	if(!m_GPIBDevice.Query(sCMD,sRet,100))
	{
		return false;
	}

	fVolt = atof(sRet);
	return true;
}

bool CPowerSupplier::MeasureCurr(double &fCurr, bool bPort2)
{
	char sRet[100],sCMD[50];

	if (bPort2 && !m_bDualport) return true;

	if(m_bKeithley)
		sprintf_s(sCMD, "MEAS%s:CURR?", (bPort2 && m_bDualport) ? _T("2") : _T(""));
	else
		sprintf_s(sCMD, "MEAS:CURR%s?", (bPort2 && m_bDualport) ? _T("2") : _T(""));

	memset(sRet,0,sizeof(sRet));
	if(!m_GPIBDevice.Query(sCMD,sRet,100))
	{
		return false;
	}

	fCurr = atof(sRet);
	return true;
}

bool CPowerSupplier::MeasureDVM(double &fDVM)
{
	char sRet[100],sCMD[50];

	if(m_bKeithley)
		sprintf_s(sCMD,"MEAS%s:DVM?");
	else
		sprintf_s(sCMD,"MEAS:DVM:ACDC?");

	if(!m_GPIBDevice.Query(sCMD,sRet,100))
	{
		return false;
	}

	fDVM = atof(sRet);
	return true;
}

bool CPowerSupplier::PowerOn(bool bPort1, bool bPort2)
{
	char sCMD[50];

	if (!bPort1 && !bPort2) return true;

	bool bRes = true;

	if (m_bKeithley)
	{
		if (bPort1)
		{
			sprintf_s(sCMD, "OUTP ON");
			bRes &= m_GPIBDevice.Write(sCMD);
		}

		if (bPort2 && m_bDualport)
		{
			sprintf_s(sCMD, "OUTP2 ON");
			bRes &= m_GPIBDevice.Write(sCMD);
		}
	}
	else if (bPort1)
	{
		sprintf_s(sCMD, "OUTP ON");
		bRes &= m_GPIBDevice.Write(sCMD);
	}

	return bRes;
}

bool CPowerSupplier::PowerOff(bool bPort1, bool bPort2)
{
	char sCMD[50];

	if (!bPort1 && !bPort2) return true;

	bool bRes = true;

	if (m_bKeithley)
	{
		if (bPort1)
		{
			sprintf_s(sCMD, "OUTP OFF");
			bRes &= m_GPIBDevice.Write(sCMD);
		}

		if (bPort2 && m_bDualport)
		{
			sprintf_s(sCMD, "OUTP2 OFF");
			bRes&= m_GPIBDevice.Write(sCMD);
		}
	}
	else
	{
		sprintf_s(sCMD, "OUTP OFF");
		bRes &= m_GPIBDevice.Write(sCMD);
	}

	return bRes;
}

bool CPowerSupplier::DVMAvailable()
{
	return m_bDVMAvailable;
}

bool CPowerSupplier::DualPort()
{
	return m_bDualport;
}