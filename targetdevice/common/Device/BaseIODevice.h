#pragma once

#include "..\..\EngineDLLAPI.h"
#include "..\Log.h"
#include "..\Common.h"

#define MAX_DEV_IDN_LEN 512

typedef enum
{
	DRV_COM = 0,
	DRV_GPIB,
	DRV_VISA
}IO_DRIVER;

typedef struct
{
	int nBus;
	int nPrimAddr;
	int nSecAddr;
	char sConnString[256];
	bool bPrimary;

	char sDevIdentity[MAX_DEV_IDN_LEN + 1];

	IO_DRIVER eType;
}IO_ADDRESS;

class CBaseIODevice
{
public:
	CBaseIODevice();
	virtual ~CBaseIODevice(void);

	void SetLogger(CLogger *p){ m_pLogger = p; };

	virtual bool Open(LPCTSTR strDevice) = 0;
	virtual bool Open(int a, int b, int c = 0) = 0;

	virtual bool Close() = 0;

	virtual bool Read(CString &strRead, int &nLen, int nTimeout) = 0;
	virtual bool Write(CString strWrite) = 0;
	virtual bool Query(CString strWrite, int &nLen, CString &strRead, int nTimeout) = 0;

	virtual bool Read(char *strRead, int &nLen, int nTimeout) = 0;
	virtual bool Write(const char *strWrite) = 0;
	virtual bool Query(const char *strWrite, int &nLen, char *sRead, int nTimeout) = 0;

	virtual bool Read(byte *strRead, int &nLen, int nTimeout) = 0;
	virtual bool Write(const byte *strWrite) = 0;
	virtual bool Query(const byte *strWrite, int &nLen, byte *sRead, int nTimeout) = 0;

	virtual bool ScanDevice(CArray<IO_ADDRESS> &addresses) = 0;

	CLogger *m_pLogger;

	void Log(LPCTSTR s);
};
