#ifndef _SETTINGBASE_H_
#define _SETTINGBASE_H_
#define USER_LOGIN
#define APP_VERSION " V1.1.2"
#include "./inifile.h"
#include <vector>
#define LAN_ACTION TRUE
#define nof(x) (sizeof(x) / sizeof(x[0]))
#define MASK_FUSE  1
#define MASK_WV    2
#define MASK_BASIC 3

/********** FLAG_TYPE ***********/
#define FLAG_SN                 0 /*must be zero **/
#define FLAG_MAC                1
#define FLAG_WIFIMAC            1
#define FLAG_BTMAC              2
#define FLAG_IMEI1              3
#define FLAG_IMEI2              4
#define FLAG_CNT                5
#define FLAG_AUTO               6


#define FLAG_DEVSN				0  /*must be zero **/
#define FLAG_WVKEY 				1
#define FLAG_WVFLG				2
#define FLAG_WOID               3
#define FLAG_WVCNT				4

#define FLAG_UNSD               TEXT("0")
#define FLAG_BUSY               TEXT("2")
#define FLAG_USED               TEXT("1")


#define IMEI_LEN                15
#define MAC_LEN                 6

#define COL_WOID                0
#define MAX_COLUMN              100

/********** DEBUG_LEVEL ***********/
#define DLEVEL_DEBUG            0
#define DLEVEL_INFO             1
#define DLEVEL_WARNING          2
#define DLEVEL_ERROR            3
#define DLEVEL_NONE             4
class CSettingBase
{
private:
    CIniSection *pCurSection;
    CIniSection *pOldSection;
protected:
    CIniFile    *pIniFile;


    std::wstring APPNAME;
    std::wstring szFileName;
    std::wstring strModulePath;
public:
    CSettingBase()
    {
        pIniFile    = NULL;
        pCurSection = NULL;
    };
    static std::string wstr2str(const std::wstring& arg)
    {
    	int requiredSize;
    	requiredSize = WideCharToMultiByte(CP_ACP,0,arg.c_str(),arg.length(),NULL,0,NULL,NULL);
    	std::string res;
    	if (requiredSize<=0) {
    		res = "";
    		return res;
    	}
    	res.assign(requiredSize,'\0');
    	WideCharToMultiByte(CP_ACP,0,arg.c_str(),arg.length(),const_cast<char*>(res.data()),requiredSize,NULL,NULL);
    	return res;
    }
    static std::wstring str2wstr(const std::string& arg)
    {
    	int requiredSize;
    	requiredSize = MultiByteToWideChar(CP_ACP,0,arg.c_str(),arg.length(),NULL,0);
    	std::wstring res;
    	if (requiredSize<=0) {
    		res = L"";
    		return res;
    	}
    	res.assign(requiredSize,L'\0');
    	MultiByteToWideChar(CP_ACP,0,arg.c_str(),arg.length(),const_cast<wchar_t *>(res.data()),requiredSize);
    	return res;
    }
	virtual ~CSettingBase(){};
    virtual bool SaveToolSetting(std::wstring strConfig) = 0;
    virtual bool LoadToolSetting(std::wstring strConfig) = 0;
    virtual std::wstring GetStr(std::wstring &key) = 0;
    virtual std::wstring GetStr(const wchar_t *key) = 0;
    virtual bool SetStr(std::wstring &key,std::wstring &value) = 0;
    virtual bool SetStr(const wchar_t *key,const wchar_t * value) = 0;
    bool    IsPathFileExists(const std::wstring strFilaName);
    bool    CreateFolder(std::wstring strDir);

    std::wstring int2str(int i)
    {
        wchar_t szTmep[32] = {0};
        swprintf(szTmep,32,TEXT("%d"),i);
        return szTmep;
    }

    CIniSection * GetCurSection(){
        return pCurSection;
    }

    void SetCurSection(CIniSection * pSec)
    {
        if(pCurSection) pOldSection = pCurSection;
        pCurSection = pSec;
    }
    void ResumeSection()
    {
        CIniSection *pSec;
        if(pOldSection) {
            pSec        = pCurSection;
            pCurSection = pOldSection;
            pOldSection = pSec;
        }
    }

};

typedef struct{
    bool    bUsePS;
    bool    bDualPS;
    int     iBatteryGPIBCard;
    int     iChargerGPIBCard;
    int     iBatteryAddr;
    int     iChargerAddr;
    double  fBatteryVolt;
    double  fChargerVolt;
    double  fBatteryCurrent;
    double  fChargerCurrent;
}STRUCT_POWER_CONFIG,*PSTRUCT_POWER_CONFIG;

class CIniSettingBase:public CSettingBase
{
public:
    CIniSettingBase()
    {
    };
	virtual ~CIniSettingBase();
    virtual bool SaveToolSetting(std::wstring strConfig) ;
    virtual bool LoadToolSetting(std::wstring strConfig) ;

