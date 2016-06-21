#include "StdAfx.h"
#include "RFTester.h"

CRFTester::CRFTester(void)
{
	m_pGPIBDevice = NULL;
	m_pVISADevice = NULL;
	m_pIO = NULL;
	m_pLogger = NULL;
}

CRFTester::~CRFTester(void)
{
	Close();
}

void CRFTester::Log(LPCTSTR s)
{
	if (m_pLogger != NULL)
	{
		try
		{
			m_pLogger->Log(s);
		}
		catch (...)
		{

		}
	}
}

void CRFTester::SleepWithLog(int nMs)
{
	Sleep(nMs);

	Log(CCommon::FormatString(_T("Sleep for %d ms"), nMs));
}

bool CRFTester::Open(LPCTSTR strVISAName)
{
	if(m_pGPIBDevice != NULL)
	{
		m_pGPIBDevice->Close();
		delete m_pGPIBDevice;
		m_pGPIBDevice = NULL;
	}

	if (m_pVISADevice == NULL)
	{
		m_pVISADevice = new CVISADevice(m_pLogger);
		//m_pVISADevice->SetLogger(m_pLogger);
	}
	else
		m_pVISADevice->Close();

	if (!m_pVISADevice->Open(strVISAName))
	{
		delete m_pVISADevice;
		m_pVISADevice = NULL;
		m_pIO = NULL;

		return false;
	}

	m_pIO = m_pVISADevice;
	return ReadIdentity();
}

bool CRFTester::Open(int nBUS, int nAddr)
{
	if(m_pVISADevice != NULL)
	{
		m_pVISADevice->Close();
		delete m_pVISADevice;
		m_pVISADevice = NULL;
	}

	if (m_pGPIBDevice == NULL)
	{
		m_pGPIBDevice = new CGPIBDevice(m_pLogger);
		//m_pGPIBDevice->SetLogger(m_pLogger);
	}
	else
		m_pGPIBDevice->Close();

	if (!m_pGPIBDevice->Open(nBUS, nAddr))
	{
		delete m_pGPIBDevice;
		m_pGPIBDevice = NULL;
		m_pIO = NULL;

		return false;
	}

	m_pIO = m_pGPIBDevice;
	return ReadIdentity();
}

bool CRFTester::Close()
{
	if(m_pGPIBDevice != NULL) m_pGPIBDevice->Close();
	if(m_pVISADevice != NULL) m_pVISADevice->Close();

	m_pGPIBDevice = NULL;
	m_pVISADevice = NULL;

	m_pIO = NULL;

	return true;
}

void CRFTester::ClearMeasureStatus()
{
	m_GSMMeasStatus.Clear();
	m_WCDMAMeasStatus.Clear();
}

bool CRFTester::GetGSMPeakTxPower(double &fPow)
{
	if (!m_GSMMeasStatus.bPeakTxPower && !Measure(GSM_PEAK_TX_POW))
		return false;

	fPow = m_GSMMeasStatus.fPeakTxPower;
	return true;
}

bool CRFTester::GetGSMAvgTxPower(double &fPow)
{
	if (!m_GSMMeasStatus.bAvgTxPower && !Measure(GSM_AVG_TX_POW))
		return false;

	fPow = m_GSMMeasStatus.fAvgTxPower;
	return true;
}

bool CRFTester::GetEDGEPeakTxPower(double &fPow)
{
	if (!m_GSMMeasStatus.bEDGEPeakTxPower && !Measure(EDGE_PEAK_TX_POW))
		return false;

	fPow = m_GSMMeasStatus.fEDGEPeakTxPower;
	return true;
}

bool CRFTester::GetEDGEAvgTxPower(double &fPow)
{
	if (!m_GSMMeasStatus.bEDGEAvgTxPower && !Measure(EDGE_AVG_TX_POW))
		return false;

	fPow = m_GSMMeasStatus.fEDGEAvgTxPower;
	return true;
}

bool CRFTester::GetGSMPeakFreqErr(double &f)
{
	if (!m_GSMMeasStatus.bPeakFreqErr && !Measure(GSM_PEAK_FREQ_ERR))
		return false;

	f = m_GSMMeasStatus.fPeakFreqErr;
	return true;
}

