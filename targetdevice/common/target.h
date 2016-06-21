#ifndef _TARGETDEVICE_H_
#define _TARGETDEVICE_H_
#include <vector>
#include "./IPCS/ipicom_v6.h"
/*#include "CSerial.h" **/
/*Mode **/
#define MODE_PTEST              0
#define MODE_NORMAL             1
#define MODE_AIRPLANE           2
#define MODE_MINIMAL            3
#define MODE_NONE               4

#define DEVTYPE_SOFIA3GR        2
#define DEVTYPE_XMM6321         3
#define DEVTYPE_WIFIONLY        4
#define DEVTYPE_AUTO            5
#ifndef _ENGINEDLL_H_
typedef enum
{
    INTL_2G850 = 0,
    INTL_2G900,
    INTL_2G1800,
    INTL_2G1900,
    INTL_2G_BAND_NUM
}INTL_GSM_BAND;

typedef enum
{
    INTL_WB1 = 0,
    INTL_WB2,
    INTL_WB3,
    INTL_WB4,
    INTL_WB5,
    INTL_WB6,
    INTL_WB7,
    INTL_WB8,
    INTL_3G_BAND_NUM
}INTL_WCDMA_BAND;
#endif
#define ACTIVE_3G_BAND_SIZE 14

#define HWID_MAGIC          0x0A
#define HWID_PARMS_START    5
#define HWID_LEN            6     /*parms 5 6 7 8 9 10 **/
#define HWID_STR_MAXLEN     (HWID_LEN*4-3)
class CTarget
{
public:
    CTarget();
    ~CTarget();
    static bool isGtiReady(int nCom,CLogger *m_pLogger = NULL);
    bool Open(int nCOMPort);
    bool Close();
    bool SendCMD(const char *pCMD);
    bool Handshake(bool bGTIPort);
    int  GetMode();
    std::string GetModeStr() {
        if(MODE_PTEST   == m_CurMode) return "PTEST";
        if(MODE_NORMAL  == m_CurMode) return "NORMAL";
        if(MODE_AIRPLANE== m_CurMode) return "AIRPLANE";
        if(MODE_MINIMAL == m_CurMode) return "MINIMAL";
        return "UNKNOWN";
    }
    bool SetMode(int mode);
    /*mac **/
    bool WriteMac       (const char * wifi_mac,const char * bt_mac,bool *bskip_force_wifimac,bool *bskip_force_btmac);

    /*harhware id **/
    bool WriteHwId      (const char * devid,bool *bskip_force_devid);
    bool ReadHwId       (std::string &hw_id);

    /*sn **/
    bool WriteSn        (const char * pcb_sn,const char * dev_sn,bool *bskip_force_pcbsn,bool *bskip_force_devsn);

    bool SendRecvCmd    (const char * buf,unsigned short *nof_output_bytes, char *poutput );
    bool CheckImei      (std::string imei1,std::string imei2);
    bool ToolEnable     (bool GetInfo,bool Enable);
	bool EnableIpcsd    ();
    bool sign           (std::string input,std::string output);
    bool reboot         ();
    bool GetCalibartionStatus(unsigned int Status[]);
    bool ReadWifiCalResult(int nof,unsigned short *data);
    bool DoWifiSelfCal(int nEvmLimit[],int nFELossCH[]);
    bool CommitWifiCalResult();
    bool CommitRfResult();
    bool DoConfigBand(bool bCheckTest[]);
    bool store_nvm();

    /*check **/
    bool ReadMac(std::string & wifi_mac,std::string & bt_mac);

    bool ReadSn(std::string &pcb_sn,std::string &dev_sn);

    bool ReadImei(std::string &imei1,std::string &imei2);
    bool write_param(int params,unsigned short value);
    bool read_param(int params,unsigned short &value);
    bool Getinfo();
    void StartLog(CLogger *pLogger) {
        m_pLogger = pLogger;
        m_Conn.StartLog(m_pLogger);
    }
    void StopLog(){
        m_Conn.StopLog();
        m_pLogger = NULL;
    }
    void SetDeviceType(unsigned char type) {
        m_DevType = type;
    }
    unsigned char GetDeviceType(void) {
        return m_DevType;
    }
    CLogger *m_pLogger;
    bool    bTraceEnable;
    bool    ReadDeviceInfo();
    bool    ReadPowerPercentage();
    unsigned short GetPowerPercentage() {
        return m_PowerPercentage;
    }
    bool ReadSoftwareVersion();
    std::wstring GetSoftwareVersion(){
        return m_strSoftwareVersion;
    }
    std::wstring GetPcbSn(){
        return str2wstr(m_strPcbSn);
    }
    bool Recovery();
    bool                SendAtCmd(const char * atcmd);
    bool                SendAtCmdWithResp(const char * buf,unsigned short *nof_output_bytes, char *poutput );