    virtual std::wstring GetStr(std::wstring &key);
    virtual std::wstring GetStr(const wchar_t *key);
    virtual bool SetStr(std::wstring &key,std::wstring &value);
    virtual bool SetStr(const wchar_t *key,std::wstring &value);
    virtual bool SetStr(const wchar_t *key,const wchar_t * value);

    void    ParseIntegers(std::wstring s, double *pInt, int *pLen);
    void    ParseIntegers(std::wstring s, int *pInt, int *nLen);
    void    ParseIntegers(std::wstring s, float *pInt, int *pLen);
    void    ParseStr(std::wstring s, std::wstring *pInt, int *pLen);
    void    ParseIp(std::wstring s, unsigned int *IPV4);
    void    ParseBools(std::wstring s, bool *pBool, int *pLen);
    static int MemoryCompare(std::wstring &s2,std::wstring &s1)
    {
        return s2.compare(s1);
    }

    std::wstring EncryptPassWord(std::wstring plaintext);
    std::wstring DecryptPassWord(std::wstring ciphertext);
    unsigned int GetIpv4()
    {
        unsigned int IPV4;
        ParseIp(strServer,&IPV4);
        return IPV4;
    }
    void SetIpv4(unsigned int IPV4)
    {
        TCHAR strIP[32] = {0};
        unsigned char ip0 = IPV4&0xff;
        unsigned char ip1 = (IPV4>>8)&0xff;
        unsigned char ip2 = (IPV4>>16)&0xff;
        unsigned char ip3 = (IPV4>>24)&0xff;
        swprintf(strIP,32,TEXT("%d.%d.%d.%d"),ip3,ip2,ip1,ip0);
        strServer = strIP;
    }
public:
    std::wstring szLan;
    bool         bDebug;
    int          nLogLevel;
    std::wstring strLogPath;

    bool         bOnlyAt;
    bool         bAutoTest;
    bool         bWideVine;
    bool         bFuse;
    std::wstring strXmlFileName;
    bool         bIsBin;
    std::wstring strFuseScriptFileName;
    /*data base config **/
    bool         bUseDB;
    std::wstring strPort;
    std::wstring strServer;
    std::wstring strUserName;
    std::wstring strPassword;
    std::wstring strDataBaseTable;
    std::wstring strDataBaseName;

    std::wstring strSupportDatabse[6];
    int          nSupportDatabse;
    int          nDataBaseType; /*ms sql access and so on **/
#ifdef USER_LOGIN
    bool         bLogin;
    std::wstring strLoginPwd;
#endif
    int          nSnType;  /*0:dev_sn 1:pcb_sn 2:hw_sn**/
    bool         bItemUsed[5];
    std::wstring strItemName[5];
    /*std::wstring strItemFlag[5]; **/
    std::wstring strFlagBusy,strFlagUnsd,strFlagUsed;

    /*Create Table Parms**/
    bool         bCreateNewTable;
    std::wstring strNewTableName;
    int          nTableColCount;
    std::wstring strTableColumnName[MAX_COLUMN];
    std::wstring strTableType[MAX_COLUMN];
    bool         bCanBeNull[MAX_COLUMN];



    std::wstring strXmlPath;
};
class CIniLocalLan:public CSettingBase
{
public:
    CIniLocalLan()
    {
    }
	virtual ~CIniLocalLan()
    {
        if(pIniFile) {
            delete pIniFile;
            /* pIniFile = NULL; **/
        }
    }
    virtual bool SaveToolSetting(std::wstring strConfig);
    virtual bool LoadToolSetting(std::wstring strConfig) ;

    virtual std::wstring GetStr(std::wstring &key,std::wstring default);
    virtual std::wstring GetStr(std::wstring &key);
    virtual std::wstring GetStr(const wchar_t *key);

    virtual bool SetStr(std::wstring &key,std::wstring &value);
    virtual bool SetStr(const wchar_t *key,const wchar_t * value);

    void    SetListViewString(HWND pWnd,TCHAR *strKey);
    void    GetListViewString(HWND pWnd,TCHAR *strKey);
public:
    bool    SetStringProc (void *pParam,void* lParam );
    bool    GetStringProc (void * pParam,void* lParam );
    bool    TreeControls(void * pParam,BOOL bSvae,int DlgId,bool bVer = false);
    void    TreeMenu(void * pParam,std::wstring strMainKeyPart);
};
/*
 * Encode a buffer into base64 format
 */
int base64_encode( unsigned char *dst, unsigned int *dlen,
                   const unsigned char *src, unsigned int slen );
/*
 * Decode a base64-formatted buffer
 */
int base64_decode( unsigned char *dst, unsigned int *dlen,
                   const unsigned char *src, unsigned int slen );
#endif
