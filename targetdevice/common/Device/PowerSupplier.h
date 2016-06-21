#pragma once

#include "GPIBDevice.h"

class CPowerSupplier
{
public:
	CPowerSupplier(void);
	~CPowerSupplier(void);

	bool Open(int nGPIBBoard,int nAddr, bool bReset = true);
	void Close();

	bool PowerOn(bool bPort1 = true, bool bPort2 = true);
	bool PowerOff(bool bPort1 = true, bool bPort2 = true);

	bool SetOutputVolt(double fVolt, bool bPort2 = false);
	bool SetCurrentLimit(double fCurr, bool bPort2 = false);

	bool MeasureVolt(double &fVolt, bool bPort2 = false);
	bool MeasureCurr(double &fCurr, bool bPort2 = false);
	bool MeasureDVM(double &fDVM);

	bool DVMAvailable();
	bool DualPort();

protected:

	CGPIBDevice m_GPIBDevice;

	bool m_bKeithley;
	bool m_bDualport;
	bool m_bDVMAvailable;
};