bool CRFTester::GetGSMPeakPhaseErr(double &f)
{
	if (!m_GSMMeasStatus.bPeakPhaseErr && !Measure(GSM_PEAK_PHS_ERR))
		return false;

	f = m_GSMMeasStatus.fPeakPhaseErr;
	return true;
}

bool CRFTester::GetGSMRMSPhaseErr(double &f)
{
	if (!m_GSMMeasStatus.bRMSPhaseErr && !Measure(GSM_RMS_PHS_ERR))
		return false;

	f = m_GSMMeasStatus.fRMSPhaseErr;
	return true;
}

bool CRFTester::GetGSMPVT(bool &b)
{
	if (!m_GSMMeasStatus.bPVT && !Measure(GSM_PVT))
		return false;

	b = m_GSMMeasStatus.bPVTResult;
	return true;
}

bool CRFTester::GetGSMSwSpectrum(bool &b)
{
	if (!m_GSMMeasStatus.bSwSpec && !Measure(GSM_SW_SPEC))
		return false;

	b = m_GSMMeasStatus.bSwSpecRes;
	return true;
}

bool CRFTester::GetGSMModSpectrum(bool &b)
{
	if (!m_GSMMeasStatus.bModSpec && !Measure(GSM_MOD_SPEC))
		return false;

	b = m_GSMMeasStatus.bModSpecRes;
	return true;
}

bool CRFTester::GetGSMBER(double &f)
{
	if (!m_GSMMeasStatus.bBER && !Measure(GSM_BER))
		return false;

	f = m_GSMMeasStatus.fBER;
	return true;
}

bool CRFTester::GetGSMRxLevel(int &n)
{
	if (!m_GSMMeasStatus.bRxLevel && !Measure(GSM_RXLVL))
		return false;

	n = m_GSMMeasStatus.nRxLevel;
	return true;
}

bool CRFTester::GetGSMRxQuality(int &n)
{
	if (!m_GSMMeasStatus.bRxQuality && !Measure(GSM_RXQ))
		return false;

	n = m_GSMMeasStatus.nRxQuality;
	return true;
}

bool CRFTester::GetGSMBLER(double &f)
{
	if (!m_GSMMeasStatus.bBLER && !Measure(GSM_BLER))
		return false;

	f = m_GSMMeasStatus.fBLER;
	return true;
}

bool CRFTester::GetWMaxTxPower(double &f)
{
	if (!m_WCDMAMeasStatus.bMaxTxPower && !Measure(WCDMA_MAX_TX_POW))
		return false;

	f = m_WCDMAMeasStatus.fMaxTxPower;
	return true;
}

bool CRFTester::GetWMinTxPower(double &f)
{
	if (!m_WCDMAMeasStatus.bMinTxPower && !Measure(WCDMA_MIN_TX_POW))
		return false;

	f = m_WCDMAMeasStatus.fMinTxPower;
	return true;
}

bool CRFTester::GetWFreqErr(double &f)
{
	if (!m_WCDMAMeasStatus.bFreqErr && !Measure(WCDMA_FREQ_ERR))
		return false;

	f = m_WCDMAMeasStatus.fFreqErr;
	return true;
}
/*
bool CRFTester::GetWPeakPhaseErr(double &f)
{
	if (!m_WCDMAMeasStatus.bPeakPhaseErr && !Measure(WCDMA_PEAK_PHS_ERR))
		return false;

	f = m_WCDMAMeasStatus.fPeakPhaseErr;
	return true;
}
*/
bool CRFTester::GetWRMSPhaseErr(double &f)
{
	if (!m_WCDMAMeasStatus.bRMSPhaseErr && !Measure(WCDMA_RMS_PHS_ERR))
		return false;

	f = m_WCDMAMeasStatus.fRMSPhaseErr;
	return true;
}

bool CRFTester::GetWOBW(double &f)
{
	if (!m_WCDMAMeasStatus.bOBW && !Measure(WCDMA_OBW))
		return false;

	f = m_WCDMAMeasStatus.fOBW;
	return true;
}

