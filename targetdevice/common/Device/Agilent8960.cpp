#include "stdafx.h"
#include "Agilent8960.h"

extern float GSMChannel2FreqMHz(INTL_GSM_BAND eBand, int nChan, bool bUL);
extern float WCDMAChannel2FreqMHz(INTL_WCDMA_BAND eBand, int nChan, bool bUL);

CAgilent8960::CAgilent8960()
{
	m_strID = _T("Agilent Technologies,8960 Series 10 E5515C");

	m_Curr2GBand = INTL_2G_BAND_NUM;
	m_Curr3GBand = INTL_3G_BAND_NUM;
	m_nCurrChan = -1;
	m_fBSPower = -1.0f;

}

CAgilent8960::~CAgilent8960()
{
}

CString CAgilent8960::GetDeviceName()
{
	return _T("Agilent8960");
}

bool CAgilent8960::ReadIdentity()
{
	ASSERT_CONN

	int nReadLen = 1024;
	CString strIDN, strApp, strLic;
	if (!m_pIO->Query(_T("*IDN?"), nReadLen, strIDN, 1000)) return false;

	strIDN.Trim();
	if (strIDN.Find(m_strID) < 0) return false;

	nReadLen = 1024;
	if (!m_pIO->Query(_T("SYST:APPL?"), nReadLen, strApp, 1000)) return false;
	strApp.Trim();

	if (strApp.Find(_T("Fast Switch Test App")) >= 0)
		m_Application = APP_FAST_SWITCH;
	else if (strApp.Find(_T("GSM/GPRS Mobile Test")) >= 0)
		m_Application = APP_GSM;
	else if (strApp.Find(_T("WCDMA Mobile Test")) >= 0)
		m_Application = APP_WCDMA;
	else
		return false;

	nReadLen = 1024;
	if (!m_pIO->Query(_T("SYST:APPL:CAT:LIC:APPL:ALL?"), nReadLen, strLic, 1000)) return false;
	strLic.Trim();

	m_bEGPRSSupport = (strLic.Find(_T("E1968A-103")) >= 0);

	return true;
}

bool CAgilent8960::GetRFPorts(CArray<CString> &IOPorts, CArray<CString> &outPorts)
{
	return true;
}

bool CAgilent8960::SetRFPort(LPCTSTR strMainPort, LPCTSTR strDivPort)
{
	ASSERT_CONN

	return m_pIO->Write("RFG:OUTP IO");
}

bool CAgilent8960::SetCableLoss(CArray<double> &freqs, CArray<double> &losses)
{
	ASSERT_CONN

	if (freqs.GetCount() != losses.GetCount())
	{
		Log(_T("Wrong cable loss configuration!"));
		return false;
	}

	if (freqs.GetCount() == 0 && losses.GetCount() == 0)
	{
		return m_pIO->Write("SYST:CORR:GAIN");//Clear loss
	}

	CString strFreqs;
	strFreqs.Format(_T("SYST:CORR:FREQ %G MHz"), freqs[0]);

	CString strLosses;
	strLosses.Format(_T("SYST:CORR:GAIN %G"), -losses[0]);

	CString sTemp;
	for (int i = 1; i < freqs.GetCount(); i++)
	{
		sTemp.Format(_T(",%G MHz"), freqs[i]);
		strFreqs += sTemp;

		sTemp.Format(_T(",%G"), -losses[i]);
		strLosses += sTemp;
	}

	bool bRes = m_pIO->Write(strFreqs);
	bRes &= m_pIO->Write(strLosses);

	return bRes;
}

bool CAgilent8960::GeneralConfig()
{
	return true;
}

bool CAgilent8960::SetupGSMTest(INTL_GSM_BAND band, int nChannel, int nPCL, double fCellPow)
{
	ASSERT_CONN

	if (m_Application != APP_FAST_SWITCH && m_Application != APP_GSM)
	{
		Log(_T("Wrong application"));
		return false;
	}

	if (!FullReset()) return false;
	if (!SetRFPort(NULL, NULL)) return false;

	if (m_Application == APP_FAST_SWITCH && !FormatSwitch(APP_GSM)) return false;

	m_bGSMFormat = true;
	m_bEDGETest = false;
	m_Curr2GBand = band;
	m_nCurrChan = nChannel;

	if (!m_pIO->Write("CALL:OPER:MODE OFF")) return false;

	if (!m_pIO->Write("SET:TXP:COUN 5")) return false;
	if (!m_pIO->Write("SET:TXP:TIM 2")) return false;

	if (!m_pIO->Write("SET:FBER:COUN 1000")) return false;
	if (!m_pIO->Write("SET:FBER:TIM 5")) return false;

	if (!m_pIO->Write("SET:PVT:COUN 5")) return false;
	if (!m_pIO->Write("SET:PVT:TIM 2")) return false;

	if (!m_pIO->Write("SET:ORFS:MOD:COUN 5")) return false;
	if (!m_pIO->Write("SET:ORFS:MOD:FREQ:OFFS:ALL ON")) return false;
	if (!m_pIO->Write("SET:ORFS:SWIT:COUN 5")) return false;
	if (!m_pIO->Write("SET:ORFS:SWIT:FREQ:OFFS:ALL ON")) return false;
	if (!m_pIO->Write("SET:ORFS:TIM 5")) return false;

	if (!m_pIO->Write("SET:PFER:COUN 5")) return false;
	if (!m_pIO->Write("SET:PFER:TIM 2")) return false;

	if (!SetBand(band)) return false;
	if (!SetTCH(nChannel)) return false;
	if (!SetPCL(nPCL)) return false;
	if (!SetCellPower(fCellPow)) return false;

	if (!m_pIO->Write("CALL:OPER:MODE CALL")) return false;

	return true;
}

