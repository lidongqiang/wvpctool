#ifndef _DEBUG_H_
#define _DEBUG_H_
#define LSHH_DEBUG
#if defined(LSHH_DEBUG)
    #define     DEBUG_WARN      TRUE
    #define     DEBUG_ERR       TRUE
    #define     DEBUG_WRITE     TRUE

    #define     DEBUG_LOG       TRUE
    #define     DEBUG_NONE      FALSE
    #define     DEBUG_CONFIG    FALSE
    #define     DEBUG_TARGET    TRUE
    #define     LDEGMSG(a)      if(m_pLog) m_pLog->PrintMSGA a
    #define     LDEGMSGW(a)     if(m_pLog) m_pLog->PrintMSGW a
#else 
    #define     DEBUG_WRITE     FALSE
    #define     DEBUG_ERR       FALSE
    #define     DEBUG_LOG       FALSE
    #define     DEBUG_WARN      FALSE
    #define     DEBUG_CONFIG    FALSE
    #define     DEBUG_NONE      FALSE
    #define     DEBUG_TARGET    FALSE
    #define     LDEGMSGW(cond,printf_exp)
    #define     LDEGMSG(cond,printf_exp)
#endif

class CLogger
{
public:
    typedef enum {
        DEBUG_ALL       = -1,        /* Debug level. Allows debug, info, warning and error traces */
        DEBUG_INFO      = 0,         /* information level. Default severity . Allows info, warning and error traces*/
        DEBUG_WARNING   = 1,         /* warning level. Allows warning and error traces*/
        DEBUG_ERROR     = 2,         /* error level. Allows only error traces*/
        DEBUG_DUT       = 3,
        DEBUG_OFF       = 4 ,         /* none level. No traces. Default when ipicom starts*/
    } DEBUG_LEVEL;
    friend 	CLogger *StartLog(LPCTSTR strPath, DEBUG_LEVEL debuglevel);
    CLogger(std::string &name);
    CLogger(std::wstring &name);
    ~CLogger();
private:
    DEBUG_LEVEL         m_curlevel;
    BOOL                m_bData;
    BOOL                m_bTime;
    /*std::string         m_name;
    std::string         m_path;
    **/
    std::wstring        m_namew;
    std::wstring        m_pathw;
    std::wstring        m_fullname;
    CRITICAL_SECTION    m_crit;
    char                m_bufa[1024];
    TCHAR               m_bufw[1024];
public:
    /*void SetDebugName(const char *); **/
    std::wstring GetModulePath();
    bool SetDebugName(const TCHAR *name);
    bool                IsPathFileExists(const TCHAR *strFilaName);
    bool CreateFolder(std::wstring strDir);
    void _cdecl PrintMSGW(DEBUG_LEVEL dlLevel,const TCHAR *format,...);
    void _cdecl PrintMSGA(DEBUG_LEVEL dlLevel,const CHAR  *format,...);
	static CLogger *StartLog(LPCTSTR strPath, DEBUG_LEVEL debuglevel);
    static std::wstring TimeStr(bool data,bool hms);
private:
    int  lock();
    int  unlock();
    bool lock_init();
    bool lock_deinit();
};
#endif
