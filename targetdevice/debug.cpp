#include "stdafx.h"
#include <string>
#include <errno.h>
#include "debug.h"
/*#include "../settings/config.h" **/
#define APP_VERSION "1.1.1"
#define dim(x) (sizeof(x) / sizeof(x[0]))
			                         //"19:02:10   App start..."
#define logmsga "DEBUG "APP_VERSION" "__DATE__" log Message\r\nTime       Message\r\n----------------------------------------------------------------------------------------------\r\n"

DWORD shWriteAsi(const TCHAR * tFileName,BYTE * pbWriteBuf,DWORD dwBufLen,DWORD dwWritePoint,DWORD dwTryTime)
{
    HANDLE              hFile;
    DWORD               dwWantToWriteCnt;
    DWORD               dwHasWriteCnt;
    DWORD               dwWriteCnt;
    int                 iTryTime;
    iTryTime            = dwTryTime;
    hFile               = CreateFile(tFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (INVALID_HANDLE_VALUE == hFile) {
        return -1;
    }
    if ( ERROR_ALREADY_EXISTS  != GetLastError()) {
        dwWantToWriteCnt = strlen(logmsga);
        WriteFile(hFile,logmsga,dwWantToWriteCnt,&dwWriteCnt,NULL);
    }
    if (0xFFFFFFFF == dwWritePoint) {
        SetFilePointer(hFile,0,NULL,FILE_END);
    } else {
        SetFilePointer(hFile,dwWritePoint,NULL,FILE_BEGIN);
    }
    dwWantToWriteCnt    = dwBufLen;
    dwHasWriteCnt       = 0;
    do  {
        dwWriteCnt = 0;
        if (!WriteFile(hFile,pbWriteBuf + dwHasWriteCnt,dwWantToWriteCnt,&dwWriteCnt,NULL)) {
            CloseHandle(hFile);
            return -1;
        }
        dwWantToWriteCnt -= dwWriteCnt; 
        dwHasWriteCnt += dwWriteCnt;
    } while (dwWantToWriteCnt > 0);
    CloseHandle(hFile);
    return 0;
}

int CLogger::lock()
{
  EnterCriticalSection(&m_crit);
  return 1;
}
int CLogger::unlock()
{
  LeaveCriticalSection(&m_crit);
  return 1;
}
bool CLogger::lock_init()
{
    InitializeCriticalSection(&m_crit);
    return true;
}
bool CLogger::lock_deinit()
{
    DeleteCriticalSection(&m_crit);
    return true;
}
CLogger::CLogger(std::string &name)
{
    /*m_name = name; **/
}
CLogger::CLogger(std::wstring &name)
{
    /*m_namew = name; **/
}
CLogger::~CLogger()
{
    lock_deinit();
}
std::wstring CLogger:: TimeStr(bool data,bool hms)
{
    TCHAR           strTemp[256]= {0};
    SYSTEMTIME      LocalTime;
    GetLocalTime( &LocalTime );
    if(data&&hms)  {
        wsprintf(strTemp, TEXT("%d-%02d-%02d_%02d.%02d.%02d.txt"),  
        LocalTime.wYear,LocalTime.wMonth, LocalTime.wDay,
        LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond );
    } else if (hms) {
        wsprintf(strTemp, TEXT("%02d.%02d.%02d.txt"),  
        LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond );
    } else if (data) {
        wsprintf(strTemp, TEXT("%d-%02d-%02d_%02d.%02d.%02d.txt"),  
        LocalTime.wYear,LocalTime.wMonth, LocalTime.wDay);
    }
    return strTemp;
}
CLogger *CLogger::StartLog(LPCTSTR strPath, CLogger::DEBUG_LEVEL debuglevel)
{
    bool ret = false;
    CLogger * p = new CLogger(std::wstring(L""));
    if(!p)  {
        goto startlogexit;
    }
    if(!p->lock_init())  {
        goto startlogexit;
    }
    p->m_curlevel = debuglevel;
    p->SetDebugName(strPath);
    ret = true;
startlogexit:
    if(ret) return p;
    if(p)   delete p;
    return NULL;
}
void _cdecl CLogger::PrintMSGW(DEBUG_LEVEL dlLevel,const TCHAR *format,...)
{
    va_list         args;
    SYSTEMTIME      LocalTime;
    INT             iLength;
    if (dlLevel < m_curlevel)   return;
    if (m_namew.empty())        return;
    lock();
    GetLocalTime( &LocalTime );
    iLength = wsprintf(m_bufw, TEXT("%02d:%02d:%02d   "), LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond );
    va_start(args, format);
    iLength += wvsprintf(&m_bufw[iLength], format, args);
    va_end(args);
    if(m_bufw[iLength] != TEXT('\n')) {
        m_bufw[iLength ++] =TEXT('\r');
        m_bufw[iLength ++] =TEXT('\n');
    }
    m_bufw[iLength] = TEXT('\0');
    WideCharToMultiByte(CP_ACP, 0, m_bufw, -1, m_bufa, dim(m_bufa), NULL, NULL);
#if 0
    OutputDebugStringA(m_bufa);
#endif
    if(!m_fullname.empty()) {
        shWriteAsi(m_fullname.c_str(),(BYTE *)m_bufa,strlen(m_bufa)*sizeof(m_bufa[0]),0xFFFFFFFF,0);
    }
    unlock();
}
void _cdecl CLogger::PrintMSGA(DEBUG_LEVEL dlLevel,const CHAR *format,...)
{
    va_list         args;
    SYSTEMTIME      LocalTime;
    INT             iLength;
    if (dlLevel < m_curlevel)   return;
    if (m_namew.empty())        return;
    lock();
    GetLocalTime( &LocalTime );
    iLength = sprintf_s( m_bufa,dim(m_bufa),"%02d:%02d:%02d   ", LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond );
    va_start(args, format);
    iLength += vsprintf_s(&m_bufa[iLength],dim(m_bufa) - iLength,format, args);
    va_end(args);
    if(m_bufa[iLength] != '\n') {
        m_bufa[iLength ++] ='\r';
        m_bufa[iLength ++] ='\n';
    }
    m_bufa[iLength]      = '\0';
#if 0
    OutputDebugStringA(m_bufa);
#endif
    if(!m_fullname.empty()) {
        shWriteAsi(m_fullname.c_str(),(BYTE *)m_bufa,strlen(m_bufa)*sizeof(m_bufa[0]),0xFFFFFFFF,0);
    }
    unlock();
}


bool CLogger::IsPathFileExists(const TCHAR *strFilaName)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFind;
    if(!strFilaName) return false;
    hFind = FindFirstFile(strFilaName, &FindFileData);
    if (INVALID_HANDLE_VALUE == hFind) {
        return false;
    }  else  {
        FindClose(hFind);
    }
    return true;
}
	