bool CAgilent8960::SetupEDGETest(INTL_GSM_BAND band, int nChannel, int nPCL, double fCellPow)
{
	ASSERT_CONN

	if (m_Application != APP_FAST_SWITCH && m_Application != APP_GSM)
	{
		Log(_T("Wrong application"));
		return false;
	}

	if (!m_bEGPRSSupport)
	{
		Log(_T("EDGE is not supported"));
		return false;
	}

	if (!FullReset()) return false;

	if (m_Application == APP_FAST_SWITCH && !FormatSwitch(APP_GSM)) return false;

	m_bGSMFormat = true;
	m_bEDGETest = true;
	m_Curr2GBand = band;
	m_nCurrChan = nChannel;

	if (!m_pIO->Write("CALL:OPER:MODE OFF")) return false;
	if (!m_pIO->Write("CALL:BCH:SCEL EGPRS")) return false;
	if (!m_pIO->Write("CALL:OPER:MODE CALL")) return false;

	if (!m_pIO->Write("CALL:FUNC:CONN:TYPE BLER")) return false;
	if (!m_pIO->Write("CALL:PDTCH:MCSC MCS9,MCS9")) return false;
	if (!m_pIO->Write("CALL:PDTCH:MSL:CONF D2U1")) return false;
	if (!m_pIO->Write("CALL:PDTCH:MSL:MEAS:BURS 1")) return false;

	if (!m_pIO->Write("SET:ETXP:COUN 5")) return false;
	if (!m_pIO->Write("SET:ETXP:TIM 2")) return false;

	if (!m_pIO->Write("SET:BLER:COUN 1000")) return false;
	if (!m_pIO->Write("SET:BLER:TIM 5")) return false;

	if (!m_pIO->Write("SET:PVT:COUN 5")) return false;
	if (!m_pIO->Write("SET:PVT:TIM 2")) return false;

	if (!m_pIO->Write("SET:ORFS:MOD:COUN 5")) return false;
	if (!m_pIO->Write("SET:ORFS:MOD:FREQ:OFFS:ALL ON")) return false;
	if (!m_pIO->Write("SET:ORFS:SWIT:COUN 5")) return false;
	if (!m_pIO->Write("SET:ORFS:SWIT:FREQ:OFFS:ALL ON")) return false;
	if (!m_pIO->Write("SET:ORFS:TIM 5")) return false;

	if (!m_pIO->Write("SET:PFER:COUN 5")) return false;
	if (!m_pIO->Write("SET:PFER:TIM 2")) return false;

	if (!SetPDTCHBand(band)) return false;
	if (!SetPDTCH(nChannel)) return false;
	if (!SetPCL(nPCL)) return false;
	if (!SetCellPower(fCellPow)) return false;

	return true;
}

bool CAgilent8960::SetupWCDMATest(INTL_WCDMA_BAND band, int nDLChannel, double fCellPow)
{
	ASSERT_CONN

	if (m_Application != APP_FAST_SWITCH && m_Application != APP_WCDMA)
	{
		Log(_T("Wrong application"));
		return false;
	}

	if (!FullReset()) return false;

	if (m_Application == APP_FAST_SWITCH && !FormatSwitch(APP_WCDMA)) return false;

	m_bGSMFormat = false;
	m_Curr3GBand = band;
	m_nCurrChan = nDLChannel;

	if (!m_pIO->Write("CALL:OPER:MODE OFF")) return false;
	if (!m_pIO->Write("CALL:PAG:SERV RBT")) return false;
	if (!m_pIO->Write("CALL:PAG:REP ON")) return false;
	if (!m_pIO->Write("CALL:SERV:RBT:RAB RMC12")) return false;

	if (!m_pIO->Write("CALL:CLPC:UPL:MODE UP")) return false;
	if (!m_pIO->Write("CALL:MS:POW:TARG 23 DBM")) return false;

	if (!m_pIO->Write("CALL:OPER:MODE CALL")) return false;

	if (!m_pIO->Write("CALL:UPL:CHAN:CONT:AUTO ON")) return false;
	if (!m_pIO->Write("CALL:SET:CHAN:UPL:CONT:AUTO ON")) return false;
	if (!m_pIO->Write("SET:CONT:OFF")) return false;

	if (!m_pIO->Write("SET:WCP:COUN 5")) return false;
	if (!m_pIO->Write("SET:WCP:TIM 2")) return false;

	if (!m_pIO->Write("SET:WOBW:COUN 5")) return false;
	if (!m_pIO->Write("SET:WOBW:TIM 2")) return false;

	if (!m_pIO->Write("SET:WSEM:COUN 5")) return false;
	if (!m_pIO->Write("SET:WSEM:TIM 2")) return false;

	if (!m_pIO->Write("SET:WACL:COUN 5")) return false;
	if (!m_pIO->Write("SET:WACL:TIM 2")) return false;

	if (!m_pIO->Write("SET:WWQ:COUN 5")) return false;
	if (!m_pIO->Write("SET:WWQ:TIM 2")) return false;

	if (!m_pIO->Write("SET:WBER:COUN 2000")) return false;
	if (!m_pIO->Write("SET:WBER:TIM 2")) return false;

	if (!m_pIO->Write("SET:WILP:TIM 3")) return false;
	if (!m_pIO->Write("SET:WILP:MAX:POW:THR:TEST:CONT:AUTO ON")) return false;
	if (!m_pIO->Write("SET:WILP:MAX:OUTP:POW:TEST:TOL 0.7")) return false;
	if (!m_pIO->Write("SET:WILP:Min:POW:THR:TEST:CONT:AUTO ON")) return false;
	if (!m_pIO->Write("SET:WILP:Min:OUTP:POW:TEST:TOL 0.7")) return false;

	if (!SetDLChannel(nDLChannel)) return false;
	if (!SetCellPower(fCellPow)) return false;

	return true;
}

bool CAgilent8960::SetupLTETest()
{
	Log(_T("LTE is not supported"));
	return false;
}

bool CAgilent8960::SetupTDSTest()
{
	Log(_T("LTE is not supported"));
	return false;
}

bool CAgilent8960::QueryOnCall(bool &bCall)
{
	ASSERT_CONN

	CString strRead;
	int nLen = 20;

	if (!m_pIO->Query(_T("CALL:STAT?"), nLen, strRead, 1000)) return false;
	bCall =(strRead.Find(_T("CONN")) >= 0);

	return true;
}