bool CRFTester::GetWACLR(double fACLR[4])
{
	if (!m_WCDMAMeasStatus.bACLR && !Measure(WCDMA_ACLR))
		return false;

	memcpy(fACLR, m_WCDMAMeasStatus.fACLR, sizeof(double) * 4);
	return true;
}
/*
bool CRFTester::GetWPeakEVM(double &f)
{
	if (!m_WCDMAMeasStatus.bPeakEVM && !Measure(WCDMA_PEAK_EVM))
		return false;

	f = m_WCDMAMeasStatus.fPeakEVM;
	return true;
}
*/
bool CRFTester::GetWRMSEVM(double &f)
{
	if (!m_WCDMAMeasStatus.bRMSEVM && !Measure(WCDMA_RMS_EVM))
		return false;

	f = m_WCDMAMeasStatus.fRMSEVM;
	return true;
}

bool CRFTester::GetWRMSMagnitude(double &f)
{
	if (!m_WCDMAMeasStatus.bRMSMagnitude && !Measure(WCDMA_RMS_MAG))
		return false;

	f = m_WCDMAMeasStatus.fRMSMagnitude;
	return true;
}
/*
bool CRFTester::GetWPeakMagnitude(double &f)
{
	if (!m_WCDMAMeasStatus.bPeakMagnitude && !Measure(WCDMA_PEAK_MAG))
		return false;

	f = m_WCDMAMeasStatus.fPeakMagnitude;
	return true;
}
*/
bool CRFTester::GetWPCDE(double &f)
{
	if (!m_WCDMAMeasStatus.bPCDE && !Measure(WCDMA_PCDE))
		return false;

	f = m_WCDMAMeasStatus.fPCDE;
	return true;
}

bool CRFTester::GetWSEM(int nRange, double &fLevel, double &fFreqOffset, double &fMaskMargin, bool &bPass)
{
	if (nRange< 0 || nRange > 3) return false;
	if (!m_WCDMAMeasStatus.bSEM && !Measure(WCDMA_SEM)) return false;

	bPass = m_WCDMAMeasStatus.bSEMPass[nRange];
	fLevel = m_WCDMAMeasStatus.fSEM_Level[nRange];
	fFreqOffset = m_WCDMAMeasStatus.fSEM_FreqOffset[nRange];
	fMaskMargin = m_WCDMAMeasStatus.fSEM_MaskMargin[nRange];

	return true;
}

bool CRFTester::GetWSEM(bool &bTotalPass)
{
	if (!m_WCDMAMeasStatus.bSEM && !Measure(WCDMA_SEM)) return false;

	bTotalPass = m_WCDMAMeasStatus.bSEMPass[0];
	bTotalPass &= m_WCDMAMeasStatus.bSEMPass[1];
	bTotalPass &= m_WCDMAMeasStatus.bSEMPass[2];
	bTotalPass &= m_WCDMAMeasStatus.bSEMPass[3];

	return true;
}

bool CRFTester::GetWILPC_E(int &nCount, double fILPC[150])
{
	if (!m_WCDMAMeasStatus.bILPC_E && !Measure(WCDMA_ILPC_E))
		return false;

	nCount = m_WCDMAMeasStatus.nILPC_E_Count;
	memcpy(fILPC, m_WCDMAMeasStatus.fILPC_E, sizeof(double)* nCount);
	return true;
}

bool CRFTester::GetWILPC_F(int &nCount, double fILPC[150])
{
	if (!m_WCDMAMeasStatus.bILPC_F && !Measure(WCDMA_ILPC_F))
		return false;

	nCount = m_WCDMAMeasStatus.nILPC_F_Count;
	memcpy(fILPC, m_WCDMAMeasStatus.fILPC_F, sizeof(double)* nCount);
	return true;
}

bool CRFTester::GetWBER(double &f)
{
	if (!m_WCDMAMeasStatus.bBER && !Measure(WCDMA_BER))
		return false;

	f = m_WCDMAMeasStatus.fBER;
	return true;
}
