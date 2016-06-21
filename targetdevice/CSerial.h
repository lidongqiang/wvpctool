#ifndef _CSERIAL_H_
#define _CSERIAL_H_
/*******************************************************************************
*file       :CSerial.h
*synopsis   :A serial class for generally communicate 
*            Create by lanshh 2015-05-05 
*******************************************************************************/
class Serial
{
public:
    Serial(){
        m_hCom = INVALID_HANDLE_VALUE;
    }
private:
    HANDLE m_hCom;
public:
    BOOL Open   (UINT uPortID, UINT uBaud, UINT uParity, UINT uDatabits, UINT uStopbits);
    VOID Close  ();
    LONG Read   (CHAR *pszBuf , int RecvCnt , int TimeOut);
    BOOL Write  (const char * buf   ,int buflen);
};

#endif



