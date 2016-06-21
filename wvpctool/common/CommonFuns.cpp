/******************************************************************************
*filename     :CommonFuns.cpp
*synopsis     :functions that can be used frequently definition
*created      :lanshh 2012-03-12
*last modified:2014-03-27 by lanshh 
******************************************************************************/
#include "stdafx.h"
#include "CommonFuns.h"
void AddStringToWindow(HWND hWnd,TCHAR *str)
{
    int             iTextLen;
    HWND            hWndShow;
    hWndShow        = hWnd;
    iTextLen    = (int)SendMessage(hWndShow,WM_GETTEXTLENGTH ,0,0);
    SendMessage(hWndShow,EM_SETSEL ,iTextLen,iTextLen);
    SendMessage(hWndShow,EM_REPLACESEL,(WPARAM)FALSE ,(LPARAM)str);
}
void ShowByteToDecimalStrW(HWND hwnd,TCHAR* Str,BYTE * DataBuf,int DataBufLength,int LineLen)
{
	TCHAR TShowString[512];
	TCHAR ShowSTR[10];
	if (NULL != Str)
	{
		wsprintf(TShowString,Str);
	}
	for(int j = 0; j < DataBufLength ; j++)
	{
		if ((j+1)%LineLen == 0 )  		
		{		
			wsprintf(ShowSTR,_T("%03d\r\n"),DataBuf[j]); 		
		}	
		else 		
		{
			wsprintf(ShowSTR,_T("%03d "),DataBuf[j]);
		}
		_tcscat(TShowString,ShowSTR);
		//wsprintf(TShowString,_T("%s%s"),TShowString,ShowSTR);
	}
	SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)TShowString);
}
void ShowByteTohexStrW(HWND hwnd,TCHAR* Str,BYTE * DataBuf,int DataBufLength,int LineLen)
{
	TCHAR TShowString[512];
	TCHAR ShowSTR[10];
	if (NULL != Str)
	{
		wsprintf(TShowString,Str);
	}
	for(int j = 0; j < DataBufLength ; j++)
	{
		if ((j+1)%LineLen == 0 )  		
		{		
			wsprintf(ShowSTR,_T("%02x\r\n"),DataBuf[j]); 		
		}	
		else 		
		{
			wsprintf(ShowSTR,_T("%02x "),DataBuf[j]);
		}
		_tcscat(TShowString,ShowSTR);
		//wsprintf(TShowString,_T("%s%s"),TShowString,ShowSTR);
	}
	SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)TShowString);
}
void ShowByteToDecimalStrA(HWND hwnd,CHAR* Str,BYTE * DataBuf,int DataBufLength,int LineLen)
{
	TCHAR TShowString[512];
	TCHAR ShowSTR[10];
	if (NULL != Str)
	{
		mbstowcs(TShowString,Str,512);
	}
	for(int j = 0; j < DataBufLength ; j++)
	{
		if ((j+1)%LineLen == 0 )  		
		{		
			wsprintf(ShowSTR,_T("%03d\r\n"),DataBuf[j]); 		
		}	
		else 		
		{
			wsprintf(ShowSTR,_T("%03d "),DataBuf[j]);
		}
		_tcscat(TShowString,ShowSTR);
	}
	SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)TShowString);
}
void ShowByteTohexStrA(HWND hwnd,CHAR* Str,BYTE * DataBuf,int DataBufLength,int LineLen)
{
	TCHAR TShowString[512];
	TCHAR ShowSTR[10];
	if (NULL != Str)
	{
		mbstowcs(TShowString,Str,512);
	}
	for(int j = 0; j < DataBufLength ; j++)
	{
		if ((j+1)%LineLen == 0 )  		
		{		
			wsprintf(ShowSTR,_T("%02x\r\n"),DataBuf[j]); 		
		}	
		else 		
		{
			wsprintf(ShowSTR,_T("%02x "),DataBuf[j]);
		}
		_tcscat(TShowString,ShowSTR);
	}
	SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)TShowString);
}
WCHAR *showbufferhex(BYTE * buf,int len)
{
    WCHAR       *szbuf;
    WCHAR       sztmp[8];
    int         j;
    int         linelen = 20;
    szbuf       = new WCHAR[len*2 + (len/linelen +1)*2 + 16 ];
    linelen     = 16;
    for(j = 0; j < len ; j++)
    {
        if ((j+1)%linelen == 0 )  		
        {		
            wsprintf(sztmp,_T("%02x\r\n"),buf[j]); 		
        }	
        else 		
        {
            wsprintf(sztmp,_T("%02x "),buf[j]);
        }
        _tcscat(szbuf,sztmp);

    }
    return szbuf;
}
void ShowMessageW(HWND hWnd,TCHAR *Message_1,TCHAR *Message_2,BYTE * DataBuf,int DataBufLength)
{
    DWORD dwStrLen;
	TCHAR TimeBuf[30];
	TCHAR *ShowStr;
    dwStrLen = wcslen(Message_1) + wcslen(Message_2) + DataBufLength*4 + 31;
    ShowStr = new TCHAR[dwStrLen];
	GetSysTimeSecW(TimeBuf,30);
	wsprintf(ShowStr,L"%s(%s)%s",Message_1,TimeBuf,Message_2);
    ByteToDecimalStrW(ShowStr,DataBuf,DataBufLength);
	SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)ShowStr);
    delete ShowStr;
}
void ShowMessageA(HWND hWnd,CHAR *Message_1,CHAR *Message_2,BYTE * DataBuf,int DataBufLength)
{
    DWORD dwStrLen;
	TCHAR TimeBuf[30];
	TCHAR *ShowStr;
    dwStrLen = strlen(Message_1) + strlen(Message_2) + DataBufLength*4 + 31;
    ShowStr = new TCHAR[dwStrLen];
    mbstowcs(ShowStr,Message_1,strlen(Message_1) + 1);
	GetSysTimeSecW(TimeBuf,30);
    _tcscat(ShowStr,TimeBuf);
    mbstowcs(&ShowStr[wcslen(ShowStr)],Message_2,strlen(Message_2) + 1);
	ByteToDecimalStrW(ShowStr,DataBuf,DataBufLength);
    SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)ShowStr);
    delete ShowStr;
}
int GetSysTimeSecW(TCHAR *Time,DWORD Len)
{
	SYSTEMTIME CollectTime;
	GetLocalTime(&CollectTime);
	wsprintf(Time,L"(%d:%d:%d)",CollectTime.wHour,CollectTime.wMinute,CollectTime.wSecond);
	return 0;
}
int GetSysTimeSecA(char *Time,DWORD Len)
{
	SYSTEMTIME CollectTime;
	GetLocalTime(&CollectTime);
	sprintf(Time,"(%d:%d:%d)",CollectTime.wHour,CollectTime.wMinute,CollectTime.wSecond);
	return 0;
}
int GetSysTimeDataSecW(TCHAR * buff,DWORD Len)
{
    SYSTEMTIME CollectTime;
    GetLocalTime(&CollectTime);
    swprintf_s(buff,Len,L"%04d-%02d-%02d",CollectTime.wYear,CollectTime.wMonth,CollectTime.wDay);
    return 0;
}
int GetSysTimeW(TCHAR * buff,DWORD Len)
{
	SYSTEMTIME CollectTime;
	GetLocalTime(&CollectTime);
	wsprintf(buff,L"(%d-%d-%d_%d-%d)", 
     CollectTime.wYear,CollectTime.wMonth,CollectTime.wDay,CollectTime.wHour,CollectTime.wMinute);
	return 0;
}
int GetSysTimeA(char * buff,DWORD Len)
{
	SYSTEMTIME CollectTime;
	GetLocalTime(&CollectTime);
	sprintf(buff,"(%d-%d-%d_%d-%d)",
		CollectTime.wYear,
		CollectTime.wMonth,
		CollectTime.wDay,
		CollectTime.wHour,
		CollectTime.wMinute);
	return 0;
}
int FindStrA(int StartPos,char * bRecieveBytes,int iBytesLength,char* CharStrToFind,int CharStrLength)
{
	BOOL bResult = TRUE;
	for (int i = StartPos; i< iBytesLength - CharStrLength + 1; i++ )
	{
		bResult = TRUE;
		for (int j = 0; j < CharStrLength; j++)
		{
			if (bRecieveBytes[i + j] != CharStrToFind[j] )
			{
				bResult = FALSE;
				break;
			}
		}
		if ( bResult )
		{
			return i;
		}
	}
	return -1;
}
int FindStrW(const int StartPos,const TCHAR * TSourceStr,const int iBytesLength,const TCHAR* TStrToFind,const int CharStrLength)
{
	BOOL bResult = TRUE;
	for (int i = StartPos; i< iBytesLength - CharStrLength + 1; i++ )
	{
		bResult = TRUE;
		for (int j = 0; j < CharStrLength; j++)
		{
			if (TSourceStr[i + j] != TStrToFind[j] )
			{
				bResult = FALSE;
				break;
			}
		}
		if ( bResult )
		{
			return i;
		}
	}
	return -1;
}
int ReverseFindStrW(const TCHAR * bRecieveBytes,int iBytesLength,const TCHAR* CharStrToFind,int CharStrLength)
{
	BOOL bResult = TRUE;
	if (0 >= CharStrLength)
	{
		return -1;
	}
	// end of the position ,the  CharStrLength can not be zero ,else int i = iBytesLength - CharStrLength will be error
	for (int i = iBytesLength - CharStrLength ; i>= 0; i-- )
	{
		bResult = TRUE;
		for (int j = 0; j < CharStrLength; j++)
		{
			if (bRecieveBytes[i + j] != CharStrToFind[j] )
			{
				bResult = FALSE;
				break;
			}
		}
		if ( bResult )
		{
			return i;
		}
	}
	return -1;
}
int ReverseFindStrA(CHAR * bRecieveBytes,int iBytesLength,CHAR* CharStrToFind,int CharStrLength)
{
	BOOL bResult = TRUE;
	if (0 >= CharStrLength)
	{
		return -1;
	}
	for (int i = iBytesLength - CharStrLength ; i>= 0; i-- )
	{
		bResult = TRUE;
		for (int j = 0; j < CharStrLength; j++)
		{
			if (bRecieveBytes[i + j] != CharStrToFind[j] )
			{
				bResult = FALSE;
				break;
			}
		}
		if ( bResult )
		{
			return i;
		}
	}
	return -1;
}
int GetRegistryDWS(HKEY HKEYLM,TCHAR *pSubKeysName,TCHAR **ppValueNames, DWORD*lpdwValueData,DWORD dwValuesCnt)
{
	DWORD dwDisposition ;
	HKEY hMainKey;
	long Result;
	DWORD dwItemsIndex;
	DWORD dwRegType;
	DWORD dwKValen;
	Result = RegCreateKeyEx(HKEYLM,
		pSubKeysName,0,NULL,
		REG_OPTION_NON_VOLATILE,NULL,NULL,&hMainKey,
		&dwDisposition);
	if ( ERROR_SUCCESS != Result)
	{
		return -1;
	}
	for (dwItemsIndex = 0;dwItemsIndex < dwValuesCnt;dwItemsIndex ++)
	{
		dwKValen = sizeof(DWORD);
		dwRegType = REG_DWORD;
		Result = RegQueryValueEx(hMainKey,ppValueNames[dwItemsIndex],
			NULL,&dwRegType,(BYTE *)&lpdwValueData[dwItemsIndex],
			&dwKValen);
		if ( ERROR_SUCCESS != Result)
		{
			if (ERROR_FILE_NOT_FOUND!= Result)
			{
				RegCloseKey(hMainKey);
				return -1;
			}
			lpdwValueData[dwItemsIndex] = 0;
		}
	}
	RegCloseKey(hMainKey);
	return 0;
}
int SetRegistryDWS(HKEY HKEYLM,TCHAR *pSubKeysName,TCHAR **ppValueNames, DWORD*lpdwValueData,DWORD dwValuesCnt)
{
	DWORD dwDisposition ;
	HKEY hMainKey;
	long Result;
	DWORD dwItemsIndex;
	Result = RegCreateKeyEx(HKEYLM,
		pSubKeysName,0,NULL,
		REG_OPTION_NON_VOLATILE,NULL,NULL,&hMainKey,
		&dwDisposition);
	if ( ERROR_SUCCESS != Result)
	{
		return -1;
	}
	for (dwItemsIndex = 0;dwItemsIndex < dwValuesCnt;dwItemsIndex ++)
	{
		Result = RegSetValueEx(hMainKey,ppValueNames[dwItemsIndex],
			NULL,REG_DWORD,(BYTE *)&lpdwValueData[dwItemsIndex],
			sizeof(DWORD));
		if ( ERROR_SUCCESS != Result)
		{
			return -1;
		}
	}
	RegCloseKey(hMainKey);
	return 0;
}
DWORD HexStringToDecimal(TCHAR * HexString,TCHAR *outdestr)
{
	DWORD dwReturnCode;
	int i,j;
	int Len;
	dwReturnCode = 0;
	Len = (int)wcslen(HexString);
	j = Len-1;
	if (7 < j)
	{
		j = 7;
	}
	for(i = 0;j>=0;i++,j--)
	{
// 		if(HexString[i]>='A'&&HexString[i]<='F')
// 			/*dwReturnCode +=(HexString[i]-'A'+10)*(pow(16,j));*/
// 			dwReturnCode +=(DWORD)((HexString[i]-'A'+10)*(pow(16,j)));
// 		if(HexString[i]>='a'&&HexString[i]<='f')
// 			dwReturnCode +=(DWORD)((HexString[i]-'a'+10)*(pow(16,j)));
// 		if(HexString[i]>='0'&& HexString[i]<='9')
// 			dwReturnCode +=(DWORD)((HexString[i]-'0')*(pow(16,j)));
	}
	if (NULL != outdestr )
	{
		wsprintf(outdestr,_T("%d"),dwReturnCode);
	}
	return dwReturnCode;
}
DWORD shCreateFile(TCHAR * tFileName)
{
        HANDLE              hFile;
        hFile               = CreateFile(tFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            return -1;
        }
        CloseHandle(hFile);
        return 0;
}
DWORD shWriteFile(TCHAR * tFileName,BYTE * pbWriteBuf,DWORD dwBufLen,DWORD dwWritePoint = 0,DWORD dwTryTime = 0)
{
	HANDLE hFile;
	DWORD dwWantToWriteCnt,dwHasWriteCnt,dwWriteCnt;
	int iTryTime;
	iTryTime = dwTryTime;
	dwWantToWriteCnt = dwBufLen;
	dwHasWriteCnt = 0;
	hFile = CreateFile(tFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return -1;
	}
	if (0xffffffff == dwWritePoint)
	{
		SetFilePointer(hFile,0,NULL,FILE_END);
	}
	else
	{
		SetFilePointer(hFile,dwWritePoint,NULL,FILE_BEGIN);
	}
	do 
	{
		dwWriteCnt = 0;
		if (!WriteFile(hFile,pbWriteBuf + dwHasWriteCnt,dwWantToWriteCnt,&dwWriteCnt,NULL))
		{
			CloseHandle(hFile);
			return -1;
		}
		dwWantToWriteCnt -= dwWriteCnt; 
		dwHasWriteCnt += dwWriteCnt;
	} while (dwWantToWriteCnt > 0);
	CloseHandle(hFile);
	return 0;
}
DWORD shGetFileZize(TCHAR * tFileName,BOOL * pbResult)
{
	HANDLE hFile;
	DWORD FileSize;
	hFile = CreateFile(tFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		if (NULL !=pbResult)
		{
			*pbResult = -1;
		}
		return 0xffffffff;
	}
	FileSize = GetFileSize(hFile,NULL);
	CloseHandle(hFile);
	return FileSize;
}
DWORD shReadFile(TCHAR * tFileName,BYTE * pbReadBuf,DWORD *pdwBufLen,DWORD dwReadPoint = 0,DWORD dwTryTime = 0)
{
	HANDLE hFile;
	DWORD dwWantToReadCnt,dwHasReadCnt,dwReadCnt;
	int iTryTime;
	iTryTime = dwTryTime;
	hFile = CreateFile(tFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return -1;
	}
	if (0xffffffff == dwReadPoint)
	{
		SetFilePointer(hFile,0,NULL,FILE_END);
	}
	else
	{
		SetFilePointer(hFile,dwReadPoint,NULL,FILE_BEGIN);
	}
	dwWantToReadCnt = *pdwBufLen;
	dwHasReadCnt = 0;
	do 
	{
		dwReadCnt = 0;
		if (!ReadFile(hFile,pbReadBuf + dwHasReadCnt,dwWantToReadCnt,&dwReadCnt,NULL))
		{
			CloseHandle(hFile);
			return -1;
		}
		dwWantToReadCnt -= dwReadCnt; 
		dwHasReadCnt += dwReadCnt;
	} while (dwWantToReadCnt > 0&&dwReadCnt != 0);
	CloseHandle(hFile);
	return 0;
}