bool CLogger::CreateFolder(std::wstring strDir)
{
    bool         bSubCreate = false;
	std::wstring strSubDir;
    size_t       pos,start;
    start   = 0;
    /*remove the last backslash **/
    pos     = strDir.find_last_of (TEXT("\\/"),std::string::npos);
    if(strDir.size() == pos + 1) {
        strDir.erase(pos,std::string::npos);
    }
    pos     = strDir.find_first_of (TEXT("\\/"),0);
    while (std::string::npos != pos ) {
		strSubDir = strDir.substr (0,pos);
#if 1 
        if(!bSubCreate) {
            if(!IsPathFileExists(strSubDir.c_str())) {
                bSubCreate = true;
            }
        }
        if (bSubCreate) {
#else 
        if(!IsPathFileExists(strSubDir.c_str())) {
#endif
            if (!CreateDirectory(strSubDir.c_str(),NULL)) {
                return false;
			}
        }
		start   = pos+1;
        pos     = strDir.find_first_of(TEXT("\\/"),start);
    }
    if (!IsPathFileExists(strDir.c_str())) {
		if (!CreateDirectory(strDir.c_str(),NULL)) {
			return false;
		}
	}
	return true;
}
bool CLogger::SetDebugName(const TCHAR *name)
{
    std::wstring    path    = name;
    bool            ret     = true;
    lock();
    std::size_t pos = path.find_last_of(L"\\/");
    if( std::wstring::npos != pos) {
        m_namew = path.substr(pos);
        path.erase(pos + 1);
        bool bRet = CreateFolder(path);
        if( bRet ) {
            m_pathw = path;
        } else {
            m_pathw = GetModulePath();
        }
        m_fullname  = m_pathw + m_namew;
    } else { /*just file name */
        m_namew     = name;
        m_pathw     = GetModulePath();
        m_fullname  = m_pathw + m_namew;
        ret = true;
    }
    unlock();
    return ret;
}
std::wstring CLogger::GetModulePath()
{
	TCHAR path[260] = {0};
    std::wstring    strPath;
    std::size_t     pos;
	DWORD dwRet;
	dwRet = GetModuleFileName(NULL, path, dim(path));
	if (dwRet>0) {
        strPath.assign(path);
        pos = strPath.find_last_of(L"\\");
        if( std::wstring::npos != pos) {
            strPath.erase(pos + 1);
            return strPath;
        }
    }
	return TEXT("C:\\");
}
