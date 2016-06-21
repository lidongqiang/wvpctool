#pragma once

#include "GPIBDevice.h"
#include "VISADevice.h"
//#include "..\Config.h"

#define INV_VAL 999999

#ifdef _DEBUG
#define ASSERT_CONN if(m_pIO==NULL){Log(_T("Connection driver is not initialized"));return false;}
#else
#define ASSERT_CONN
#endif

class CRFTester
{
protected:

	typedef enum
	{
		GSM_PEAK_TX_POW = 0,
		GSM_AVG_TX_POW,
		EDGE_PEAK_TX_POW,
		EDGE_AVG_TX_POW,
		GSM_PEAK_FREQ_ERR,
		GSM_PEAK_PHS_ERR,
		GSM_RMS_PHS_ERR,
		GSM_PVT,
		GSM_SW_SPEC,
		GSM_MOD_SPEC,
		GSM_BER,
		GSM_BLER,
		GSM_RXQ,
		GSM_RXLVL,
		GSM_ITEM_COUNT
	}GSM_MEASITEM;

	typedef enum
	{
		WCDMA_MAX_TX_POW = 0,
		WCDMA_MIN_TX_POW,
		WCDMA_FREQ_ERR,
		//WCDMA_PEAK_PHS_ERR,
		WCDMA_RMS_PHS_ERR,
		WCDMA_OBW,
		WCDMA_PCDE,
		//WCDMA_PEAK_EVM,
		WCDMA_RMS_EVM,
		WCDMA_SEM,
		WCDMA_ILPC_E,
		WCDMA_ILPC_F,
		//WCDMA_PEAK_MAG,
		WCDMA_RMS_MAG,
		WCDMA_ACLR,
		WCDMA_BER,
		WCDMA_ITEM_COUNT
	}WCDMA_MEASITEM;

	class CGSMMeasurementItems
	{
	public:

		CGSMMeasurementItems(){ Clear(); };

		//Tx
		bool bPeakTxPower;
		double fPeakTxPower;

		bool bAvgTxPower;
		double fAvgTxPower;

		bool bEDGEPeakTxPower;
		double fEDGEPeakTxPower;

		bool bEDGEAvgTxPower;
		double fEDGEAvgTxPower;

		bool bPeakFreqErr;
		double fPeakFreqErr;

		bool bPeakPhaseErr;
		double fPeakPhaseErr;

		bool bRMSPhaseErr;
		double fRMSPhaseErr;

		bool bPVT;
		bool bPVTResult;
		double fPVTPowers[12];

		bool bSwSpec;
		bool bSwSpecRes;
		double fSWSpec[8];

		bool bModSpec;
		bool bModSpecRes;
		double fModSpec[22];

		//Rx
		bool bBER;
		double fBER;

		bool bRxLevel;
		int nRxLevel;

		bool bRxQuality;
		int nRxQuality;

		bool bBLER;
		double fBLER;

		void Clear()
		{
			bPeakTxPower = false;
			fPeakTxPower = INV_VAL;

			bAvgTxPower = false;
			fAvgTxPower = INV_VAL;

			bEDGEPeakTxPower = false;
			fEDGEPeakTxPower = INV_VAL;

			bEDGEAvgTxPower = false;
			fEDGEAvgTxPower = INV_VAL;

			bPeakFreqErr = false;
			fPeakFreqErr = INV_VAL;

			bPeakPhaseErr = false;
			fPeakPhaseErr = INV_VAL;

			bRMSPhaseErr = false;
			fRMSPhaseErr = INV_VAL;

			bPVT = false;
			bPVTResult = false;
			memset(fPVTPowers, 0, sizeof(fPVTPowers));

			bSwSpec = false;
			bSwSpecRes = false;
			memset(fSWSpec, 0, sizeof(fSWSpec));

			bModSpec = false;
			bModSpecRes = false;
			memset(fModSpec, 0, sizeof(fModSpec));

			bBER = false;
			fBER = INV_VAL;

			bRxLevel = false;
			nRxLevel = INV_VAL;

			bRxQuality = false;
			nRxQuality = INV_VAL;

			bBLER = false;
			fBLER = INV_VAL;
		}
	};