void DrawComboBox(HWND hWnd,TCHAR *ShowStr,DWORD dwCnt,int iPlus,bool bPlus)
{
	DWORD dwIndex;
	DWORD dwMax;
	TCHAR ComboboxStr[64];
	dwMax = dwCnt -1 + iPlus;
	if ( NULL != ShowStr )
	{
		if (bPlus)
		{
			for (dwIndex = 0; dwIndex < dwCnt;dwIndex ++)
			{
				wsprintf(ComboboxStr,_T("%s%02X"),ShowStr,dwIndex+iPlus);
				SendMessage(hWnd,CB_INSERTSTRING,(WPARAM)dwIndex,(LPARAM)ComboboxStr);
			}
		}
		else
		{
			for (dwIndex = 0; dwIndex < dwCnt;dwIndex ++)
			{
				wsprintf(ComboboxStr,_T("%s%02X"),ShowStr,dwMax --);
				SendMessage(hWnd,CB_INSERTSTRING,(WPARAM)dwIndex,(LPARAM)ComboboxStr);
			}
		}
	}
	else
	{
		if (bPlus)
		{
			for (dwIndex = 0; dwIndex < dwCnt;dwIndex ++)
			{
				wsprintf(ComboboxStr,_T("%02X"),dwIndex+iPlus);
				SendMessage(hWnd,CB_INSERTSTRING,(WPARAM)dwIndex,(LPARAM)ComboboxStr);
			}
		}
		else
		{

			for (dwIndex = 0; dwIndex < dwCnt;dwIndex ++)
			{
				wsprintf(ComboboxStr,_T("%02X"),dwMax --);
				SendMessage(hWnd,CB_INSERTSTRING,(WPARAM)dwIndex,(LPARAM)ComboboxStr);
			}
		}
	}
}
DWORD HexStrToDword(TCHAR *pszWCHAR)
{
	int i,sum = 0;
	for(i = 0;pszWCHAR[i] != _T('\0');i++)
	{
		if(pszWCHAR[i] >=_T('0') && pszWCHAR[i]<=_T('9'))
			sum = sum*16 + pszWCHAR[i]-_T('0');
		else if(pszWCHAR[i]<= _T('f') && pszWCHAR[i]>=_T('a'))
			sum = sum*16 + pszWCHAR[i]-_T('a')+10;
		else if(pszWCHAR[i]<= _T('F') && pszWCHAR[i]>= _T('A'))
			sum = sum*16 + pszWCHAR[i]-_T('A')+10;
	}
	return sum;
}
//------------------------------ Added by lanshh ----------------------------------------
BOOL ByteToDecimalStrW(TCHAR* Str,BYTE * DataBuf,int DataBufLength)
{
	TCHAR ShowSTR[5];
	if (NULL == Str)
	{
		return FALSE;
	}
	for(int j = 0; j < DataBufLength ; j++)
	{
        wsprintf(ShowSTR,_T(" %03d"),DataBuf[j]); 		
		_tcscat(Str,ShowSTR);
	}
    return TRUE;
}


