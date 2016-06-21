#include "stdafx.h"
#include "scriptexe.h"

bool CSpawn::Exe(std::wstring szCmd,int nTimeOut,bool sync)
{
    bSync       = sync;
    m_strCmd    = szCmd;
    if(!InitEnv()){
        return false;
    }
    m_hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadPipeThreadProc, this, 0, &m_dwReadThreadId);
    if(NULL == m_hReadThread) {
        goto exe_exit;
    }
    if(!CreateChildProcess()) {
        goto exe_exit;
    }
    if(bSync) {
        WaitForExit(nTimeOut);
        DeInitEnv();
    }
    return true;
exe_exit:
    WaitForExit(nTimeOut);
    if(NULL != m_hReadThread) {
        SetEvent(m_hExitEvent);
        if(WAIT_OBJECT_0 != WaitForSingleObject(m_hReadThread, TIME_FOR_READ*10)) {
            TerminateThread(m_hReadThread, 0);
        }
        m_hReadThread = NULL;
    }
    DeInitEnv();
    SetResult(-1);
    return false;
}
bool CSpawn::ExeArg(int nTimeOut,bool sync,wchar_t * format,...)
{
    wchar_t     strCmd[260] = {0};  
    va_list     args;
    va_start(args, format);
    wvsprintf(strCmd,format, args);
    bSync       = sync;
    m_strCmd    = strCmd;
    if(!InitEnv()){
        return false;
    }
    m_hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadPipeThreadProc, this, 0, &m_dwReadThreadId);
    if(NULL == m_hReadThread) {
        goto exe_exit;
    }
    if(!CreateChildProcess()) {
        goto exe_exit;
    }
    if(bSync) {
        WaitForExit(nTimeOut);
        DeInitEnv();
    }
    return true;
exe_exit:
    WaitForExit(nTimeOut);
    if(NULL != m_hReadThread) {
        SetEvent(m_hExitEvent);
        if(WAIT_OBJECT_0 != WaitForSingleObject(m_hReadThread, TIME_FOR_READ*10)) {
            TerminateThread(m_hReadThread, 0);
        }
        m_hReadThread = NULL;
    }
    DeInitEnv();
    SetResult(-1);
    return false;
}
/*always exit **/
bool CSpawn::WaitForExit(int nTimeOut)
{
    DWORD dwExitCode;
    DWORD dwWaitResult;
    if(NULL == m_hProcess ) return false;
    dwWaitResult = WaitForSingleObject(m_hProcess, nTimeOut);
    CloseHandle(m_hChildStd_OUT_Wr);
    m_hChildStd_OUT_Wr = NULL;
    if(WAIT_OBJECT_0 != dwWaitResult ) {
        TerminateProcess(m_hProcess,-1);
        WaitForSingleObject(m_hProcess, INFINITE);
    }
    if (GetExitCodeProcess(m_hProcess, &dwExitCode)) {
        SetResult(dwExitCode);
    } else {
        SetResult(-1);
    }
    CloseHandle(m_hProcess);
    m_hProcess = NULL;
    SetEvent(m_hExitEvent);
    if(WAIT_OBJECT_0 != WaitForSingleObject(m_hReadThread, TIME_FOR_READ*10)) {
        TerminateThread(m_hReadThread, 0);
    }
    m_hReadThread = NULL;
    return true;
}
bool CSpawn::CreateChildProcess() 
{
    STARTUPINFO         siStartInfo = {0}; 
    PROCESS_INFORMATION piProcInfo  = {0};
    BOOL                bSuccess    = FALSE; 
    TCHAR               shellCmd[4096];
#if _MSC_VER>1200
    _tcscpy_s(shellCmd,m_strCmd.c_str());
#else
    _tcscpy  (shellCmd,m_strCmd.c_str());
#endif
    siStartInfo.cb          = sizeof(STARTUPINFO); 
    GetStartupInfo(&siStartInfo);
    siStartInfo.hStdInput   = m_hChildStd_IN_Rd;
    siStartInfo.hStdOutput  = m_hChildStd_OUT_Wr;
    siStartInfo.hStdError   = m_hChildStd_OUT_Wr;
    siStartInfo.wShowWindow = SW_HIDE; 
    siStartInfo.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    bSuccess  = CreateProcess(NULL, 
        shellCmd,           /*applicatin name                    **/
        NULL,               /*process security attributes        **/
        NULL,               /*primary thread security attributes **/
        TRUE,               /*handles are inherited              **/
        CREATE_NEW_CONSOLE, /*creation flags                     **/
        NULL,               /*use parent's environment           **/
        NULL,               /*use parent's current directory     **/
        &siStartInfo,       /*STARTUPINFO pointer                **/
        &piProcInfo);       /*receives PROCESS_INFORMATION       **/
    if(bSuccess ) {
        m_hProcess    = piProcInfo.hProcess;
        m_dwProcessId = piProcInfo.dwProcessId;
    }
    return bSuccess ?true:false;
}
bool CSpawn::DeInitEnv()
{
    if(NULL != m_hExitEvent) {
        CloseHandle(m_hExitEvent );
        m_hExitEvent        = NULL;
    }
    if(NULL != m_hExitEvent){
        CloseHandle(m_hChildStd_OUT_Rd);
        m_hChildStd_OUT_Rd= NULL;
    }
    if(NULL != m_hExitEvent) {
        CloseHandle(m_hChildStd_OUT_Wr);
        m_hChildStd_OUT_Wr  = NULL;
    }
    if(NULL != m_hExitEvent) {
        CloseHandle(m_hChildStd_IN_Rd);
        m_hChildStd_IN_Rd   = NULL;
    }
    if(NULL != m_hExitEvent) {
        CloseHandle(m_hChildStd_IN_Wr);
        m_hChildStd_IN_Wr = NULL;
    }
    return true;
}
bool CSpawn::InitEnv()
{
    bool cpo = false;
    bool cpi = false;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength              = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle       = TRUE; /*can be inherited when a new process is created **/
    saAttr.lpSecurityDescriptor = NULL;
    if (::CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0) ) {
        cpo = true; /*tag if fail occur,clean**/
        if(!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)){
            goto createpipe_exit;
        }
    } else {
        goto createpipe_exit;
    }
    if (::CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0) ) {
        cpi = true;  /*tag if fail occur,clean**/
        if(!SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)){
            goto createpipe_exit;
        }
    } else {
        goto createpipe_exit;
    }
    m_hExitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if(NULL == m_hExitEvent) {
        goto createpipe_exit;
    }
    memset(m_ReadBuf,0,sizeof(m_ReadBuf));
    m_ncBuf     = 0;
    m_nExitCode = -1;
    return true;