	class CWCDMAMeasrementItems
	{
	public:
		CWCDMAMeasrementItems(){ Clear(); };

		bool bMaxTxPower;
		double fMaxTxPower;

		bool bMinTxPower;
		double fMinTxPower;

		bool bFreqErr;
		double fFreqErr;

		bool bPeakPhaseErr;
		double fPeakPhaseErr;

		bool bRMSPhaseErr;
		double fRMSPhaseErr;

		bool bPeakEVM;
		double fPeakEVM;

		bool bRMSEVM;
		double fRMSEVM;

		bool bRMSMagnitude;
		double fRMSMagnitude;

		bool bPeakMagnitude;
		double fPeakMagnitude;

		bool bPCDE;
		double fPCDE;

		bool bACLR;
		double fACLR[4];//[-5,5,-10,10]

		bool bOBW;
		double fOBW;

		bool bILPC_E;
		int nILPC_E_Count;
		double fILPC_E[150];

		bool bILPC_F;
		int nILPC_F_Count;
		double fILPC_F[150];

		bool bSEM;
		double fSEM_Level[4];
		double fSEM_FreqOffset[4];
		double fSEM_MaskMargin[4];
		bool bSEMPass[4];

		//Rx
		bool bBER;
		double fBER;

		void Clear()
		{
			bMaxTxPower = false;
			fMaxTxPower = INV_VAL;

			bMinTxPower = false;
			fMinTxPower = INV_VAL;

			bFreqErr = false;
			fFreqErr = INV_VAL;

			bPeakPhaseErr = false;
			fPeakPhaseErr = INV_VAL;

			bRMSPhaseErr = false;
			fRMSPhaseErr = INV_VAL;

			bPeakEVM = false;
			fPeakEVM = INV_VAL;

			bRMSEVM = false;
			fRMSEVM = INV_VAL;

			bRMSMagnitude = false;
			fRMSMagnitude = INV_VAL;

			bPeakMagnitude = false;
			fPeakMagnitude = INV_VAL;

			bPCDE = false;
			fPCDE = INV_VAL;

			bACLR = false;
			memset(fACLR, 0, sizeof(fACLR));

			bOBW = false;
			fOBW = INV_VAL;

			bILPC_E = false;
			nILPC_E_Count = -1;
			memset(fILPC_E, 0, sizeof(fILPC_E));

			bILPC_F = false;
			nILPC_F_Count = -1;
			memset(fILPC_F, 0, sizeof(fILPC_F));

			bSEM = false;
			memset(fSEM_Level, 0, sizeof(double) * 4);
			memset(fSEM_FreqOffset, 0, sizeof(double)* 4);
			memset(fSEM_MaskMargin, 0, sizeof(double)* 4);
			memset(bSEMPass, 0, sizeof(bool) * 4);

			//Rx
			bBER = false;
			fBER = -1;
		};
	};

public:

	CRFTester(void);
	virtual ~CRFTester(void);

	virtual CString GetDeviceName() = 0;

	void SetLogger(CLogger *p){ m_pLogger = p; };
	void SleepWithLog(int nMs);
	void Log(LPCTSTR s);

	bool Open(LPCTSTR strVISAName);
	bool Open(int nBUS, int nAddr);
	bool Close();

	virtual bool GetRFPorts(CArray<CString> &IOPorts, CArray<CString> &outPorts) = 0;
	virtual bool SetRFPort(LPCTSTR strMainPort, LPCTSTR strDivPort) = 0;
	virtual bool SetCableLoss(CArray<double> &freqs, CArray<double> &losses) = 0;

	virtual bool GeneralConfig() = 0;