bool CAgilent8960::QuerySync(bool &bSync)
{
	ASSERT_CONN

	CString strRead;
	int nLen = 20;

	if (!m_pIO->Query(_T("CALL:MS:REP:IMEI?"), nLen, strRead, 1000)) return false;
	bSync = (strRead.GetLength() == 15);

	return true;
}

bool CAgilent8960::ArmCallConnection()
{
	return m_pIO->Write("CALL:CONN:ARM");
}

bool CAgilent8960::BaseCall()
{
	ASSERT_CONN

	return m_pIO->Write("CALL:ORIG");
}

bool CAgilent8960::Handover(INTL_GSM_BAND band, int nDestChan, int nPCL, double fBSPow)
{
	return m_bEDGETest ? Handover_EDGE(band, nDestChan, nPCL, fBSPow) : Handover_GMSK(band, nDestChan, nPCL, fBSPow);
}

bool CAgilent8960::Handover_GMSK(INTL_GSM_BAND band, int nDestChan, int nPCL, double fBSPow)
{
	ASSERT_CONN

	if (fBSPow != 0 && !SetCellPower(fBSPow)) return false;

	if (band == m_Curr2GBand)
	{
		if (nDestChan >= 0 && !SetTCH(nDestChan)) return false;
		if (nPCL >= 0 && !SetPCL(nPCL)) return false;
	}
	else
	{
		switch (band)
		{
		case INTL_2G850:
			if (!m_pIO->Write("CALL:SET:TCH:BAND GSM850")) return false;
			break;
		case INTL_2G900:
			if (!m_pIO->Write("CALL:SET:TCH:BAND EGSM")) return false;
			break;
		case INTL_2G1800:
			if (!m_pIO->Write("CALL:SET:TCH:BAND DCS")) return false;
			if (!m_pIO->Write("CALL:BIND DCS")) return false;
			break;
		case INTL_2G1900:
			if (!m_pIO->Write("CALL:SET:TCH:BAND PCS")) return false;
			if (!m_pIO->Write("CALL:BIND PCS")) return false;
			break;
		default:
			break;
		}

		if (!m_pIO->Write(CCommon::FormatString(_T("CALL:SET:TCH %d"), nDestChan))) return false;
		if (!m_pIO->Write(CCommon::FormatString(_T("CALL:SET:TCH:MS:TXL %d"), nPCL))) return false;

		char sRet[10];
		int nLen = 10;
		if (!m_pIO->Query("CALL:HAND;*OPC?", nLen, sRet, 3000)) return false;
	}

	bool bStatus = false;
	DWORD dwStartTime = GetTickCount();
	while ((GetTickCount() - dwStartTime) < 3000)
	{
		if (!QueryOnCall(bStatus)) return false;

		if (bStatus)
		{
			m_Curr2GBand = band;
			break;
		}
		else
			SleepWithLog(200);
	}

	return bStatus;
}

bool CAgilent8960::Handover_EDGE(INTL_GSM_BAND band, int nDestChan, int nPCL, double fBSPow)
{
	ASSERT_CONN

	if (fBSPow != 0 && !SetCellPower(fBSPow)) return false;

	if (band != m_Curr2GBand)
	{
		switch (band)
		{
		case INTL_2G850:
			if (!m_pIO->Write("CALL:SET:PDTCH:BAND GSM850")) return false;
			break;
		case INTL_2G900:
			if (!m_pIO->Write("CALL:SET:PDTCH:BAND EGSM")) return false;
			break;
		case INTL_2G1800:
			if (!m_pIO->Write("CALL:SET:PDTCH:BAND DCS")) return false;
			if (!m_pIO->Write("CALL:BIND DCS")) return false;
			break;
		case INTL_2G1900:
			if (!m_pIO->Write("CALL:SET:PDTCH:BAND PCS")) return false;
			if (!m_pIO->Write("CALL:BIND PCS")) return false;
			break;
		default:
			break;
		}
	}

	if (!m_pIO->Write(CCommon::FormatString(_T("CALL:SET:PDTCH %d"), nDestChan))) return false;
	if (!m_pIO->Write(CCommon::FormatString(_T("CALL:SET:PDTCH:MS:TXL:BURS %d"), nPCL))) return false;

	char sRet[10];
	int nLen = 10;
	if (!m_pIO->Query("CALL:HAND;*OPC?", nLen, sRet, 3000)) return false;

	bool bStatus = false;
	DWORD dwStartTime = GetTickCount();
	while ((GetTickCount() - dwStartTime) < 3000)
	{
		if (!QueryDataConnected(bStatus)) return false;

		if (bStatus)
		{
			m_Curr2GBand = band;
			break;
		}
		else
			SleepWithLog(200);
	}

	return bStatus;
}

bool CAgilent8960::Handover(INTL_WCDMA_BAND band, int nDestChan, double fBSPow)
{
	ASSERT_CONN

	if (nDestChan == m_nCurrChan) return true;

	char sCMD[100];
	sprintf_s(sCMD, "CALL:SET:CHAN:DOWN %d", nDestChan);
	if (!m_pIO->Write(sCMD)) return false;

	if (!m_pIO->Write("CALL:HAND:PCR")) return false;

	bool bCallOn = false;
	for (int i = 0; i < 5; i++)
	{
		if (!QueryOnCall(bCallOn)) return false;

		if (bCallOn)
		{
			m_Curr3GBand = band;
			m_nCurrChan = nDestChan;

			return true;
		}

		SleepWithLog(200);
	}

	return false;
}

bool CAgilent8960::BaseEndCall()
{
	ASSERT_CONN
	return m_pIO->Write("CALL:END");
}

bool CAgilent8960::BaseStartDataConnect()
{
	ASSERT_CONN
	return m_pIO->Write("CALL:FUNC:DATA:START");
}

bool CAgilent8960::BaseEndDataConnection()
{
	ASSERT_CONN
	return m_pIO->Write("CALL:FUNC:DATA:STOP");
}

bool CAgilent8960::QueryDataConnected(bool &b)
{
	ASSERT_CONN

	CString strRead;
	int nLen = 20;

	if(!m_pIO->Query(_T("CALL:STAT:DATA?"), nLen, strRead, 1000)) return false;
	b = (strRead.Find(_T("TRAN")) >= 0);

	return true;
}