//------------------------------- Added End -----------------------------------





//------------------------------ Added by lanshh  2013-10-16 ----------------------------------------
BOOL  WcToUtf8( TCHAR wcInput,char *pcOutput,DWORD dwMaxOutBytes,DWORD *pdwBytesInUtf8 )
{

    DWORD           dwOutLen;
    BOOL            bRet;
    bRet            = TRUE;
    dwOutLen        = 0;
    if( pcOutput == NULL )
    {
        if( wcInput < 0x80 )
        {
            dwOutLen = 1;
        }
        else if( wcInput < 0x800 )
        {
            dwOutLen = 2;
        }
        else
        {
            dwOutLen = 3;
        }
        goto EXIT;
    }
    if( wcInput < 0x80 )
    {
        pcOutput[dwOutLen++] = (char)wcInput;
    }
    else if( wcInput < 0x800 )
    {
        if( dwMaxOutBytes < 2  )
        {
            bRet = FALSE;
            goto EXIT;
        }
        pcOutput[dwOutLen++] = 0xC0 | (char)(wcInput >> 6);
        pcOutput[dwOutLen++] = 0x80 | (char)(wcInput & 0x3F);
    }
    else
    {
        if( dwMaxOutBytes < 3  )
        {
            bRet = FALSE;
            goto EXIT;
        }        
        pcOutput[dwOutLen++] = 0xE0 | (char)(wcInput >> 12);
        pcOutput[dwOutLen++] = 0x80 | (char)((wcInput >> 6) & 0x3F);
        pcOutput[dwOutLen++] = 0x80 | (char)(wcInput & 0x3F);
    }

EXIT:
    *pdwBytesInUtf8 = dwOutLen;
    return bRet;
}