	virtual bool SetupGSMTest(INTL_GSM_BAND, int, int, double) = 0;
	virtual bool SetupEDGETest(INTL_GSM_BAND, int, int, double) = 0;
	virtual bool SetupWCDMATest(INTL_WCDMA_BAND, int, double) = 0;
	virtual bool SetupLTETest() = 0;
	virtual bool SetupTDSTest() = 0;

	virtual bool SetupGSMNSigTxTest() = 0;
	virtual bool SetupGSMNSigTxMeasSetup(int Band, int nChan, double fExpPower) = 0;

	virtual bool SetupGSMNSigRx() = 0;
	virtual bool SetupGSMNSigRxTest(INTL_GSM_BAND, int, double) = 0;

	virtual bool SetupWCDMANSigTx() = 0;
	virtual bool SetupWCDMANSigTxTest(INTL_WCDMA_BAND, int, double) = 0;

	virtual bool SetupWCDMANSigRx() = 0;
	virtual bool SetupWCDMANSigRxTest(INTL_WCDMA_BAND, int, double) = 0;

	virtual bool QueryOnCall(bool &bCall) = 0;
	virtual bool QuerySync(bool &bSync) = 0;
	virtual bool ArmCallConnection() = 0;
	virtual bool BaseCall() = 0;
	virtual bool Handover(INTL_GSM_BAND band, int nDestChan, int nPCL, double fBSPow) = 0;
	virtual bool Handover(INTL_WCDMA_BAND band, int nDestChan, double fBSPow) = 0;
	virtual bool BaseEndCall() = 0;

	virtual bool BaseStartDataConnect() = 0;
	virtual bool BaseEndDataConnection() = 0;
	virtual bool QueryDataConnected(bool &b) = 0;
	virtual bool QueryDataAttached(bool &b) = 0;

	virtual bool Measure(GSM_MEASITEM item) = 0;
	virtual bool Measure(WCDMA_MEASITEM item) = 0;

	void ClearMeasureStatus();

	//GSM
	bool GetGSMPeakTxPower(double &);
	bool GetGSMAvgTxPower(double &);
	bool GetEDGEPeakTxPower(double &);
	bool GetEDGEAvgTxPower(double &);
	bool GetGSMPeakFreqErr(double &);
	bool GetGSMPeakPhaseErr(double &);
	bool GetGSMRMSPhaseErr(double &);
	bool GetGSMPVT(bool &);
	bool GetGSMSwSpectrum(bool &);
	bool GetGSMModSpectrum(bool &);

	bool GetGSMBER(double &);
	bool GetGSMRxLevel(int &);
	bool GetGSMRxQuality(int &);
	bool GetGSMBLER(double &);

	//WCDMA
	bool GetWMaxTxPower(double &);
	bool GetWMinTxPower(double &);
	bool GetWFreqErr(double &);
	//bool GetWPeakPhaseErr(double &);
	bool GetWRMSPhaseErr(double &);
	bool GetWOBW(double &);
	bool GetWACLR(double fACLR[4]);
	//bool GetWPeakEVM(double &);
	bool GetWRMSEVM(double &);
	bool GetWRMSMagnitude(double &);
	//bool GetWPeakMagnitude(double &);
	bool GetWPCDE(double &);
	bool GetWSEM(int nRange, double &fLevel, double &fFreqOffset, double &fMaskMargin, bool &bPass);
	bool GetWSEM(bool &bTotalPass);
	bool GetWILPC_E(int &nCount, double fILPC[150]);
	bool GetWILPC_F(int &nCount, double fILPC[150]);

	bool GetWBER(double &);

private:

	CGPIBDevice *m_pGPIBDevice;
	CVISADevice *m_pVISADevice;

	CLogger *m_pLogger;
protected:

	virtual bool ReadIdentity() = 0;

	CBaseIODevice *m_pIO;

	CGSMMeasurementItems m_GSMMeasStatus;
	CWCDMAMeasrementItems m_WCDMAMeasStatus;
};
