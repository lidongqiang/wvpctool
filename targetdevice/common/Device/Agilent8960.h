#pragma once

#include "RFTester.h"

typedef enum
{
	APP_GSM,
	APP_WCDMA,
	APP_FAST_SWITCH
}APP_TYPE;

class CAgilent8960 : public CRFTester
{
public:
	CAgilent8960();
	~CAgilent8960();

	virtual CString GetDeviceName();

	virtual bool GetRFPorts(CArray<CString> &IOPorts, CArray<CString> &outPorts);
	virtual bool SetRFPort(LPCTSTR strMainPort, LPCTSTR strDivPort);
	virtual bool SetCableLoss(CArray<double> &freqs, CArray<double> &losses);

	virtual bool GeneralConfig();

	virtual bool SetupGSMTest(INTL_GSM_BAND, int, int, double);
	virtual bool SetupEDGETest(INTL_GSM_BAND, int, int, double);
	virtual bool SetupWCDMATest(INTL_WCDMA_BAND, int, double);
	virtual bool SetupLTETest();
	virtual bool SetupTDSTest();

	virtual bool SetupGSMNSigTxTest();
	virtual bool SetupGSMNSigTxMeasSetup(int Band, int nChan, double fExpPower);

	virtual bool SetupGSMNSigRx();
	virtual bool SetupGSMNSigRxTest(INTL_GSM_BAND, int, double);

	virtual bool SetupWCDMANSigTx();
	virtual bool SetupWCDMANSigTxTest(INTL_WCDMA_BAND, int, double);

	virtual bool SetupWCDMANSigRx();
	virtual bool SetupWCDMANSigRxTest(INTL_WCDMA_BAND, int, double);

	virtual bool QueryOnCall(bool &bCall);
	virtual bool QuerySync(bool &bSync);
	virtual bool ArmCallConnection();
	virtual bool BaseCall();
	virtual bool Handover(INTL_GSM_BAND band, int nDestChan, int nPCL, double fBSPow);
	virtual bool Handover(INTL_WCDMA_BAND band, int nDestChan, double fBSPow);
	virtual bool BaseEndCall();

	virtual bool BaseStartDataConnect();
	virtual bool BaseEndDataConnection();
	virtual bool QueryDataConnected(bool &b);
	virtual bool QueryDataAttached(bool &b);

	virtual bool Measure(GSM_MEASITEM item);
	virtual bool Measure(WCDMA_MEASITEM item);

private:
	virtual bool ReadIdentity();

	bool FullReset();
	bool FormatSwitch(APP_TYPE app);
	bool ParseResult(CString s, double *pfResults, int &nMaxCount);

	bool Handover_GMSK(INTL_GSM_BAND band, int nDestChan, int nPCL, double fBSPow);
	bool Handover_EDGE(INTL_GSM_BAND band, int nDestChan, int nPCL, double fBSPow);

	//GSM
	bool SetBand(INTL_GSM_BAND band);
	bool SetBCHBand(INTL_GSM_BAND band);
	bool SetTCH(int nTCH);
	bool SetBCH(int nBCH);
	bool SetPCL(int nPCL);
	bool SetCellPower(double fBSLevel);

	bool SetPDTCHBand(INTL_GSM_BAND band);
	bool SetPDTCH(int nTCH);

	bool InitMeasure(CString sItem, int nTimeoutMS);
	bool MeasureGSM_TxPower();
	bool MeasureGSM_PFER();
	bool MeasureGSM_PVT();
	bool MeasureGSM_ORFS();
	bool MeasureGSM_BER();
	bool MeasureGSM_BLER();
	bool MeasureGSM_RxLevel();
	bool MeasureGSM_RxQuality();
	bool MeasureEDGE_TxPower();

	bool MeasureWTxPower(double &fPow);
	bool Measure_WMaxTxPower();
	bool Measure_WMinTxPower();
	bool Measure_WWQ();
	bool Measure_OBW();
	bool Measure_SEM();
	bool Measure_ACLR();
	bool Measure_ILPC(double fPowers[150], int &nCount);
	bool Measure_ILPC_E();
	bool Measure_ILPC_F();
	bool Measure_WBER();

	bool SetDLChannel(int nDLChan);
	bool SetULChannel(int nULChan);

	bool SaveGPIBLog(CString strFile);

	CString m_strID;

	APP_TYPE m_Application;
	bool m_bEGPRSSupport;
	bool m_bGSMFormat;
	bool m_bEDGETest;

	INTL_GSM_BAND m_Curr2GBand;
	INTL_WCDMA_BAND m_Curr3GBand;
	int m_nCurrChan;
	double m_fBSPower;
};