BOOL  Utf8ToWc( const char *pczInput,TCHAR *pwcOutput,DWORD *pdwUTF8CharLen )
{
    DWORD               dwUtfLength;
    unsigned char       ucFirstChar;
    unsigned char       ucOutMask;
    TCHAR               wcOutChar;
    DWORD               i;
    BOOL                bRet;
    bRet                = TRUE;
    dwUtfLength         = 0;
    if( pczInput == NULL )
    {
        //Input is NULL, make it an empty string
        pczInput = "";
    }
    ucFirstChar = (unsigned char)(*pczInput);
    if( ucFirstChar < 0x80 )
    {
        //Do quick completion of ASCII encoding
        dwUtfLength = 1;
        if( pwcOutput != NULL )
        {
            *pwcOutput = (TCHAR)ucFirstChar;
        }
        goto EXIT;
    }
    //Determine how long the UTF-8 sequence should be.
    if( (ucFirstChar & 0xE0) == 0xC0 )
    {
        if( (ucFirstChar & 0xFE) == 0xC0 )
        {
            //Overlong UTF-8 input
            bRet = FALSE;
            goto EXIT;
        }
        ucOutMask = 0x1F;
        dwUtfLength = 2;
    }
    else if( (ucFirstChar & 0xF0) == 0xE0 )
    {
        if( (ucFirstChar == 0xE0) && ((pczInput[1] & 0xE0) == 0x80) )
        {
            //Overlong UTF-8 input
            bRet = FALSE;
            goto EXIT;
        }
        ucOutMask = 0x0F;
        dwUtfLength = 3;
    }
    else
    {
        //First char did not match any valid value
        bRet = FALSE;
        goto EXIT;
    }
    //Mask out length indication bits of first char
    wcOutChar = (wchar_t)(ucFirstChar & ucOutMask);
    //Convert additional UTF-8 chars
    for( i=1; i<dwUtfLength; i++ )
    {
        if( (pczInput[i] & 0xC0) != 0x80 )
        {
            bRet = FALSE;
            dwUtfLength = 0;
            goto EXIT;
        }
        wcOutChar = wcOutChar << 6;
        wcOutChar |= (wchar_t)(pczInput[i] & 0x3F);
    }
    if( (wcOutChar == 0xFFFF) ||
        (wcOutChar == 0xFFFE) ||
        ( (wcOutChar >= 0xD800) && (wcOutChar <= 0xDFFF) ) )
    {
        //Invalid Unicode value detected.
        bRet = FALSE;
        dwUtfLength = 0;
        goto EXIT;
    }
    if( pwcOutput != NULL )
    {
        *pwcOutput = wcOutChar;
    }

EXIT:

    *pdwUTF8CharLen = dwUtfLength;
    return bRet;
}