createpipe_exit:
    if(NULL != m_hExitEvent) {
        CloseHandle(m_hExitEvent);
        m_hExitEvent = NULL;
    }
    if(cpo) {
        CloseHandle(m_hChildStd_OUT_Rd);
        CloseHandle(m_hChildStd_OUT_Wr);
        m_hChildStd_OUT_Wr  = m_hChildStd_OUT_Rd= NULL;
    }
    if(cpi) {
        CloseHandle(m_hChildStd_IN_Rd);
        CloseHandle(m_hChildStd_IN_Wr);
        m_hChildStd_IN_Rd   = m_hChildStd_IN_Wr = NULL;
    }
    return false;
}
UINT CSpawn::ReadPipeThreadProc(LPVOID pParam)
{
    CSpawn * p = (CSpawn*)pParam;
    if(p ) return p->ReadOutputThread();
    return 0;
}
unsigned int CSpawn::ReadOutputThread()
{
    DWORD       dwWaitRet;
    DWORD       bytesRead;
    char        rev[4096];
    DWORD       leftLen;
    BYTE        *pBuf       = (BYTE*)m_ReadBuf;;
    HANDLE      hRead       = m_hChildStd_OUT_Rd;
    leftLen     = sizeof(m_ReadBuf);
    m_ncBuf                 = 0;
    memset(m_ReadBuf,0,sizeof(m_ReadBuf));
    while(1) {
        dwWaitRet = WaitForSingleObject(m_hExitEvent,TIME_FOR_READ);
        /*always reading **/
        memset(rev, 0, sizeof(rev));
        if ( ReadFile(hRead,rev,4095,&bytesRead,NULL)) {
            DWORD cpyLen = leftLen > bytesRead ?bytesRead:leftLen;
            if(bSync&&(0<cpyLen)) {
                memcpy(pBuf + m_ncBuf,rev,cpyLen);
                m_ncBuf += cpyLen;
                leftLen -= cpyLen;
            }
            CallBack(rev,bytesRead);
        } else {
            if(ERROR_BROKEN_PIPE == GetLastError()) {
                /*
                If an anonymous pipe is being used and the write handle has been closed
                when ReadFile attempts to read using the pipe's corresponding read handle
                the function returns FALSE 
                **/
            }
            /*read fail **/
            break;
        }
        if(0 == leftLen) {
            /*buf used up **/
            /*break; **/
        }
        if(WAIT_TIMEOUT != dwWaitRet) { /* WAIT_OBJECT_0 **/
            break;
        }
    }
    return 0;
}

bool CSpawn::CallBack(char *buf,int size) 
{
    if(m_CallBack) {
        m_CallBack(m_UserData,buf,size);
        return true;
    }
    return false;
}
unsigned int CSpawn::GetResult()
{
    return m_nExitCode;
}
void CSpawn::SetResult(unsigned int v)
{
    m_nExitCode = v ;
}
void CSpawn::SetCallBack(ReadCallBack* v,void* data)
{
    m_UserData = data;
    m_CallBack = v;
}