bool CAgilent8960::QueryDataAttached(bool &b)
{
	ASSERT_CONN

	CString strRead;
	int nLen = 20;

	if (!m_pIO->Query(_T("CALL:STAT:DATA?"), nLen, strRead, 1000)) return false;
	b = (strRead.Find(_T("ATTG")) < 0 && strRead.Find(_T("ATT")) >= 0);

	return true;
}

bool CAgilent8960::Measure(GSM_MEASITEM item)
{
	ASSERT_CONN

	if (!m_bGSMFormat)
	{
		Log(_T("Wrong format!"));
		return false;
	}

	switch (item)
	{
	case GSM_PEAK_TX_POW:
	case GSM_AVG_TX_POW:
		return MeasureGSM_TxPower();
	case GSM_PEAK_FREQ_ERR:
	case GSM_PEAK_PHS_ERR:
	case GSM_RMS_PHS_ERR:
		return MeasureGSM_PFER();
	case GSM_PVT:
		return MeasureGSM_PVT();
	case GSM_SW_SPEC:
	case GSM_MOD_SPEC:
		return MeasureGSM_ORFS();
	case GSM_BER:
		return MeasureGSM_BER();
	case GSM_BLER:
		return MeasureGSM_BLER();
	case GSM_RXLVL:
		return MeasureGSM_RxLevel();
	case GSM_RXQ:
		return MeasureGSM_RxQuality();
	case EDGE_PEAK_TX_POW:
	case EDGE_AVG_TX_POW:
		return MeasureEDGE_TxPower();
	default:
		break;
	}

	Log(_T("Invalid test item name!"));
	return false;
}

bool CAgilent8960::Measure(WCDMA_MEASITEM item)
{
	ASSERT_CONN

	if (m_bGSMFormat)
	{
		Log(_T("Wrong format!"));
		return false;
	}

	switch (item)
	{
	case WCDMA_MAX_TX_POW:
		return Measure_WMaxTxPower();
	case WCDMA_MIN_TX_POW:
		return Measure_WMinTxPower();
	case WCDMA_FREQ_ERR:
	case WCDMA_RMS_PHS_ERR:
	case WCDMA_PCDE:
	case WCDMA_RMS_EVM:
	case WCDMA_RMS_MAG:
		return Measure_WWQ();
	case WCDMA_OBW:
		return Measure_OBW();
	case WCDMA_SEM:
		return Measure_SEM();
	case WCDMA_ACLR:
		return Measure_ACLR();
	case WCDMA_ILPC_E:
		return Measure_ILPC_E();
	case WCDMA_ILPC_F:
		return Measure_ILPC_F();
	case WCDMA_BER:
		return Measure_WBER();
	default:
		break;
	};

	Log(_T("Invalid test item name!"));
	return false;
}

bool CAgilent8960::FormatSwitch(APP_TYPE app)
{
	ASSERT_CONN

	if ((app != APP_GSM) && (app != APP_WCDMA))
	{
		Log(_T("Wrong application!"));
		return false;
	}

	CString strTargetFormat = (app == APP_GSM) ? _T("GSM/GPRS") : _T("WCDMA");

	CString strFormat;
	int nReadLen = 50;
	m_pIO->Query(_T("SYST:APPL:FORM?"), nReadLen, strFormat, 1000);

	if (strFormat.Find(strTargetFormat) >= 0) return true;

	CString strCMD;
	strCMD.Format(_T("SYST:APPL:FORM '%s';*OPC?"), strTargetFormat);

	nReadLen = 5; strFormat.Empty();
	m_pIO->Query(strCMD, nReadLen, strFormat, 5000);

	//Read and check
	nReadLen = 50; strFormat.Empty();
	m_pIO->Query(_T("SYST:APPL:FORM?"), nReadLen, strFormat, 1000);

	if (strFormat.Find(strTargetFormat) >= 0)
		return true;
	else
	{
		Log(_T("Format switch fail!"));
		return false;
	}
}

bool CAgilent8960::FullReset()
{
	ASSERT_CONN

	int nLen = 5;
	CString ss;

	if (!m_pIO->Query(_T("*RST;*OPC?"), nLen, ss, 3000)) return false;
	if (!m_pIO->Write("*CLS")) return false;

	if (!m_pIO->Write("SYST:LOG:UI:CLEAR")) return false;
	if (!m_pIO->Write("SYST:LOG:UI:GPIB:STAT ON")) return false;

	return true;
}

bool CAgilent8960::InitMeasure(CString sItem, int nTimeoutMS)
{
	ASSERT_CONN

	CString strCMD, strRead;

	strCMD.Format(_T("INIT:%s:ON"), sItem);
	if (!m_pIO->Write(strCMD)) return false;

	int nLen = 20;
	DWORD dwStartTime = GetTickCount(), dwTimeout = (DWORD)nTimeoutMS;
	while ((GetTickCount() - dwStartTime) < dwTimeout)
	{
		nLen = 20;
		if (!m_pIO->Query(_T("INIT:DONE?"), nLen, strRead, 1000)) return false;

		if (strRead.Find(_T("WAIT")) >= 0)
		{
			SleepWithLog(100);
			continue;
		}
		else if (strRead.Find(sItem) >= 0)
		{
			return true;
		}
		else
		{
			strCMD.Format(_T("ABOR:%s"), sItem);
			if (!m_pIO->Write(strCMD)) return false;

			return false;
		}
	}

	strCMD.Format(_T("ABOR:%s"), sItem);
	if (!m_pIO->Write(strCMD)) return false;

	return false;
}