BOOL WcsToUtf8( TCHAR *wcsInput,char *pczOutput,DWORD dwOutputSize,DWORD *pdwBytesWritten )
{
    DWORD           dwCharCount;
    DWORD           dwOutCount;
    DWORD           dwOutSizeLeft;
    BOOL            bRet;
    bRet            = TRUE;
    dwOutSizeLeft   = dwOutputSize - 1;
    dwOutCount      = 0;
    if( (dwOutputSize == 0) &&(pczOutput != NULL) )
    {
        bRet = FALSE;
        goto EXIT;
    }
    if( wcsInput == NULL )
    {
        wcsInput = L"";
    }
    while( (*wcsInput != L'\0') &&
        ((pczOutput == NULL) || (dwOutCount < (dwOutputSize-1))) &&
        (TRUE == bRet))
    {
        bRet = WcToUtf8( *wcsInput, pczOutput, dwOutSizeLeft, &dwCharCount );
        if( pczOutput != NULL )
        {
            pczOutput += dwCharCount;
        }
        wcsInput++;
        dwOutCount += dwCharCount;
        dwOutSizeLeft -= dwCharCount;
    }
    if( pczOutput != NULL )
    {
        *pczOutput = '\0';
    }

EXIT:
    *pdwBytesWritten = dwOutCount;
    return bRet;
}


