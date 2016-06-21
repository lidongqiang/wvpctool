/******************************************************************************
*filename     :CommonFuns.h
*synopsis     :functions that can be used frequently statement
*created      :lanshh 2012-03-12
*last modified:2014-03-27 by lanshh 
******************************************************************************/
#pragma once
/******************************************************************************
*Function Name:void doFileEnumeration(LPSTR lpPath, BOOL bRecursion, 
*              BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData)
*Synopsis     :Enumerate files from a specifyed folder ,when files find do 
*              call back Create by lanshh 2014-02-26 
*parameters   :
*              lpPath        -    Folder Name
*              bRecursion    -    whether find files in the subdirectories or not
*              bEnumFiles    -    just find files or include the direcories
*              pFunc         -    call back when find files or directory
                                  (if bEnumFiles set false ,)
*              pUserData     -    the user data pass back to the 
                                  call back as the second parameters(pUserData)
*typedef BOOL (WINAPI *EnumerateFunc) (LPCTSTR lpFileOrPath, void* pUserData);
******************************************************************************/
typedef BOOL (WINAPI *EnumerateFunc) (LPCTSTR lpFileOrPath,BOOL bEnumFiles,void* pUserData);
BOOL FileEnum(LPTSTR lpPath, BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData);
void ShowByteToDecimalStrW(HWND hwnd,TCHAR* Str,BYTE * DataBuf,int DataBufLength,int LineLen);
void ShowByteToDecimalStrA(HWND hwnd,CHAR * Str,BYTE * DataBuf,int DataBufLength,int LineLen);
void ShowByteTohexStrW(HWND hwnd,TCHAR* Str,BYTE * DataBuf,int DataBufLength,int LineLen);
void ShowByteTohexStrA(HWND hwnd,CHAR* Str,BYTE * DataBuf,int DataBufLength,int LineLen);
void ShowMessageW(HWND hWnd,TCHAR *Message_1,TCHAR *Message_2,BYTE * DataBuf,int DataBufLength);
void ShowMessageA(HWND hWnd,CHAR *Message_1,CHAR *Message_2,BYTE * DataBuf,int DataBufLength);
int GetSysTimeDataSecW(TCHAR * buff,DWORD Len);
int GetSysTimeSecW(TCHAR *Time,DWORD Len);
int GetSysTimeSecA(CHAR  *Time,DWORD Len);
int GetSysTimeW(TCHAR * buff,DWORD Len);
int GetSysTimeA(CHAR  * buff,DWORD Len);
int FindStrW(const int StartPos,const TCHAR * TSourceStr,const int iBytesLength,const TCHAR* TStrToFind,const int CharStrLength);
int FindStrA(int StartPos,CHAR  * TSourceStr,int iBytesLength,CHAR * TStrToFind,int CharStrLength);
int ReverseFindStrW(const TCHAR * bRecieveBytes,int iBytesLength,const TCHAR* CharStrToFind,int CharStrLength);
int ReverseFindStrA(CHAR  * bRecieveBytes,int iBytesLength,CHAR * CharStrToFind,int CharStrLength);
int GetRegistryDWS(HKEY HKEYLM,TCHAR *pSubKeysName,TCHAR **ppValueNames, DWORD*lpdwValueData,DWORD dwValuesCnt);
int SetRegistryDWS(HKEY HKEYLM,TCHAR *pSubKeysName,TCHAR **ppValueNames, DWORD*lpdwValueData,DWORD dwValuesCnt);
DWORD HexStringToDecimal(TCHAR * HexString,TCHAR *outdestr);
DWORD shCreateFile(TCHAR * tFileName);
DWORD shWriteFile(TCHAR * tFileName,BYTE * pbWriteBuf,DWORD dwBufLen,DWORD dwWritePoint ,DWORD dwTryTime );
/******************************************************************************
*Function Name:DWORD HexStrToDword(TCHAR *pszWCHAR);
*Synopsis     :convert a hex string to DWORD Create by lanshh 2013-05-04
*parameters   :
*              pszWCHAR        -    null terminate string that shall be convert 
*return       :the string value in DWORD
******************************************************************************/
DWORD HexStrToDword(TCHAR *pszWCHAR);
/******************************************************************************
*Function Name:BOOL ByteToDecimalStrW(TCHAR* Str,BYTE * DataBuf,int DataBufLength);
*Synopsis     :convert bytes to a string that contains value of bytes  
*              Create by lanshh 2013-05-04
*parameters   :
*              Str           -   out,string tha contain the bytes value,the string 
*                                must be long enough to cantain the bytes value string
*              DataBuf       -   in,bytes to be converted
*              DataBufLength -   in,byte count in the DataBuf
*return       :the string value in DWORD
******************************************************************************/
BOOL ByteToDecimalStrW(TCHAR* Str,BYTE * DataBuf,int DataBufLength);
/******************************************************************************
*Function Name:BOOL Utf8ToWcs( char *pczInput,TCHAR *wcsOutput,
*              DWORD dwOutputSize,DWORD *pdwWordsWritten );
*Synopsis     :convert a utf_8 string to wide character string
*              Added by lanshh  2013-10-16 
*parameters   :
*              pczInput         -   
*              wcsOutput        - 
*              dwOutputSize     - 
*              pdwWordsWritten  - 
*
*return       :success if TRUE
******************************************************************************/
BOOL Utf8ToWcs( char *pczInput,TCHAR *wcsOutput,DWORD dwOutputSize,DWORD *pdwWordsWritten );
/******************************************************************************
*Function Name:BOOL WcsToUtf8( TCHAR *wcsInput,char *pczOutput,
*              DWORD dwOutputSize,DWORD *pdwBytesWritten );
*Synopsis     :convert  wide character string to utf_8 string
*              Added by lanshh  2013-10-16 
*parameters   :
*              wcsInput         -   
*              pczOutput        - 
*              dwOutputSize     - 
*              pdwBytesWritten  - 
*
*return       :success if TRUE
******************************************************************************/
BOOL WcsToUtf8( TCHAR *wcsInput,char *pczOutput,DWORD dwOutputSize,DWORD *pdwBytesWritten );
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
DWORD shWriteUniFile(TCHAR * tFileName,BYTE * pbWriteBuf,DWORD dwBufLen,DWORD dwWritePoint,DWORD dwTryTime);
/******************************************************************************
*Function Name:shWriteAnsiFile
*Synopsis     :save strings in a ansi file,if file existing,then append 
*              at the end of file ,else create a new file
*              Create by lanshh 2014-03-29
*parameters   :
*               tFileName       - file name 
*               pbWriteBuf      -
*               dwBufLen        - 
*               dwWritePoint    - 
*               dwTryTime       - 
******************************************************************************/
DWORD shWriteAnsiFile(TCHAR * tFileName,BYTE * pbWriteBuf,DWORD dwBufLen,DWORD dwWritePoint = 0,DWORD dwTryTime = 0);
/******************************************************************************
*Function Name:void AddStringToWindow(HWND hWnd,TCHAR *str);
*Synopsis     : Add string to the windows
*              Create by lanshh 2014-04-14
******************************************************************************/
void AddStringToWindow(HWND hWnd,TCHAR *str);
BOOL InflateRect( RECT*lprc, int l,int t,int r,int b);
BOOL FolderExists(LPCTSTR lpPath);
int  CharToWChar(WCHAR *pszDst,int nDstSize,const CHAR *pszSrc);
int  WCharToChar(CHAR *pszDst,int nDstSize,const WCHAR *pszSrc);