bool CAgilent8960::MeasureGSM_TxPower()
{
	if (!InitMeasure(_T("TXP"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	if (!m_pIO->Query(_T("FETC:TXP:INT?"), nLen, sRead, 2000)) return false;

	sRead.Trim();

	int nIntegrity = _tstoi(sRead);
	if (nIntegrity != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	sRead.Empty();
	nLen = 100;
	if (!m_pIO->Query(_T("FETC:TXP:POW:ALL?"), nLen, sRead, 2000)) return false;

	int nCount =10 ;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;
	if (nCount != 4)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	m_GSMMeasStatus.bPeakTxPower = true;
	m_GSMMeasStatus.bAvgTxPower = true;

	m_GSMMeasStatus.fPeakTxPower = fResult[1];
	m_GSMMeasStatus.fAvgTxPower = fResult[2];

	return true;
}

bool CAgilent8960::ParseResult(CString s, double *pfResults, int &nMaxCount)
{
	memset(pfResults, 0, sizeof(double)* nMaxCount);

	int nCount = 0;
	CString *pstrResults = CCommon::ExtractStr(s, _T(","), &nCount);

	if (pstrResults == NULL || nCount == 0)
	{
		Log(_T("Failed to parse measurement result!"));
		return false;
	}

	for (int i = 0; i < nCount && i < nMaxCount; i++)
	{
		pfResults[i] = _tstof(pstrResults[i]);
	}

	CCommon::FreeStrings(pstrResults, nCount);
	nMaxCount = nCount;

	return true;
}

bool CAgilent8960::MeasureGSM_PFER()
{
	if (!InitMeasure(_T("PFER"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	sRead.Empty();
	nLen = 500;
	if (!m_pIO->Query(_T("FETC:PFER?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;
	if (nCount != 4)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_GSMMeasStatus.bPeakFreqErr = true;
	m_GSMMeasStatus.bPeakPhaseErr = true;
	m_GSMMeasStatus.bRMSPhaseErr = true;

	m_GSMMeasStatus.fRMSPhaseErr = fResult[1];
	m_GSMMeasStatus.fPeakPhaseErr = fResult[2];
	m_GSMMeasStatus.fPeakFreqErr = fResult[3];

	return true;
}

bool CAgilent8960::MeasureGSM_PVT()
{
	if (!InitMeasure(_T("PVT"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	sRead.Empty();
	nLen = 500;
	if (!m_pIO->Query(_T("FETC:PVT:ALL?"), nLen, sRead, 2000)) return false;

	int nCount = 20;
	double fResult[20];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 15)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_GSMMeasStatus.bPVT = true;
	m_GSMMeasStatus.bPVTResult = (fResult[1] == 0);
	memcpy(m_GSMMeasStatus.fPVTPowers, &fResult[3], 12);

	return true;
}

bool CAgilent8960::MeasureGSM_ORFS()
{
	if (!InitMeasure(_T("ORFS"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	sRead.Empty();
	nLen = 1024;
	if (!m_pIO->Query(_T("FETC:ORFS?"), nLen, sRead, 3000)) return false;

	int nCount = 50;
	double fResult[50];
	if (ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 32)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_GSMMeasStatus.bSwSpec = true;
	m_GSMMeasStatus.bModSpec = true;

	memcpy(m_GSMMeasStatus.fSWSpec, &fResult[2], 8);
	memcpy(m_GSMMeasStatus.fModSpec, &fResult[11], 22);

	sRead.Empty();
	nLen = 10;
	m_pIO->Query(_T("FETC:ORFS:SWIT:LIM?"), nLen, sRead, 1000); sRead.Trim();
	m_GSMMeasStatus.bSwSpecRes = (sRead == _T("0"));

	sRead.Empty();
	nLen = 10;
	m_pIO->Query(_T("FETC:ORFS:MOD:LIM?"), nLen, sRead, 1000); sRead.Trim();
	m_GSMMeasStatus.bModSpecRes = (sRead == _T("0"));

	return true;
}

bool CAgilent8960::MeasureGSM_BER()
{
	if (!InitMeasure(_T("FBER"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	sRead.Empty();
	nLen = 500;
	if (!m_pIO->Query(_T("FETC:FBER?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 4)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_GSMMeasStatus.bBER = true;
	m_GSMMeasStatus.fBER = fResult[2];

	return true;
}

bool CAgilent8960::MeasureGSM_BLER()
{
	if (!InitMeasure(_T("BLER"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	sRead.Empty();
	nLen = 500;
	if (!m_pIO->Query(_T("FETC:BLER?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 4)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_GSMMeasStatus.bBLER = true;
	m_GSMMeasStatus.fBLER = fResult[2];

	return true;
}

bool CAgilent8960::MeasureGSM_RxLevel()
{
	CString sRead;

	sRead.Empty();
	int nLen = 100;
	if (!m_pIO->Query(_T("CALL:MS:REP:MEAS:SACCH:RXL:FULL:NEW?;NEW?;NEW?"), nLen, sRead, 5000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 3)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	m_GSMMeasStatus.bRxLevel = true;
	m_GSMMeasStatus.nRxLevel = (int)fResult[2];

	return true;
}

bool CAgilent8960::MeasureGSM_RxQuality()
{
	CString sRead;

	sRead.Empty();
	int nLen = 100;
	if (!m_pIO->Query(_T("CALL:MS:REP:MEAS:SACCH:RXQ:FULL:NEW?;NEW?;NEW?"), nLen, sRead, 5000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 3)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	m_GSMMeasStatus.bRxQuality = true;
	m_GSMMeasStatus.nRxQuality = (int)fResult[2];

	return true;
}

bool CAgilent8960::MeasureEDGE_TxPower()
{
	if (!InitMeasure(_T("ETXP"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	if (!m_pIO->Query(_T("FETC:ETXP:INT?"), nLen, sRead, 2000)) return false;

	sRead.Trim();
	int nIntegrity = _tstoi(sRead);
	if (nIntegrity != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	sRead.Empty();
	nLen = 100;
	if (!m_pIO->Query(_T("FETC:ETXP:POW:ALL?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 4)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	m_GSMMeasStatus.bEDGEPeakTxPower = true;
	m_GSMMeasStatus.bEDGEAvgTxPower = true;

	m_GSMMeasStatus.fEDGEPeakTxPower = fResult[1];
	m_GSMMeasStatus.fEDGEAvgTxPower = fResult[2];

	return true;
}

bool CAgilent8960::MeasureWTxPower(double &fPow)
{
	if (!InitMeasure(_T("WCP"), 2000)) return false;

	CString sRead;
	int nLen = 10;

	if (!m_pIO->Query(_T("FETC:WCP:INT?"), nLen, sRead, 2000)) return false;

	sRead.Trim();
	int nIntegrity = _tstoi(sRead);
	if (nIntegrity != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	sRead.Empty();
	nLen = 50;
	if (m_pIO->Query(_T("FETC:WCP:POW?"), nLen, sRead, 2000))
	{
		fPow = _tstof(sRead);
		return true;
	}

	return false;
}

bool CAgilent8960::Measure_WMaxTxPower()
{
	double fPow = 0;
	if (!MeasureWTxPower(fPow)) return false;

	m_WCDMAMeasStatus.bMaxTxPower = true;
	m_WCDMAMeasStatus.fMaxTxPower = fPow;

	return true;
}

bool CAgilent8960::Measure_WMinTxPower()
{
	if (!m_pIO->Write(_T("CALL:MS:POW:TARG -50"))) return false;
	if (!m_pIO->Write(_T("CALL:CLPC:UPL:DOWN"))) return false;

	SleepWithLog(100);

	double fPow = 0;
	if (!MeasureWTxPower(fPow)) return false;

	m_WCDMAMeasStatus.bMinTxPower = true;
	m_WCDMAMeasStatus.fMinTxPower = fPow;

	if (!m_pIO->Write(_T("CALL:MS:POW:TARG 23"))) return false;
	if (!m_pIO->Write(_T("CALL:CLPC:UPL:UP"))) return false;

	return true;
}

bool CAgilent8960::Measure_WWQ()
{
	if (!InitMeasure(_T("WWQ"), 2000)) return false;

	CString sRead;
	int nLen = 1024;

	if (!m_pIO->Query(_T("FETC:WWQ?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 8)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_WCDMAMeasStatus.bRMSEVM = true;
	m_WCDMAMeasStatus.fRMSEVM = fResult[1];

	m_WCDMAMeasStatus.bFreqErr = true;
	m_WCDMAMeasStatus.fFreqErr = fResult[2];

	m_WCDMAMeasStatus.bRMSPhaseErr = true;
	m_WCDMAMeasStatus.fRMSPhaseErr = fResult[4];

	m_WCDMAMeasStatus.bRMSMagnitude = true;
	m_WCDMAMeasStatus.fRMSMagnitude = fResult[5];

	m_WCDMAMeasStatus.bPCDE = true;
	m_WCDMAMeasStatus.fPCDE = fResult[7];

	return true;
}

bool CAgilent8960::Measure_OBW()
{
	if (!InitMeasure(_T("WOBW"), 2000)) return false;

	CString sRead;
	int nLen = 1024;

	if (!m_pIO->Query(_T("FETC:WOBW?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 4)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_WCDMAMeasStatus.bOBW = true;
	m_WCDMAMeasStatus.fOBW = fResult[1];

	return true;
}

bool CAgilent8960::Measure_SEM()
{
	if (!InitMeasure(_T("WSEM"), 2000)) return false;

	CString sRead;
	int nLen = 5;

	if (!m_pIO->Query(_T("FETC:WSEM:INT?"), nLen, sRead, 2000)) return false;

	sRead.Trim();
	int nIntegrity = _tstoi(sRead);
	if (nIntegrity != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	sRead.Empty();
	nLen = 1024;
	if (!m_pIO->Query(_T("FETC:WSEM:RANG?"), nLen, sRead, 2000)) return false;

	int nCount = 20;
	double fResult[20];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 16)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	m_WCDMAMeasStatus.bSEM = true;

	for (int i = 0; i < 4; i++)
	{
		m_WCDMAMeasStatus.fSEM_Level[i] = fResult[i * 4];
		m_WCDMAMeasStatus.fSEM_FreqOffset[i] = fResult[i * 4 + 1];
		m_WCDMAMeasStatus.fSEM_MaskMargin[i] = fResult[i * 4 + 2];
		m_WCDMAMeasStatus.bSEMPass[i] = (fResult[i * 4 + 3] == 0);
	}

	return false;
}

bool CAgilent8960::Measure_ACLR()
{
	if (!InitMeasure(_T("WACL"), 2000)) return false;

	CString sRead;
	int nLen = 150;

	if (!m_pIO->Query(_T("FETC:WACL?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 5)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_WCDMAMeasStatus.bACLR = true;

	m_WCDMAMeasStatus.fACLR[0] = fResult[3];//-10
	m_WCDMAMeasStatus.fACLR[1] = fResult[1];//-5
	m_WCDMAMeasStatus.fACLR[2] = fResult[2];//5
	m_WCDMAMeasStatus.fACLR[3] = fResult[4];//10

	return true;
}

bool CAgilent8960::Measure_ILPC(double fPowers[150], int &nCount)
{
	if (!InitMeasure(_T("WILP"), 5000)) return false;

	CString sRead;
	int nLen = 5;

	if (!m_pIO->Query(_T("FETC:WILP:INT?"), nLen, sRead, 1000)) return false;

	sRead.Trim();
	int nIntegrity = _tstoi(sRead);
	if (nIntegrity != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	sRead.Empty();
	nLen = 1024;
	if (!m_pIO->Query(_T("FETC:WILP:TRAC?"), nLen, sRead, 1000)) return false;

	nCount = 150;
	return ParseResult(sRead, fPowers, nCount);
}

bool CAgilent8960::Measure_ILPC_E()
{
	if (!m_pIO->Write(_T("SET:WILP:SEG E"))) return false;

	if (!Measure_ILPC(m_WCDMAMeasStatus.fILPC_E, m_WCDMAMeasStatus.nILPC_E_Count)) return false;

	m_WCDMAMeasStatus.bILPC_E = true;

	return true;
}

bool CAgilent8960::Measure_ILPC_F()
{
	if (!m_pIO->Write(_T("SET:WILP:SEG F"))) return false;

	if (!Measure_ILPC(m_WCDMAMeasStatus.fILPC_F, m_WCDMAMeasStatus.nILPC_F_Count)) return false;

	m_WCDMAMeasStatus.bILPC_F = true;
	return true;
}

bool CAgilent8960::Measure_WBER()
{
	if (!InitMeasure(_T("WBER"), 2000)) return false;

	CString sRead;
	int nLen = 100;

	if (!m_pIO->Query(_T("FETC:WBER?"), nLen, sRead, 2000)) return false;

	int nCount = 10;
	double fResult[10];
	if (!ParseResult(sRead, fResult, nCount)) return false;

	if (nCount != 4)
	{
		Log(_T("Wrong measurement result count!"));
		return false;
	}

	if (fResult[0] != 0)
	{
		Log(_T("Wrong measuremnt integrity"));
		return false;
	}

	m_WCDMAMeasStatus.bBER = true;
	m_WCDMAMeasStatus.fBER = fResult[1];

	return true;
}

bool CAgilent8960::SaveGPIBLog(CString strFile)
{
	ASSERT_CONN

	CString sRead;
	int nLen = 10;

	if (!m_pIO->Query(_T("SYST:LOG:UI:COUN?"), nLen, sRead, 1000)) return false;

	sRead.Trim();
	int nLineNum = _tstoi(sRead);

	if (nLineNum <= 0) return true;

	CStdioFile logfile;
	if (!logfile.Open(strFile, CFile::modeCreate | CFile::modeWrite))
	{
		Log(CCommon::FormatString(_T("Failed to open GPIB log file:\n%s\nError information:\n%s"), strFile, CCommon::GetSystemErrorMessage()));
		return false;
	}

	CString sTemp;
	for (int i = 0; i < nLineNum; i++)
	{
		sTemp.Format(_T("SYST:LOG:UI:REP? %d"), i + 1);

		sRead.Empty();
		nLen = 512;

		if (!m_pIO->Query(sTemp, nLen, sRead, 1000)) { logfile.Close(); return false; }

		logfile.WriteString(sRead + _T("\r\n"));
	}

	logfile.Close();
	return true;
}

bool CAgilent8960::SetBand(INTL_GSM_BAND band)
{
	ASSERT_CONN

	switch (band)
	{
	case INTL_2G850:
		if (!m_pIO->Write("CALL:TCH:BAND GSM850")) return false;
		break;
	case INTL_2G900:
		if (!m_pIO->Write("CALL:TCH:BAND EGSM")) return false;
		break;
	case INTL_2G1800:
		if (!m_pIO->Write("CALL:TCH:BAND DCS")) return false;
		if (!m_pIO->Write("CALL:BIND DCS")) return false;
		break;
	case INTL_2G1900:
		if (!m_pIO->Write("CALL:TCH:BAND PCS")) return false;
		if (!m_pIO->Write("CALL:BIND PCS")) return false;
		break;
	default:
		return false;
	}

	return true;
}

bool CAgilent8960::SetBCHBand(INTL_GSM_BAND band)
{
	ASSERT_CONN

	switch (band)
	{
		case INTL_2G850:
			if (!m_pIO->Write("CALL:BAND GSM850")) return false;
			break;
		case INTL_2G900:
			if (!m_pIO->Write("CALL:BAND EGSM")) return false;
			break;
		case INTL_2G1800:
			if (!m_pIO->Write("CALL:BAND DCS")) return false;
			break;
		case INTL_2G1900:
			if (!m_pIO->Write("CALL:BAND PCS")) return false;
			break;
		default:
			return false;
	}

	return true;
}

bool CAgilent8960::SetPDTCHBand(INTL_GSM_BAND band)
{
	ASSERT_CONN

	switch (band)
	{
	case INTL_2G850:
		if (!m_pIO->Write("CALL:PDTCH:BAND GSM850")) return false;
		break;
	case INTL_2G900:
		if (!m_pIO->Write("CALL:PDTCH:BAND EGSM")) return false;
		break;
	case INTL_2G1800:
		if (!m_pIO->Write("CALL:PDTCH:BAND DCS")) return false;
		if (!m_pIO->Write("CALL:BIND DCS")) return false;
		break;
	case INTL_2G1900:
		if (!m_pIO->Write("CALL:PDTCH:BAND PCS")) return false;
		if (!m_pIO->Write("CALL:BIND PCS")) return false;
		break;
	default:
		return false;
	}

	return true;
}

bool CAgilent8960::SetTCH(int nTCH)
{
	ASSERT_CONN

	char sCMD[100];
	sprintf_s(sCMD, "CALL:TCH %d", nTCH);
	return m_pIO->Write(sCMD);
}

bool CAgilent8960::SetPDTCH(int nTCH)
{
	ASSERT_CONN

	char sCMD[100];
	sprintf_s(sCMD, "CALL:PDTCH %d", nTCH);
	return m_pIO->Write(sCMD);
}

bool CAgilent8960::SetBCH(int nBCH)
{
	ASSERT_CONN

	char sCMD[100];
	sprintf_s(sCMD, "CALL:BCH %d", nBCH);
	return m_pIO->Write(sCMD);
}

bool CAgilent8960::SetPCL(int nPCL)
{
	ASSERT_CONN

	char sCMD[100];

	if (m_bEDGETest)
		sprintf_s(sCMD, "CALL:PDTCH:MS:TXL:BURS %d", nPCL);
	else
		sprintf_s(sCMD, "CALL:MS:TXL %d", nPCL);

	return m_pIO->Write(sCMD);
}

bool CAgilent8960::SetCellPower(double fCellPower)
{
	ASSERT_CONN

	char sCMD[100];
	sprintf_s(sCMD, "CALL:POW %.3f", fCellPower);
	return m_pIO->Write(sCMD);
}

bool CAgilent8960::SetDLChannel(int nDLChan)
{
	ASSERT_CONN

	char sCMD[100];
	sprintf_s(sCMD, "CALL:CHAN %d", nDLChan);
	return m_pIO->Write(sCMD);
}

bool CAgilent8960::SetULChannel(int nULChan)
{
	ASSERT_CONN

	char sCMD[100];
	sprintf_s(sCMD, "CALL:UPL:CHAN %d", nULChan);
	return m_pIO->Write(sCMD);
}

bool CAgilent8960::SetupGSMNSigTxTest()
{
	ASSERT_CONN

	if (m_Application != APP_FAST_SWITCH && m_Application != APP_GSM)
	{
		Log(_T("Wrong application"));
		return false;
	}

	if (!FullReset()) return false;
	if (!SetRFPort(NULL, NULL)) return false;

	if (m_Application == APP_FAST_SWITCH && !FormatSwitch(APP_GSM)) return false;

	m_bGSMFormat = true;
	m_bEDGETest = false;

	if (!m_pIO->Write("CALL:OPER:MODE PBPT")) return false;

	if (!m_pIO->Write("SET:TXP:COUNt 5")) return false;
	if (!m_pIO->Write("SET:TXP:CONT OFF")) return false;
	if (!m_pIO->Write("SET:TXP:COUN:STAT ON")) return false;
	if (!m_pIO->Write("SET:TXP:TIM:STIM 0.2")) return false;
	if (!m_pIO->Write("SET:TXP:TRIG:SOUR RISE")) return false;
	if (!m_pIO->Write("CALL:BURS:TYPE TSC0")) return false;
	if (!m_pIO->Write("RFAN:CONT:POW:AUTO OFF")) return false;

	return true;
}

bool CAgilent8960::SetupGSMNSigTxMeasSetup(int nBand, int nChan, double fExpPower)
{
	char cmd[100];
	sprintf_s(cmd, "RFAN:MAN:MEAS %.1f MHz", GSMChannel2FreqMHz((INTL_GSM_BAND)nBand, nChan, true));
	if (!m_pIO->Write(cmd)) return false;

	sprintf_s(cmd, "RFAN:MAN:POW:BURS %.1f", fExpPower);
	if (!m_pIO->Write(cmd)) return false;

	return true;
}

bool CAgilent8960::SetupGSMNSigRx()
{
	ASSERT_CONN

	if (m_Application != APP_FAST_SWITCH && m_Application != APP_GSM)
	{
		Log(_T("Wrong application"));
		return false;
	}

	if (!FullReset()) return false;
	if (!SetRFPort(NULL, NULL)) return false;

	if (m_Application == APP_FAST_SWITCH && !FormatSwitch(APP_GSM)) return false;

	m_bGSMFormat = true;
	m_bEDGETest = false;

	if (!m_pIO->Write("CALL:OPER:MODE GBTT")) return false;

	m_Curr2GBand = INTL_2G_BAND_NUM;
	m_nCurrChan = -1;
	m_fBSPower = -1.0;

	return true;
}

bool CAgilent8960::SetupGSMNSigRxTest(INTL_GSM_BAND band, int nChannel, double fBSPower)
{
	ASSERT_CONN

	if (m_Curr2GBand != band && !SetBand(band)) return false;
	if (!SetTCH(nChannel)) return false;

	if (m_Curr2GBand != band && !SetBCHBand(band)) return false;
	if (!SetBCH(nChannel)) return false;

	if (m_fBSPower != fBSPower && !SetCellPower(fBSPower)) return false;

	m_Curr2GBand = band;
	m_nCurrChan = nChannel;
	m_fBSPower = fBSPower;

	char sRet[100];
	memset(sRet, 0, sizeof(sRet));
	int nLen = 100;

	return m_pIO->Query("SYST:SYNC?", nLen, sRet, 2000);
}

bool CAgilent8960::SetupWCDMANSigTx()
{
	ASSERT_CONN

	if (m_Application != APP_FAST_SWITCH && m_Application != APP_WCDMA)
	{
		Log(_T("Wrong application"));
		return false;
	}

	if (!FullReset()) return false;
	if (!SetRFPort(NULL, NULL)) return false;

	if (m_Application == APP_FAST_SWITCH && !FormatSwitch(APP_WCDMA)) return false;

	m_bGSMFormat = false;
	m_bEDGETest = false;

	if (!m_pIO->Write("CALL:OPER:MODE FDDT")) return false;

	return true;
}

bool CAgilent8960::SetupWCDMANSigTxTest(INTL_WCDMA_BAND eBand, int nChan, double fExpPow)
{
	ASSERT_CONN

	float fFreqMHz = WCDMAChannel2FreqMHz(eBand, nChan, true);

	char sRet[100];

	sprintf_s(sRet, "RFAN:MAN:UPL:MFR %.1f MHz", fFreqMHz);
	if (!m_pIO->Write(sRet)) return false;

	sprintf_s(sRet, "RFAN:MAN:MEAS:MFR %.1f MHz", fFreqMHz);
	if (!m_pIO->Write(sRet)) return false;

	if (!m_pIO->Write("RFAN:CONT:POW:AUTO OFF")) return false;

	sprintf_s(sRet, "RFAN:MAN:POW:FDD %.1f dbm", fExpPow);
	if (!m_pIO->Write(sRet)) return false;

	return true;
}

bool CAgilent8960::SetupWCDMANSigRx()
{
	ASSERT_CONN

	if (m_Application != APP_FAST_SWITCH && m_Application != APP_WCDMA)
	{
		Log(_T("Wrong application"));
		return false;
	}

	if (!FullReset()) return false;
	if (!SetRFPort(NULL, NULL)) return false;

	if (m_Application == APP_FAST_SWITCH && !FormatSwitch(APP_WCDMA)) return false;

	m_bGSMFormat = false;
	m_bEDGETest = false;

	m_fBSPower = -1;

	if (!m_pIO->Write("CALL:OPER:MODE CW")) return false;

	return true;
}

bool CAgilent8960::SetupWCDMANSigRxTest(INTL_WCDMA_BAND eBand, int nChannel, double fBSPower)
{
	ASSERT_CONN

	if (m_fBSPower != fBSPower && !SetCellPower(fBSPower)) return false;

	m_fBSPower = fBSPower;

	char sRet[100];
	sprintf_s(sRet, "CALL:RFG:FREQ %.1f MHz", WCDMAChannel2FreqMHz(eBand, nChannel, false));
	if (!m_pIO->Write(sRet)) return false;

	int nLen = 100;
	memset(sRet, 0, sizeof(sRet));
	return m_pIO->Query("SYST:SYNC?", nLen, sRet, 2000);

	return true;
}