BOOL Utf8ToWcs( char *pczInput,TCHAR *wcsOutput,DWORD dwOutputSize,DWORD *pdwWordsWritten )
{
    DWORD           dwOutCount;
    DWORD           dwCharCount;
    DWORD           dwOutputInc;
    BOOL            bRet;
    dwOutCount      = 0;
    dwOutputInc     = 1;
    bRet            = TRUE;
    if( (dwOutputSize == 0) &&(wcsOutput != NULL) )
    {
        bRet = FALSE;
        goto EXIT;
    }
    if( pczInput == NULL )
    {
        pczInput = "";
    }
    if( wcsOutput == NULL )
    {
        //Don't increment output ptr if it is == NULL
        dwOutputInc = 0;
    }
    while( (*pczInput != '\0') &&
        ((wcsOutput == NULL) || (dwOutCount < (dwOutputSize-1))) &&
        (TRUE == bRet))
    {
        bRet = Utf8ToWc( pczInput, wcsOutput, &dwCharCount );
        wcsOutput+= dwOutputInc;
        pczInput+= dwCharCount;
        dwOutCount += 1;
    }
    if( wcsOutput != NULL )
    {
        *wcsOutput = L'\0';
    }
EXIT:
    *pdwWordsWritten = dwOutCount;
    return bRet;
}
/******************************************************************************
*Function Name:void doFileEnumeration(LPSTR lpPath, BOOL bRecursion, 
               BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData)
*Synopsis     :Enumerate files from a specifyed folder ,
               when files find do call back 
*              Create by lanshh 2014-02-26 
*parameters   :
*              lpPath        -    Folder Name
*              bRecursion    -    whether find files in the subdirectories or not
*              bEnumFiles    -    just find files or include the direcories
*              pFunc         -    call back when find files or 
                                  directory(if bEnumFiles set false ,)
*              pUserData     -    the user data pass back to the call back as 
                                  the second parameters(pUserData)
*typedef BOOL (WINAPI *EnumerateFunc) (LPCTSTR lpFileOrPath, void* pUserData);
******************************************************************************/
BOOL FileEnum(LPTSTR lpPath, BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData)
{
    int                 cbpathlen;
    TCHAR               szPath[MAX_PATH];
    WIN32_FIND_DATA     fd;
    HANDLE              hFindFile;
    TCHAR               tempPath[MAX_PATH]; 
    BOOL                bIsDirectory;
    BOOL                bFinish     = FALSE;
    BOOL                busereturn  = TRUE;
    try {
        cbpathlen = _tcslen(lpPath);
        if(NULL == lpPath || 0 >= cbpathlen)  {
            return FALSE;
        }
        _tcscpy(szPath, lpPath);
        if(TEXT('\\') != lpPath[cbpathlen - 1] ) {
            _tcscat(szPath,TEXT("\\"));
        }
        _tcscat(szPath,TEXT("*"));
        hFindFile   = FindFirstFile(szPath, &fd);
        if( INVALID_HANDLE_VALUE == hFindFile ) {
                return FALSE;
        }
        while(!bFinish) {
            _tcscpy(tempPath,lpPath);
            if(TEXT('\\') != lpPath[cbpathlen - 1] )  {
                _tcscat(tempPath, TEXT("\\"));
            }
            _tcscat(tempPath,fd.cFileName);
            bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
            if( bIsDirectory&&(_tcscmp(fd.cFileName,TEXT("."))== 0|| _tcscmp(fd.cFileName, TEXT("..")) == 0))  {
                bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
                continue;
            }
            if(pFunc && bEnumFiles != bIsDirectory) {
                busereturn = pFunc(tempPath,bIsDirectory, pUserData);
                if( FALSE == busereturn ) {
                    FindClose( hFindFile );
                    return FALSE;
                }
            }
            if(bIsDirectory && bRecursion) {
                if (FALSE == FileEnum(tempPath, bRecursion, bEnumFiles, pFunc, pUserData)) {
                    FindClose( hFindFile );
                    return FALSE;
                }
            }
            bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
        }
        FindClose(hFindFile);
    }
    catch(...)
    { 
        return FALSE; 
    }
    return TRUE;
}
BOOL FolderExists(LPCTSTR lpPath)  
{  
    HANDLE              hFindFile;
    WIN32_FIND_DATA     fd      = {0};
    DWORD               dwLen;
    TCHAR               tempPath[MAX_PATH]; 
    if(!lpPath) return FALSE;
    dwLen               = _tcslen(lpPath);
    if(0 == dwLen)  return FALSE;
    _tcscpy(tempPath,lpPath);
    if(TEXT('\\') == tempPath[dwLen - 1] ) {
        tempPath[dwLen - 1] = TEXT('\0');
    }
    hFindFile   = FindFirstFile(tempPath, &fd);
    /*
    To examine a directory that is not a root directory, use the path to that directory, without a trailing backslash. 
    For example, an argument of "C:\Windows" returns information about the directory "C:\Windows",
    not about a directory or file in "C:\Windows". 
    To examine the files and directories in "C:\Windows", use an lpFileName of "C:\Windows\*".
    */
    if( INVALID_HANDLE_VALUE == hFindFile ) {
        return FALSE;
    }
    BOOL bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    FindClose( hFindFile );
    return bIsDirectory;    
}
/******************************************************************************
*Function Name:shWriteUniFile
*Synopsis     :save strings in a unicode file,if file existing,
*              then append at the end of file ,else create a new file
*              Create by lanshh 2013-06-26 
*parameters   :
*               tFileName       - file name 
*               pbWriteBuf      -
*               dwBufLen        - 
*               dwWritePoint    - 
*               dwTryTime       - 
******************************************************************************/