    bool                fus_attach();

    bool                DoWvKeyBox(const char * base64);
    static void         trim(std::string& input ,std::string whitespace );
    static std::string  wstr2str(const std::wstring& arg);
    static std::wstring str2wstr(const std::string& arg);
private:
    bool rebootxmm6321();
    bool parmstohwid(char * hwid,const unsigned short * parms,const int parms_cnt,const int hwidlen = -1);
    bool hwidtoparms(unsigned short * parms,const int parms_cnt,const char * hwid);
    bool ReadPcbSn(std::string &pcb_sn);
	bool ReadPcbOldSn(std::string &pcb_sn);
    bool ReadDevSn(std::string &dev_sn);  
	bool ReadDevOldSn(std::string &pcb_sn);
    bool WritePcbSn     (const char pcb_sn[],bool *bskip_force_pcbsn);
    bool WritePcbOldSn  (const char pcb_sn[],bool *bskip_force_pcbsn);
    bool WriteDevSn     (const char dev_sn[],bool *bskip_force_devsn);
    bool WriteDevOldSn (const char dev_sn[],bool *bskip_force_devsn);
    
    bool ReadBtMac(unsigned char bt_mac[]);
    bool ReadWifiMac(unsigned char wifi_mac[]);
    bool WriteBtMac     (unsigned char bt_mac[]     ,bool *bskip_force_btmac);
    bool WriteWiFiMac   (unsigned char wifi_mac[]   ,bool *bskip_force_wifimac);
    bool commit_mac(bool wifi,bool bt);

    bool ConnectRouter(int iRoute); 
    bool InitNvm(char **buf,int nofnvm);
    bool ReadSWVer();
    bool Read3GSupportBand();
    bool Read2GSupportBand();
    bool get_mode(unsigned short *m);
    bool CheckServicesOn(std::vector<std::string> &nodes);

private:
    CIPICOMConnectionV6 m_Conn;
    bool gti_read_us(const char pinput[], unsigned short nof_values, unsigned short values[]);
    bool gti_read_s (const char pinput[], unsigned short nof_values,short values[]);
    bool gti_generic(unsigned short /*opcode*/, unsigned int32 /*nof_input_bytes*/, const char pinput[], unsigned short *nof_output_bytes, char *poutput, unsigned short /*timeout*/);
    bool gti_generic_ty(unsigned short a, unsigned int b, const char pinput[], unsigned short *nof_output_bytes, char *poutput, unsigned short c,int retry,int to);
    bool Query(const char *pCMD, char *pOut, int &nLen);
    bool wait_nvm_stored();

private:
    std::string     m_strPcbSn;
    std::string     m_strDevSn;
    unsigned short m_CurMode;
    unsigned short m_PowerPercentage;
    unsigned short m_DdrSize;
    unsigned short m_FlashSize;
    std::wstring   m_strSoftwareVersion;
    unsigned char   m_DevType;

    bool  m_bGtiPort;
    short m_nGSMRxBandMapping[INTL_2G_BAND_NUM];
	short m_nWCDMARxBandMapping[ACTIVE_3G_BAND_SIZE];
public:
    std::wstring   m_strDdrSize;
    std::wstring   m_strFlashSize;
	bool m_b2GSupportBand[INTL_2G_BAND_NUM];
	bool m_b3GSupportBand[INTL_3G_BAND_NUM];
};
bool ConnectDUT     (CTarget &,int ,bool,CLogger *m_pLogger = NULL);
bool DisConnectDUT  (CTarget &);
bool ReBoot         (int );
int  RunCmd         (TCHAR *,DWORD *,BYTE *,DWORD *,DWORD );
bool WriteCheckIMEI (std::wstring &,std::wstring &,int ,std::wstring &,bool ,bool bUseGtiCom = false,CLogger *m_pLogger = NULL);
#endif
