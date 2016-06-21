#ifndef _SCRIPT_H_
#define _SCRIPT_H_
#include "string"
typedef bool (ReadCallBack)(void*,char *,int size);
class CSpawn
{
    #define TIME_FOR_READ 100
public:
    CSpawn(){
        bSync               = true;
        m_ncBuf             = 0;
        m_nExitCode         = -1;
        m_hExitEvent        = NULL;
        m_hChildStd_IN_Rd   = m_hChildStd_IN_Wr = NULL;
        m_hChildStd_OUT_Wr  = m_hChildStd_OUT_Rd= NULL;
        m_hProcess          = NULL;
        m_hReadThread       = NULL;
        m_UserData          = NULL;
        m_CallBack          = NULL;
    };
    ~CSpawn(){
        if(!bSync) {
            WaitForExit(1000);
        }
    }
private:
    unsigned int    m_nExitCode;
    bool            WaitForExit         (int nTimeOut);
public:
    unsigned int    GetResult           ();
    void            SetResult           (unsigned int v);
    char            *GetResultStr       ()
    {
        if(m_ncBuf > 0&& m_ncBuf <sizeof(m_ReadBuf)/sizeof(m_ReadBuf[0])) {
            m_ReadBuf[m_ncBuf] = '\0';
            return m_ReadBuf;
        } else {
            return NULL;
        }
    }
    void            SetCallBack         (ReadCallBack* v,void* data);
    bool            Exe                 (std::wstring szCmd,int nTimeOut,bool sync);
    bool            ExeArg              (int nTimeOut,bool sync,wchar_t * format,...);
    bool            CreateChildProcess  ();
    bool            InitEnv             ();
    bool            DeInitEnv           ();
    unsigned int    ReadOutputThread    ();
    bool            CallBack            (char *buf,int size);
    bool            bSync;
    ReadCallBack    *m_CallBack;
    void            *m_UserData;
    std::wstring    m_strCmd;
    char            m_ReadBuf[40960];
    unsigned int    m_ncBuf;

    /*win32 **/
    static  UINT    ReadPipeThreadProc  (LPVOID pParam);
    DWORD           m_dwThreadId;
    DWORD           m_dwProcessId;
    HANDLE          m_hProcess;
    HANDLE          m_hReadThread;
    DWORD           m_dwReadThreadId;
    HANDLE          m_hRead;
    HANDLE          m_hWrite;   
    HANDLE          m_hExitEvent;
    HANDLE          m_hChildStd_IN_Rd;
    HANDLE          m_hChildStd_IN_Wr;
    HANDLE          m_hChildStd_OUT_Rd;
    HANDLE          m_hChildStd_OUT_Wr;

};
#endif