DWORD shWriteUniFile(TCHAR * tFileName,BYTE * pbWriteBuf,DWORD dwBufLen,DWORD dwWritePoint,DWORD dwTryTime)
{
    HANDLE              hFile;
    DWORD               dwWantToWriteCnt;
    DWORD               dwHasWriteCnt;
    DWORD               dwWriteCnt;
    int                 iTryTime;
    BYTE                bMagic[2];
    iTryTime            = dwTryTime;
    hFile               = CreateFile(tFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return -1;
    }
    if ( ERROR_ALREADY_EXISTS  != GetLastError())
    {
        bMagic[0] = 0xFF;
        bMagic[1] = 0xFE;
        dwWantToWriteCnt = 2;
        WriteFile(hFile,bMagic,dwWantToWriteCnt,&dwWriteCnt,NULL);
    }
    if (0xFFFFFFFF == dwWritePoint)
    {
        SetFilePointer(hFile,0,NULL,FILE_END);
    }
    else
    {
        SetFilePointer(hFile,dwWritePoint,NULL,FILE_BEGIN);
    }
    dwWantToWriteCnt    = dwBufLen;
    dwHasWriteCnt       = 0;
    do 
    {
        dwWriteCnt = 0;
        if (!WriteFile(hFile,pbWriteBuf + dwHasWriteCnt,dwWantToWriteCnt,&dwWriteCnt,NULL))
        {
            CloseHandle(hFile);
            return -1;
        }
        dwWantToWriteCnt    -= dwWriteCnt; 
        dwHasWriteCnt       += dwWriteCnt;
    } while (dwWantToWriteCnt > 0);
    CloseHandle(hFile);
    return 0;
}
/******************************************************************************
*Function Name:shWriteAnsiFile
*Synopsis     :save strings in a ansi file,if file existing,then 
               append at the end of file
*              else create a new file
*              Create by lanshh 2014-03-29
*parameters   :
*              tFileName       - file name 
*              pbWriteBuf      -
*              dwBufLen        - 
*              dwWritePoint    - 
*              dwTryTime       - 
******************************************************************************/
DWORD shWriteAnsiFile(TCHAR * tFileName,BYTE * pbWriteBuf,DWORD dwBufLen,DWORD dwWritePoint ,DWORD dwTryTime )
{
    HANDLE              hFile;
    DWORD               dwWantToWriteCnt;
    DWORD               dwHasWriteCnt;
    DWORD               dwWriteCnt;
    int                 iTryTime;
    iTryTime            = dwTryTime;
    hFile               = CreateFile(tFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return -1;
    }
    if (0xffffffff == dwWritePoint)
    {
        SetFilePointer(hFile,0,NULL,FILE_END);
    }
    else
    {
        SetFilePointer(hFile,dwWritePoint,NULL,FILE_BEGIN);
    }
    dwWantToWriteCnt    = dwBufLen;
    dwHasWriteCnt       = 0;
    do 
    {
        dwWriteCnt = 0;
        if (!WriteFile(hFile,pbWriteBuf + dwHasWriteCnt,dwWantToWriteCnt,&dwWriteCnt,NULL))
        {
            CloseHandle(hFile);
            return -1;
        }
        dwWantToWriteCnt    -= dwWriteCnt; 
        dwHasWriteCnt       += dwWriteCnt;
    } while (dwWantToWriteCnt > 0);
    CloseHandle(hFile);
    return 0;
}
BOOL InflateRect( RECT*lprc, int l,int t,int r,int b)
{
    lprc->left      -= l;
    lprc->right     += r;
    lprc->top       -= t;
    lprc->bottom    += b;
    if(lprc->left < 0) lprc->left   = 0;
    if(lprc->top < 0)  lprc->top    = 0;
    if(lprc->left > lprc->right ) lprc->right   = lprc->left + 1;
    if(lprc->top > lprc->bottom ) lprc->bottom  = lprc->top + 1;
    return TRUE;
}
int WCharToChar(CHAR *pszDst,int nDstSize,const WCHAR *pszSrc)
{
	CHAR    *pszOut = NULL;
	int     nOutSize;
	if (!pszSrc) {
		return 0;
	}
	if ((NULL == pszDst)&&(0 != nDstSize)) {
        return 0;
    } else if (pszDst&&(0 == nDstSize)) {
        return 0;
    }
    /*If the function succeeds and cbMultiByte is 0, the return value is the required size*/
    nOutSize = WideCharToMultiByte(CP_ACP, 0,pszSrc, -1,NULL, 0/*cbMultiByte*/, NULL, NULL) /*+ 1*/;
    if( 0 == nDstSize) {
        /*Calculate the requires size*/
        return nOutSize;
    }
    if(nOutSize > nDstSize) {
        return 0;
    }
    nOutSize = WideCharToMultiByte(CP_ACP, 0,pszSrc,-1,pszDst,nDstSize,NULL,NULL);
    return nOutSize;
}
int CharToWChar(WCHAR *pszDst,int nDstSize,const CHAR *pszSrc)
{
    WCHAR   *pszOut = NULL;
    int     nOutSize;
	if (!pszSrc) {
		return 0;
	}
	if ((NULL == pszDst)&&(0 != nDstSize)) {
        return 0;
    } else if (pszDst&&(0 == nDstSize)) {
        return 0;
    }
    nOutSize = MultiByteToWideChar(CP_ACP, 0,pszSrc,-1,NULL,0);
    if(0 == nDstSize) {
        /*Calculate the requires size*/
        return nOutSize;
    }
    if(nOutSize > nDstSize) {
        return NULL;
    }
    nOutSize = MultiByteToWideChar(CP_ACP, 0,pszSrc,-1,pszDst,nDstSize);
    return nOutSize;
}
#if 0
VOID WalkMenu(CMenu *pMenu,CString strMainKeyPart)
{
    CString strKey;
    int     id ;
    UINT    i;
    if(NULL == pMenu) return ;
    for (i=0;i < pMenu->GetMenuItemCount();i++) {
        strKey.Format(_T("%s_%d"),strMainKeyPart,i);
        id = pMenu->GetMenuItemID(i);
        if (0 == id) { 
        /*If nPos corresponds to a SEPARATOR menu item, the return value is 0. **/
        } else if (-1 == id) { 
        /*If the specified item is a pop-up menu (as opposed to an item within the pop-up menu), the return value is ¨C1 **/
            pMenu->ModifyMenu(i, MF_BYPOSITION, i, GetLocalString(strKey));
        	WalkMenu(pMenu->GetSubMenu(i),strKey);
        } else {
            pMenu->ModifyMenu(id, MF_BYCOMMAND, id, GetLocalString(strKey));
        }
    }
}
#endif

#if 0
BOOL MoveWindow(HWND, RECT &rt, BOOL bRepaint);
{
    return MoveWindow(g_hTaskWnd, 0, 0, nWidth, nHeight , TRUE);
}
#endif
