#include "stdafx.h"
#include "CSerial.h"
/*******************************************************************************
*file       :CSerial.cpp
*synopsis   :A serial class for generally communicate 
*            Create by lanshh 2015-05-05 
*******************************************************************************/
BOOL Serial::Open   (UINT uPortID, UINT uBaud, UINT uParity, UINT uDatabits, UINT uStopbits)
{

    DCB     commParam;
    TCHAR   chPort[15] = {0};
    wsprintf(chPort,TEXT("\\\\.\\COM%d"),uPortID);
    m_hCom = CreateFile(chPort,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    if(m_hCom == INVALID_HANDLE_VALUE) {
        goto open_exit;
    }
    if(!GetCommState(m_hCom,&commParam)){
        goto open_exit;
    }
    commParam.BaudRate          = uBaud;
    commParam.fBinary           = TRUE;
    commParam.fParity           = TRUE;
    commParam.ByteSize          = uDatabits;
    commParam.Parity            = NOPARITY;
    commParam.StopBits          = uStopbits;
    commParam.fOutxCtsFlow      = FALSE;
    commParam.fOutxDsrFlow      = FALSE;
    commParam.fDtrControl       = DTR_CONTROL_ENABLE;
    commParam.fDsrSensitivity   = FALSE;
    commParam.fTXContinueOnXoff = TRUE;
    commParam.fOutX             = FALSE;
    commParam.fInX              = FALSE;
    commParam.fErrorChar        = FALSE;
    commParam.fNull             = FALSE;
    commParam.fRtsControl       = RTS_CONTROL_ENABLE;
    commParam.fAbortOnError     = FALSE;
    if(!SetCommState(m_hCom,&commParam)){
        goto open_exit;
    }
    COMMTIMEOUTS commtimeouts;
    GetCommTimeouts(m_hCom,&commtimeouts);
    commtimeouts.ReadIntervalTimeout            = 30;
    commtimeouts.ReadTotalTimeoutMultiplier     = 1;
    commtimeouts.ReadTotalTimeoutConstant       = 0;
    commtimeouts.WriteTotalTimeoutMultiplier    = 0;
    commtimeouts.WriteTotalTimeoutConstant      = 20000;
    if(!SetCommTimeouts(m_hCom,&commtimeouts)) {
        goto open_exit;
    }
    SetCommMask(m_hCom,EV_RXCHAR|EV_RXFLAG);
    SetupComm(m_hCom,512,512);
    PurgeComm(m_hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
    return TRUE;
open_exit:
    if(INVALID_HANDLE_VALUE != m_hCom) {
	    CloseHandle(m_hCom);
        m_hCom = INVALID_HANDLE_VALUE;
    }
    //LDEGMSGW(DEBUG_LOG, (TEXT("open(%s) failed\r\n"),chPort));
    return FALSE;
}
VOID Serial::Close  ()
{
	if(INVALID_HANDLE_VALUE != m_hCom) {
    	CloseHandle(m_hCom);
        m_hCom = INVALID_HANDLE_VALUE;
    }
}
LONG Serial::Read(char *pszBuf , int RecvCnt , int TimeOut)
{
    BOOL            fRet;
    LONG            i;
    LONG            iCnt;
    DWORD           iReadCnt;
    COMMTIMEOUTS    CommTimeOuts;
    DWORD           error;
    CommTimeOuts.ReadIntervalTimeout        = MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutMultiplier = MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutConstant   = TimeOut;
    SetCommTimeouts( m_hCom , &CommTimeOuts );
    i       = 0;
    iCnt    =(int)RecvCnt;
    do{
        ClearCommError(m_hCom,&error,NULL);
        fRet    = ReadFile( m_hCom , pszBuf+i , iCnt , &iReadCnt, NULL);
        i       += iReadCnt;
        iCnt    -= iReadCnt;
        if(!fRet)
            if(GetLastError() != ERROR_IO_INCOMPLETE) return i;
        if(fRet && !iReadCnt) return i;
    }while (iCnt > 0);
    return i;
}
BOOL Serial::Write(const char * buf,int buflen)
{
    DWORD       dwNumBytesWritten;
    if(NULL == buf|| m_hCom == INVALID_HANDLE_VALUE) {
        return FALSE;
	}
	PurgeComm(m_hCom,PURGE_RXCLEAR);
    WriteFile(m_hCom,buf,buflen,&dwNumBytesWritten,NULL);
    return TRUE;
}