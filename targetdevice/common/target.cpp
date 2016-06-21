#include "stdafx.h"
#include "target.h"
#include "strtok.h"
#include "scriptexe.h"
#include "CSerial.h"
#define dim(x) (sizeof(x) / sizeof(x[0]))
/*
#define MODE_PTEST              0
#define MODE_NORMAL             1
#define MODE_AIRPLANE           2
#define MODE_MINIMAL            3
#define MODE_NONE               4
**/
const char g_sGSMBandNames[INTL_2G_BAND_NUM][10] = { "GSM850", "GSM900", "DCS1800", "PCS1900" };
const char g_sWBandNames[INTL_3G_BAND_NUM][10] = { "WB1", "WB2", "WB3", "WB4", "WB5", "WB6", "WB7", "WB8" };
const char *g_mode_str[5] = {
    "at@bmm:UtaModePresetReq(UTA_MODE_CALIBRATION)",
    "at@bmm:UtaModePresetReq(UTA_MODE_NORMAL)",
    "at@bmm:UtaModePresetReq(UTA_MODE_NORMAL)", /*air plane **/
    "at@bmm:UtaModePresetReq(UTA_MODE_MINIMAL)",
    "",
};
const char *g_mode_read[5]  = 
{
    "stored_mode=UTA_MODE_CALIBRATION",
    "stored_mode=UTA_MODE_NORMAL",
    "stored_mode=",
    "stored_mode=UTA_MODE_MINIMAL",
    ""
};
static int hexchar2value(const char ch)
{
    int result = 0;
    if(ch >= '0' && ch <= '9') {
        result = (int)(ch - '0');
    } else if(ch >= 'a' && ch <= 'z') {
        result = (int)(ch - 'a') + 10;
    } else if(ch >= 'A' && ch <= 'Z') {
        result = (int)(ch - 'A') + 10;
    } else{
        result = -1;
    }
    return result;
}

static char value2hexchar(const int value)
{
    char result = 0;
    if(value >= 0 && value <= 9) {
        result = (char)(value + '0');
    } else if(value >= 10 && value <= 15) {
        result = (char)(value - 10 + 'A');
    }
    return result;
}
unsigned short hexstr2ushort(const char * str,int len)
{
    unsigned short  usRet   = 0;
    int             i,j;
    int             nlen;
    if(0 > len ) {
        nlen = strlen(str);
    } else {
        nlen = len;
    }
    if( 0 == nlen ) return usRet;
    if (8 < nlen) {
        j = 8;
    }
    for(int i = 0 ; i < nlen ;i ++ ) {
        unsigned short val = 0;  
        if(str[i]>='A'&&str[i]<='F') {
            val = (str[i]-'A'+10);
        } else if (str[i]>='a'&&str[i]<='f') {
            val = (str[i]-'a'+10);
        } else if (str[i]>='0'&&str[i]<='9') {
            val = (str[i]-'0');
        }
        usRet = usRet * 16+ val;
    }
    return usRet;
}

// Convert wstring to string
bool CTarget::isGtiReady(int nCom,CLogger *m_pLogger)
{
    bool    bRet = false;
    Serial  serial;
    INT     revlen;
    char    bufresp[256];
    char    *querystr = "dir\r\n";
    if (serial.Open(nCom, 115200,NOPARITY,8,ONESTOPBIT)) {
        if (serial.Write(querystr,strlen(querystr))) {
            Sleep(200);
            revlen = serial.Read(bufresp, sizeof(bufresp),500);
            bufresp[revlen] = '\0';
            if (strstr(bufresp, "OK") != NULL) {
                bRet = true;
            } 
        }
        serial.Close();
    } else {
        LOGER((CLogger::DEBUG_DUT,"serial.Open(%d, 115200) fail\r\n",nCom));
    }
    LOGER((CLogger::DEBUG_DUT,"isGtiReadys %s\r\n",bRet?"PASS":"FAIL"));
    return bRet;
}
CTarget::CTarget() : m_Conn(NULL),m_CurMode(MODE_NONE),m_pLogger(NULL),m_DevType(DEVTYPE_SOFIA3GR)
{
    for(int i = 0; i <INTL_2G_BAND_NUM;i ++)m_b2GSupportBand[i] = false;
    for(int i = 0; i <INTL_3G_BAND_NUM;i ++)m_b3GSupportBand[i] = false;

}
CTarget::~CTarget()
{
	Close();
}
bool CTarget::Open(int nCOMPort)
{
	return m_Conn.open(nCOMPort);
}
bool CTarget::Close()
{
    m_CurMode = MODE_NONE;
	return m_Conn.close_and_unload();
}
bool CTarget::SendCMD(const char *pCMD)
{
	std::string output;
	return m_Conn.query(pCMD, output) != 0;
}

bool CTarget::Query(const char *pCMD, char *pOut, int &nLen)
{
	std::string output;
	bool result = m_Conn.query(pCMD, output) == 0 ? false : true;
	strncpy_s(pOut, nLen, output.c_str(), GTI_RESPONSE_SIZE - 1);
	nLen = output.size();
	return result;
}
bool sharep(const unsigned char * buf ,int length,int nof,unsigned char *resp);
bool CTarget::sign(std::string input,std::string output)
{

    char            cmd[1024];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    unsigned short  rd;
    char            buf[32]     = {0};
    char            shavalue[32]     = {0};
    char            *p,sha[128]= {0};


    /*gen sign **/
    rd      = (unsigned short)GetTickCount();
    sprintf_s(buf,dim(buf),"%d",rd);
    if(!sharep((unsigned char *)buf,strlen(buf) + 1,dim(shavalue),(unsigned char *)shavalue)) return false;


    /*Save sha response **/
    p = sha;
    for(int i = 0;i < 32;i ++ ) {
        p += sprintf(p,"%02X",(unsigned char)shavalue[i]);
    }
    LOGER((CLogger::DEBUG_DUT, "%s\r\n",sha));
    sprintf_s(cmd,dim(cmd),"at@rkcom:sign=%s",buf);
    size = dim(resp);
    if (gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        if(sha == strstr(sha,resp)) {
            return true;
        } else {
        }
    } else {
        return false;
    }
	return false;
}

bool CTarget::Handshake(bool bGTIPort)
{
    m_bGtiPort = bGTIPort;
	return m_Conn.check(bGTIPort);
}
bool CTarget::CheckServicesOn(std::vector<std::string> &nodes)
{
	if(m_bGtiPort) return m_Conn.check_services_on(nodes,2,1000);
	return true;
}
bool CTarget::ConnectRouter(int iRoute)
{
    if(!m_bGtiPort) iRoute = 2;
	return m_Conn.reconfigure(iRoute);
}
bool CTarget::gti_generic(unsigned short a, unsigned int b, const char pinput[], unsigned short *nof_output_bytes, char *poutput, unsigned short c)
{
	std::string output;
	bool result = m_Conn.query(pinput, output);
	strncpy_s(poutput, *nof_output_bytes, output.c_str(),  *nof_output_bytes - 1);
	*nof_output_bytes = output.size();
	return result;
}
bool CTarget::gti_generic_ty(unsigned short a, unsigned int b, const char pinput[], unsigned short *nof_output_bytes, char *poutput, unsigned short c,int retry,int to)
{
	std::string output;
    m_Conn.timeout(to);
	bool result = m_Conn.query(pinput, output,0,retry);
	strncpy_s(poutput, *nof_output_bytes, output.c_str(),  *nof_output_bytes - 1);
	*nof_output_bytes = output.size();
    m_Conn.timeout(0);
	return result;
}
bool CTarget::gti_read_us(const char pinput[], unsigned short nof_values, unsigned short values[])
{
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size = GTI_RESPONSE_SIZE;
    char            token[GTI_RESPONSE_SIZE];
    char            *p;
    if (gti_generic(0, strlen(pinput), pinput, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        p = strstr(resp, "{");
        if(p == 0) {
          p = resp;
        } else {
          ++p;
        }
        int idx = 0;
        StrTok tok(p, " ,=");
        for(; (idx < nof_values) && (tok.length() != 0); ++(tok)) {
          tok.copyto(token,dim(token));
          values[idx++] = atoi(token);
        }
        return (idx == nof_values) ? true : false;
	}
    return false;
}
bool CTarget::gti_read_s(const char pinput[], unsigned short nof_values,short values[])
{
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size = GTI_RESPONSE_SIZE;
    char            token[GTI_RESPONSE_SIZE];
    char            *p;
    if (gti_generic(0, strlen(pinput), pinput, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        p = strstr(resp, "{");
        if(p == 0) {
          p = resp;
        } else {
          ++p;
        }
        int idx = 0;
        StrTok tok(p, " ,=");
        for(; (idx < nof_values) && (tok.length() != 0); ++(tok)) {
          tok.copyto(token,dim(token));
          values[idx++] = atoi(token);
        }
        return (idx == nof_values) ? true : false;
	}
    return false;
}
bool CTarget::get_mode(unsigned short *m)
{
    int             mode = MODE_NONE;
    char            buf[1024]   = "at@bmm:UtaModeGet()";
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size        = GTI_RESPONSE_SIZE;
    int             i           = 0;
    /*wait untill mode stable **/
    while(i < 15) {
        i ++;
	    if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            if (strstr(resp, "mode_switch_in_progress=FALSE") != NULL) {
                if (strstr(resp, "current_mode=UTA_MODE_CALIBRATION") != NULL) {
                    LOGER((CLogger::DEBUG_DUT,"--- DUT is in UTA_MODE_CALIBRATION ------"));
			        mode = MODE_PTEST;
		        } else if (strstr(resp, "current_mode=UTA_MODE_NORMAL") != NULL) {
                    LOGER((CLogger::DEBUG_DUT,"--- DUT is in UTA_MODE_NORMAL ------"));
			        mode =  MODE_NORMAL;
		        } else if (strstr(resp, "current_mode=UTA_MODE_MINIMAL") != NULL) {
                    LOGER((CLogger::DEBUG_DUT,"--- DUT is in UTA_MODE_MINIMAL ------"));
			        mode =  MODE_MINIMAL;
		        } else if (strstr(resp, "current_mode=UTA_MODE_AIRPLANE") != NULL) {
                    LOGER((CLogger::DEBUG_DUT,"--- DUT is in UTA_MODE_AIRPLANE ------"));
			        mode =  MODE_AIRPLANE;
                }
                break;
            } else {
                Sleep(500);
	        }
        } else {
                LOGER((CLogger::DEBUG_DUT,"--- mode_switch_in_progress!=FALSE ------"));
                LOGER((CLogger::DEBUG_DUT,"-- get_mode():fail"));
                return false;
        }
    }
    LOGER((CLogger::DEBUG_DUT,"-- get_mode():%d",mode));
    *m = mode;
	return true;
}
int CTarget::GetMode()
{
    if(MODE_NONE != m_CurMode) return m_CurMode;
    if(!ConnectRouter(0))       return MODE_NONE;
    if(!get_mode(&m_CurMode))    return MODE_NONE;
	return m_CurMode;

}
bool CTarget::SetMode(int mode)
{
    char            buf[1024];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(MODE_MINIMAL < mode || MODE_PTEST > mode) return false;
    if(!ConnectRouter(0)) return false;
    /*Set **/
    sprintf_s(buf, dim(buf),g_mode_str[mode]);
    size = dim(resp);
	if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"--SetMode():false"));
    	return false;
    }

    /*Get **/
    sprintf_s(buf, dim(buf),"at@bmm:UtaModeGet()");
    size = dim(resp);
    if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
		if (strstr(resp, g_mode_read[mode]) != NULL) {
            return true;
		}
	}
    LOGER((CLogger::DEBUG_DUT,"--SetMode():false"));
	return false;

}
bool CTarget::WritePcbOldSn  (const char pcb_sn[],bool *bskip_force_pcbsn)
{
    bool            bforce_pcbsn;
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    bforce_pcbsn    = *bskip_force_pcbsn;
    LOGER((CLogger::DEBUG_DUT,"++ WritePcbOldSn()\r\n"));
    if(!bforce_pcbsn) {
        sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_serial.pcb?");
        size = dim(resp);
        if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            if(strlen(resp) > 2 ) {
            *bskip_force_pcbsn = true;
            return true;
        } 
    }
    }

    /*write old pcb sn **/
    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_serial.pcb=\"%s\"",pcb_sn);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
	}
 
    /*read old pcb sn and compare **/
    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_serial.pcb?");
    size = dim(resp);
    if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        if(strstr(resp,pcb_sn)){
            *bskip_force_pcbsn = false;
            return true;
        }
    }
    return false;
}
bool CTarget::WritePcbSn(const char pcb_sn[],bool *bskip_force_pcbsn)
{
    bool            bforce_pcbsn;
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    bforce_pcbsn    = *bskip_force_pcbsn;
    LOGER((CLogger::DEBUG_DUT,"++ WritePcbSn()\r\n"));
    if(!bforce_pcbsn) {
        sprintf_s(cmd,dim(cmd),"sec:serial_number_get(\"pcb\")");
        size = dim(resp);
        if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            if(strlen(resp) > 2) {
            *bskip_force_pcbsn = true;
            return true;
        } 
    }
    }

    /*write pcb sn **/
    sprintf_s(cmd,dim(cmd),"sec:serial_number_set(\"pcb\",\"%s\")",pcb_sn);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
	}
 
    /*read pcb sn and compare **/
    sprintf_s(cmd,dim(cmd),"sec:serial_number_get(\"pcb\")");
    size = dim(resp);
    if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        if(strstr(resp,pcb_sn)){
            *bskip_force_pcbsn = false;
            return true;
        }
    }
    return false;
}
bool CTarget::commit_mac(bool wifi,bool bt)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(1)) return false;
    sprintf_s(cmd,dim(cmd),"at@wlan_mvt:nvm_commit()");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:commit_mac-->get nvm_commit failed\r\n"));

	    return false;
    }
    return true;
}
bool CTarget::WriteDevSn(const char dev_sn[],bool *bskip_force_devsn)
{
    bool            bforce_devsn;
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    bforce_devsn          = *bskip_force_devsn;
    LOGER((CLogger::DEBUG_DUT,"++ WriteDevSn()\r\n"));
    if(!bforce_devsn) {
        sprintf_s(cmd,dim(cmd),"sec:serial_number_get(\"device\")");
        size = dim(resp);
        if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            if(strlen(resp) > 2) {
            *bskip_force_devsn = true;
            return true;
        } 
    }
    }

    /*write pcb sn **/
    sprintf_s(cmd,dim(cmd),"sec:serial_number_set(\"device\",\"%s\")",dev_sn);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
	}
 
    /*read pcb sn and compare **/
    sprintf_s(cmd,dim(cmd),"sec:serial_number_get(\"device\")");
    size = dim(resp);
    if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        if(strstr(resp,dev_sn)){
            *bskip_force_devsn = false;
            return true;
        }
    }
    return false;
}

bool CTarget::WriteDevOldSn(const char dev_sn[],bool *bskip_force_devsn)
{
	bool            bforce_devsn;
	char            cmd[256];
	char            resp[GTI_RESPONSE_SIZE];
	unsigned short  size;
	bforce_devsn          = *bskip_force_devsn;
	LOGER((CLogger::DEBUG_DUT,"++ WriteDevOldSn()\r\n"));
	if(!bforce_devsn) {
		sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_serial.dev?");
		size = dim(resp);
		if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            if(strlen(resp) > 2 ) {
    			*bskip_force_devsn = true;
    			return true;
            }
		} 
	}

	/*write dev sn **/
	sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_serial.dev=\"%s\"",dev_sn);
	size = dim(resp);
	if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
		return false;
	}

	/*read pcb sn and compare **/
	sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_serial.dev?");
	size = dim(resp);
	if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
		if(strstr(resp,dev_sn)){
			*bskip_force_devsn = false;
			return true;
		}
	}
	return false;
}
bool CTarget::ReadPcbSn(std::string &pcb_sn)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    LOGER((CLogger::DEBUG_DUT,"++ ReadPcbSn()\r\n"));
    pcb_sn.clear();
    sprintf_s(cmd,dim(cmd),"sec:serial_number_get(\"pcb\")");
    size = dim(resp);
    if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        char *token1 = strstr(resp,"\"");
        char *token2;
        if(token1) {
            token1 ++;
            token2 = strstr(token1,"\"");
        }
        if(token1&&token2) {
            pcb_sn.assign(token1,token2 - token1 );
        }
        return true;
    }
    return false;
}
bool CTarget::ReadPcbOldSn(std::string &pcb_sn)
{
	char            cmd[256];
	char            resp[GTI_RESPONSE_SIZE];
	unsigned short  size;
	LOGER((CLogger::DEBUG_DUT,"++ ReadPcbOldSn()\r\n"));

	sprintf_s(cmd,dim(cmd),"gticom:registry.cust_serial.pcb?");
	size = dim(resp);
	if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
		char *token1 = strstr(resp,"\"");
		char *token2;
		if(token1) {
			token1 ++;
			token2 = strstr(token1,"\"");
		}
		if(token1&&token2) {
			pcb_sn.assign(token1,token2 - token1 );
		}
		return true;
	}
	return false;
}
bool CTarget::ReadDevSn(std::string &dev_sn)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    LOGER((CLogger::DEBUG_DUT,"++ ReadPcbSn()\r\n"));
    sprintf_s(cmd,dim(cmd),"sec:serial_number_get(\"device\")");
    size = dim(resp);
    if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        char *token1 = strstr(resp,"\"");
        char *token2;
        if(token1) {
            token1 ++;
            token2 = strstr(token1,"\"");
        }
        if(token1&&token2) {
            dev_sn.assign(token1,token2 - token1 );
        }
        return true;
    } 

    return false;
}
bool CTarget::ReadDevOldSn(std::string &dev_sn)
{
	char            cmd[256];
	char            resp[GTI_RESPONSE_SIZE];
	unsigned short  size;
	LOGER((CLogger::DEBUG_DUT,"++ ReadDevOldSn()\r\n"));
	sprintf_s(cmd,dim(cmd),"gticom:registry.cust_serial.dev?");
	size = dim(resp);
	if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
		char *token1 = strstr(resp,"\"");
		char *token2;
		if(token1) {
			token1 ++;
			token2 = strstr(token1,"\"");
		}
		if(token1&&token2) {
			dev_sn.assign(token1,token2 - token1 );
		}
		return true;
	} 

	return false;
}
bool CTarget::ReadSn(std::string &pcb_sn,std::string &dev_sn)
{
    LOGER((CLogger::DEBUG_DUT,"ReadSn()\r\n"));
    if(!ConnectRouter(0)) return false;

    if(DEVTYPE_XMM6321 == m_DevType) {
        if(!ReadPcbOldSn(pcb_sn)) {
			return false;
		}
        if(!ReadDevOldSn(dev_sn)) {
			return false;
		}
    } else {
    if(!ReadPcbSn(pcb_sn)) {
        return false;
    }
    if(!ReadDevSn(dev_sn)) {
        return false;
    }
    }

    return true;
}
bool CTarget::WriteSn(const char * pcb_sn,const char * dev_sn,bool *bskip_force_pcbsn,bool *bskip_force_devsn)
{
    LOGER((CLogger::DEBUG_DUT,"WriteSn()\r\n"));
    if(!ConnectRouter(0)) return false;
    
    if(DEVTYPE_XMM6321 == m_DevType) {
		if(pcb_sn) {
			if(0 != strlen(pcb_sn)) {
                if(!WritePcbOldSn(pcb_sn,bskip_force_pcbsn)) {
					return false;
				}
			}
		}
		if(dev_sn) {
			if(0 != strlen(dev_sn)) {
				if(!WriteDevOldSn(dev_sn,bskip_force_devsn)) {
					return false;
				}
			}
		}
        if(!store_nvm()) return false;
    } else {
    if(pcb_sn) {
        if(0 != strlen(pcb_sn)) {
            if(!WritePcbSn(pcb_sn,bskip_force_pcbsn)) {
                return false;
            }
        }
    }
    if(dev_sn) {
        if(0 != strlen(dev_sn)) {
            if(!WriteDevSn(dev_sn,bskip_force_devsn)) {
                return false;
            }
        }
    }
    }
    return true;
}

/*
the struct of parms for store hardware ID
by lanshh 2016-04-08
---------------------------------------------
|  magic  |  length |       hwid ....       |
|   4bits |  8 bits |     length*4 bits     |
---------------------------------------------
**/
bool CTarget::hwidtoparms(unsigned short * parms,const int parms_cnt,const char * hwid)
{
    const unsigned char magic   = HWID_MAGIC;
    unsigned char       len     = 0;
    int                 nIdex   = 3;
    int                 idx,idy;

    unsigned char       hex;     
    unsigned short      tmp;
    for(;*hwid;) {
        hex         = hexchar2value(*hwid);
        idx         = nIdex/4;
        idy         = (nIdex%4)*4;      /*shift left 4 **/
        if(parms_cnt == idx) break;
        tmp         = parms[idx];
        tmp         &=(~(0x0f<<idy));
        tmp         |= (hex&0xf)<<idy;
        parms[idx]  = tmp;
        hwid++;
        nIdex++;
    }
    /*
    save magic and length parms[0]bits 0:11 
    **/
    len     = nIdex - 3; /*the actual length of hwid**/
    tmp     = parms[0];
    tmp     &= ~0xfff;
    tmp     |=(((len&0xff)<<4)| (0xf&magic));
    parms[0] = tmp;
    return true;
}

bool CTarget::parmstohwid(char * hwid,const unsigned short * parms,const int parms_cnt,const int buflen)
{
    int                 buf_len;
    unsigned char       magic   = parms[0]&0xF;
    unsigned char       len     = ((parms[0]&0xff0)>> 4);
    int                 hwid_idx= 0;
    int                 nIdex   = 3;
    int                 idx,idy;

    if(HWID_MAGIC!=magic) {
        LOGER((CLogger::DEBUG_ERROR,"read hwid magic=%d error and return",(int)magic));
        return false; /*invalid hwid **/
    }
    if(len> ((parms_cnt<<2)-3) || 0 >= len) {
        LOGER((CLogger::DEBUG_ERROR,"read hwid len=%d error and return",(int)len));
        return false; /*invalid hwid **/
    }
    if(0 >= buflen ) {
        buf_len = (parms_cnt<<2) - 3 + 1; /*null terminate **/
    } else {
        buf_len = buflen;
    }

    for(;hwid_idx <len;) {
        idx             = nIdex/4;
        idy             = (nIdex%4)*4;          /* shift left 4    **/
        if(parms_cnt    == idx) break;          /* parms overload  **/
        if(hwid_idx     == buf_len - 1) break;  /* hwid overload   **/
        hwid[hwid_idx]  = value2hexchar((parms[idx]>>idy)&0xf);
        hwid_idx++;
        nIdex++;
    }
    hwid[hwid_idx] = '\0';
    LOGER((CLogger::DEBUG_ERROR,"parmstohwid:len=%d,id=%s",(int)len,hwid));
    return true;
}
/*hardware Id **/
bool CTarget::ReadHwId (std::string &hw_id)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    LOGER((CLogger::DEBUG_INFO,"ReadHwId()"));
    int             parmidx,idx;
    unsigned short  usparms[HWID_LEN];

    parmidx = HWID_PARMS_START;
    for (idx = 0;idx < HWID_LEN ;idx++) {
        sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_%d?",parmidx);
        size = dim(resp);
        if(!(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0))) {
            return false;
        }
        usparms[idx] =  atoi(resp);
        parmidx++;
    }
    memset(cmd,0,sizeof(cmd));
    if(!parmstohwid(cmd,usparms,HWID_LEN)) {
        LOGER((CLogger::DEBUG_INFO,"ReadHwId() parmstohwid return 0"));
    }
    hw_id = cmd;
    return true;
}

bool CTarget::WriteHwId (const char * hwid,bool *bskip_force_devid)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    int             parmidx,idx;
    unsigned short  usparms[HWID_LEN];

    bool            bforce_devid    = *bskip_force_devid;
    LOGER((CLogger::DEBUG_INFO,"WriteDevID()\r\n"));
    if(!ConnectRouter(0)) return false;

    hwidtoparms(usparms,HWID_LEN,hwid);
    parmidx = HWID_PARMS_START;
	for(idx = 0;idx< HWID_LEN;idx++,parmidx++) {
		sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_%d=%d",parmidx,usparms[idx]);
		size = dim(resp);
		if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
    }
	}

    /*check device id **/
    memset(usparms,0,sizeof(usparms));
    parmidx = HWID_PARMS_START;
    for (idx = 0;idx < HWID_LEN ;idx++,parmidx++) {
        sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_%d?",parmidx);
        size = dim(resp);
        if(!(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0))) {
            return false;
        }
        usparms[idx] = atoi(resp);
    }
    /*compare read parms **/
    memset(cmd,0,sizeof(cmd));
    if(!parmstohwid(cmd,usparms,HWID_LEN)) return false;
    if( 0 != strcmp(cmd,hwid)) return false;
	*bskip_force_devid = false;
    if(!store_nvm()) return false;
    return true;
}
bool CTarget::SendAtCmdWithResp(const char * buf,unsigned short *nof_output_bytes, char *poutput )
{
    LOGER((CLogger::DEBUG_DUT,"++SendAtCmdWithResp()\r\n"));
    if(!ConnectRouter(0)) return false;
    return SendRecvCmd(buf,nof_output_bytes,poutput);
}

bool CTarget::SendAtCmd(const char * atcmd)
{
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size = GTI_RESPONSE_SIZE;
    LOGER((CLogger::DEBUG_DUT,"++SendAtCmd()\r\n"));
    if(!ConnectRouter(0)) return false;
    size = dim(resp);
    if(gti_generic(0, strlen(atcmd), atcmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        LOGER((CLogger::DEBUG_DUT,"SendAtCmd ret %s\r\n",resp));
        /*if(1 == atoi(resp)) { **/
            return true;
        /*} **/
    }
    LOGER((CLogger::DEBUG_DUT,"--SendAtCmd()\r\n"));
    return false;
}
/*
fus_action = 0
fus_value = 0
result_cause = 4130
**/
bool CTarget::fus_attach()
{

    bool  bRet      = false;
    DWORD dwStart_Ticks ,dwCurrent_Ticks,dwTicks_Cnt;;
    char            *cmd[] = {
        "at@",
        "at@sec:fus_attach()",
        "at@sec:!"
        };
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size = GTI_RESPONSE_SIZE;
    LOGER((CLogger::DEBUG_DUT,"++fus_attach()\r\n"));
    if(!ConnectRouter(0)) return false;
    dwStart_Ticks = GetTickCount();
    do{
        dwCurrent_Ticks = GetTickCount();
        dwTicks_Cnt = dwCurrent_Ticks >= dwStart_Ticks?(dwCurrent_Ticks -  dwStart_Ticks):(0xffffffff + dwCurrent_Ticks - dwStart_Ticks);
        if(dwTicks_Cnt > 40*1000) break;
        size = dim(resp);
        if(gti_generic(0, strlen(cmd[0]), cmd[0], &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            LOGER((CLogger::DEBUG_DUT,"%s ret %s\r\n",cmd[0],resp));
        } else {
            LOGER((CLogger::DEBUG_DUT,"%s ret %s\r\n",cmd[0],resp));
            break;
        }

        size = dim(resp);
        if(gti_generic(0, strlen(cmd[1]), cmd[1], &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            LOGER((CLogger::DEBUG_DUT,"%s ret %s\r\n",cmd[1],resp));
            std::string strRet      = resp;
            std::size_t len         = strRet.size();
            std::size_t funpos      = strRet.find("fus_action = ");
            std::size_t resultpos   = strRet.find("result_cause = ");
            if((std::string::npos != funpos)&&(std::string::npos != resultpos )
                &&(funpos + 13 < len)&&(resultpos + 15 < len)) {
                int nfunid  = atoi(strRet.substr(funpos + 13).c_str());
                int nresult = atoi(strRet.substr(resultpos + 15).c_str());
                LOGER((CLogger::DEBUG_DUT,"nfunid=%d,nresult=%d\r\n",nfunid,nresult));
                if(0 == nfunid && 0 == nresult ) {
                    bRet = TRUE;
                } else if(0 != nresult){
                    break;
                }
            }
        } else {
            LOGER((CLogger::DEBUG_DUT,"%s ret %s\r\n",cmd[1],resp));
            break;
        }

        size = dim(resp);
        if(gti_generic(0, strlen(cmd[2]), cmd[2], &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
            LOGER((CLogger::DEBUG_DUT,"%s ret %s\r\n",cmd[1],resp));
        } else {
            LOGER((CLogger::DEBUG_DUT,"%s ret %s\r\n",cmd[1],resp));
            break;
        }
        Sleep(100);
    } while(false == bRet );

    
    LOGER((CLogger::DEBUG_DUT,"--fus_attach()\r\n"));
    return bRet;

}
bool CTarget::DoWvKeyBox(const char * base64)
{
    char            cmd[1024];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size = GTI_RESPONSE_SIZE;
    LOGER((CLogger::DEBUG_DUT,"++DoWvKeyBox()\r\n"));
    if(!ConnectRouter(0)) return false;
    sprintf_s(cmd,dim(cmd),"at@rkcom:keyboxbin=\"%s\"",base64);
    size = dim(resp);
    if(gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        LOGER((CLogger::DEBUG_DUT,"DoWvKeyBox ret %s\r\n",resp));
        if(1 == atoi(resp)) {
            return true;
        }
    }
    LOGER((CLogger::DEBUG_DUT,"--DoWvKeyBox()\r\n"));
    return false;
}

bool CTarget::WriteMac(const char * wifi_mac,const char * bt_mac,bool *bskip_force_wifimac,bool *bskip_force_btmac)
{
    unsigned char   mac[6];
    LOGER((CLogger::DEBUG_DUT,"WriteMac()\r\n"));
    if(!ConnectRouter(1)) return false;

    if(wifi_mac) {
        if(12 == strlen(wifi_mac)) {
            for(int i = 0; i < 6 ; i++ ) {
                mac[i] = (hexchar2value(wifi_mac[2*i]) << 4)|(hexchar2value(wifi_mac[2*i + 1]));
            }
            /*LDEGMSG(DEBUG_LOG, ("wifi mac %02x:%02x:%02x:%02x:%02x:%02x\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5])); **/
            if(!WriteWiFiMac(mac,bskip_force_wifimac)) {
                return false;
            }
        }
    }
    if(bt_mac) {
        if(12 == strlen(bt_mac)) {
            for(int i = 0; i < 6 ; i++ ) {
                mac[i] = (hexchar2value(bt_mac[2*i]) << 4)|(hexchar2value(bt_mac[2*i + 1]));
            }
            /*LDEGMSG(DEBUG_LOG, ("bt mac %02x:%02x:%02x:%02x:%02x:%02x\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5])); **/
            if(!WriteBtMac(mac,bskip_force_btmac)) {
                return false;
            }
        }
    }
    /* if(!commit_mac(true,true))return false; **/
    return true;
}
bool CTarget::rebootxmm6321()
{   
    char            *cmd;
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size = GTI_RESPONSE_SIZE;
    LOGER((CLogger::DEBUG_DUT,"++Reboot()\r\n"));
    if(!ConnectRouter(0)) return false;
#if 0
    /*this method will lost some message 
      here just reboot the modem 
    **/
    cmd     = "prodif:at_cmd[0,1020]=%b64:Cgk=";
    size    = GTI_RESPONSE_SIZE;
    if (!gti_generic_ty(0, strlen(cmd), cmd, &size, resp, 0,1,200) || (strstr(resp, "ERR") != 0)) {
        return false;
    }
    cmd     = "prodif:run_cmd()";
    size    = GTI_RESPONSE_SIZE;
    if (!gti_generic_ty(0, strlen(cmd), cmd, &size, resp, 0,1,200) || (strstr(resp, "ERR") != 0)) {
        return false;
    }
    

#else 
    cmd     = "at@bmm:UtaBootShutdownReq(UTA_BOOT_SHUTDOWN_TYPE_SOFTWARE_RESET)";
    size    = GTI_RESPONSE_SIZE;
    if (!gti_generic_ty(0, strlen(cmd), cmd, &size, resp, 0,1,200) || (strstr(resp, "ERR") != 0)) {
        return false;
    }
#endif
    LOGER((CLogger::DEBUG_DUT,"--Reboot():true\r\n"));
    return true;
}
bool CTarget::reboot()
{
    char            *cmd;
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size = GTI_RESPONSE_SIZE;
    LOGER((CLogger::DEBUG_DUT,"++Reboot()\r\n"));
    if(DEVTYPE_XMM6321 == m_DevType ) return rebootxmm6321();
    if(!ConnectRouter(0)) return false;
    cmd = "at@rkcom:reboot=\"true\"";
    if (gti_generic_ty(0, strlen(cmd), cmd, &size, resp, 0,1,200) && (strstr(resp, "ERR") == 0)) {
        LOGER((CLogger::DEBUG_DUT,"--Reboot():true\r\n"));
        return true;
    }
    LOGER((CLogger::DEBUG_DUT,"--Reboot():false\r\n"));
    return false;
}


bool CTarget::DoConfigBand(bool bCheckTest[])
{
    char            buf[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    int             n2GBand = 0;
    int             n3GBand = 0;
    int             i;
    std::string strReadValue;
    std::string     szValue;
    std::string str2GBand="";
    std::string str3GBand="";
    LOGER((CLogger::DEBUG_DUT,"++DoConfigBand()\r\n"));
    /*GSM **/
    if (bCheckTest[0]) { //850 
        str2GBand += "6,";
        n2GBand++;
    }
    if (bCheckTest[1]) { //900 
        str2GBand += "2,";
        n2GBand++;
    }
    if (bCheckTest[2]) { //1800 
        str2GBand += "4,";
        n2GBand++;
    }
    if (bCheckTest[3]) {//1900 
        str2GBand += "5,";
        n2GBand++;
    }

    /*WCDMA **/
    if (bCheckTest[4]) { //B1 
        str3GBand += "1,";
        n3GBand++;
    }
    if (bCheckTest[5]) {//B2
        str3GBand += "2,";
        n3GBand++;
    }
    if (bCheckTest[6]){ //B5
        str3GBand += "5,";
        n3GBand++;
    }
    if (bCheckTest[7]) {//B8
        str3GBand += "8,";
        n3GBand++;
    }
    for(i = n2GBand;i < 10;i ++) {
        if(9 == i) {
            str2GBand += "255";
        } else {
            str2GBand += "255,";
        }
    }
	str2GBand = "{" + str2GBand + "}";

    for(i = n3GBand;i < 8;i ++) {
        if(7 == i) {
            str3GBand += "0";
        } else {
            str3GBand += "0,";
        }
    }
    str3GBand = "{" + str3GBand + "}";
    
    if(!ConnectRouter(0)) return false;
	sprintf_s(buf, dim(buf),"at@nvm:fix_cps.gsm_band_config.nvr_band_config_list[0,9]?");
    size = dim(resp);
    if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
    }
    strReadValue.assign(resp);
	if (strReadValue.compare(str2GBand) !=0 ) {
		sprintf_s(buf,dim(buf),"at@nvm:fix_cps.gsm_band_config.nvr_nbr_band_config=%d", n2GBand);
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
			}
		sprintf_s(buf, dim(buf),"at@nvm:fix_cps.gsm_band_config.nvr_band_config_list[0]=%s", str2GBand.c_str());
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
		}
		sprintf_s(buf, "at@nvm:store_nvm(fix_cps)");
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
			}
		sprintf_s(buf, "at@nvm:wait_nvm_idle()");
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
		}
		sprintf_s(buf, "at@nvm:fix_cps.gsm_band_config.nvr_band_config_list[0,9]?");
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
			}
        szValue.assign(resp);
        szValue.erase(0, szValue.find_first_not_of(" "));     /*remove space **/
		if ( str2GBand.compare(szValue) !=0 ) {
            LOGER((CLogger::DEBUG_DUT,"Error:DoConfigBand-->Check nvr_band_config_list[0,9] failed,2GBand=%s,read=%s\r\n",str2GBand.c_str(),szValue.c_str()));
            return false;
		}
	}
    m_b2GSupportBand[INTL_2G850]    = bCheckTest[0];
    m_b2GSupportBand[INTL_2G900]    = bCheckTest[1];
    m_b2GSupportBand[INTL_2G1800]   = bCheckTest[2];
    m_b2GSupportBand[INTL_2G1900]   = bCheckTest[3];
    sprintf_s(buf, dim(buf),"at@nvm:fix_rf_ue2.tx_fix.tx3g_config_table.active_bands[0,7]?");
    size = dim(resp);
    if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
    }
    strReadValue.assign(resp);
	if (strReadValue.compare(str3GBand)!=0)
	{
		sprintf_s(buf, "at@nvm:fix_rf_ue2.tx_fix.tx3g_config_table.active_bands[0]=%s",str3GBand.c_str());
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
			}
		sprintf_s(buf, "at@nvm:store_nvm(fix_rf_ue2)");
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
		}
		sprintf_s(buf, "at@nvm:wait_nvm_idle()");
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
			}
		sprintf_s(buf, "at@nvm:fix_rf_ue2.tx_fix.tx3g_config_table.active_bands[0,7]?");
        size = dim(resp);
        if (!gti_generic(0, strlen(buf), buf, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
            return false;
		}
        szValue.assign(resp);
        szValue.erase(0, szValue.find_first_not_of(" "));     /*remove space **/
		if (str3GBand.compare(szValue)!=0) { /*number ,no case **/
            LOGER((CLogger::DEBUG_DUT,"Error:DoConfigBand-->Check nvr_band_config_list[0,9] failed,2GBand=%s,read=%s\r\n",str2GBand.c_str(),szValue.c_str()));
            return false;
			}
	}
    m_b3GSupportBand[INTL_WB1] = bCheckTest[4];
    m_b3GSupportBand[INTL_WB2] = bCheckTest[5];
    m_b3GSupportBand[INTL_WB5] = bCheckTest[6];
    m_b3GSupportBand[INTL_WB8] = bCheckTest[7];

    m_b3GSupportBand[INTL_WB3] = false;
    m_b3GSupportBand[INTL_WB4] = false;
    m_b3GSupportBand[INTL_WB6] = false;
    m_b3GSupportBand[INTL_WB7] = false;
    
    return true;
}
bool CTarget::Getinfo()
{
    LOGER((CLogger::DEBUG_DUT,"++Getinfo()\r\n"));
    if(!ConnectRouter(0))       return false;
    if(!Read2GSupportBand())    return false;
    if(!Read3GSupportBand())    return false;
    if(MODE_NONE == m_CurMode) {
        if(!get_mode(&m_CurMode))
            return false;
    }
    return true;
}

/*
*bool WriteMac   (const char * wifi_mac,const char * bt_mac,bool *bskip_force_wifimac,bool *bskip_force_btmac);
* only gti port
**/

bool CTarget::ToolEnable(bool GetInfo,bool bEnable)
{
    char            *cmd;
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    std::vector<std::string> nodes;
#if 1
    ReadSWVer();
    LOGER((CLogger::DEBUG_DUT,"++ToolEnable(%d)\r\n",bEnable));
#else 
    /*Sleep(2000);**/
    LOGER((CLogger::DEBUG_DUT,"++ToolEnable(%d) sleep(2000)\r\n",bEnable));
#endif
    if(!ConnectRouter(0)) return false;

    /*get info **/
    /*if(!get_mode(&m_CurMode)) return false; **/
    if(GetInfo) {
        if(!Read2GSupportBand())    return false;
        if(!Read3GSupportBand())    return false;
        if(!get_mode(&m_CurMode))   return false;
        /*if(!ReadPcbSn(m_strPcbSn))return false; **/
        /*if(!InitNvm(initnvm,3))    return false; **/
    }

#if TOOL_SIGN_ON 
        /*sign dut **/
    if(DEVTYPE_SOFIA3GR == m_DevType) {
        if(!sign("","")) {
        LOGER((CLogger::DEBUG_DUT,"Failed to sign with DUT\r\n"));
        LOGER((CLogger::DEBUG_DUT,"--ToolEnable():failed\r\n"));
            return false;
        }
    LOGER((CLogger::DEBUG_DUT,"sign with DUT ok\r\n"));
    }
#endif

#if TOOL_ENABLE_ON
    if(MODE_NORMAL == m_CurMode) {
        if(bEnable) {
            cmd  = "at@rkcom:tool_enable=\"true\"";
            size = dim(resp);
            if (gti_generic_ty(0, strlen(cmd), cmd, &size, resp, 0,2,5000) && (strstr(resp, "ERR") == 0)) {
                if( !strstr(resp,"1")) {
                    LOGER((CLogger::DEBUG_DUT,"--ToolEnable():failed\r\n"));
                    return false;
                }
        	} else {
                LOGER((CLogger::DEBUG_DUT,"--ToolEnable():failed\r\n"));
                return false;
            }
           
        }  
        /*
        else {
            cmd  = "at@rkcom:tool_enable=\"false\"";
            size = dim(resp);
            if (gti_generic_ty(0, strlen(cmd), cmd, &size, resp, 0,2,5000) && (strstr(resp, "ERR") == 0)) {
                if( !strstr(resp,"1")) {
                    LDEGMSG(DEBUG_LOG, ("--ToolEnable():failed\r\n"));
                    return false;
                }
        	} else {
                LDEGMSG(DEBUG_LOG, ("--ToolEnable():failed\r\n"));
                return false;
            }
            
        }
        **/
    }
    /*if in enable,check rsnode service**/
    if(bEnable) {
        nodes.clear();
		nodes.push_back("rsnode");
		if(MODE_PTEST == m_CurMode) {
			nodes.push_back("cp");
        }
        if(!CheckServicesOn(nodes)) {
            LOGER((CLogger::DEBUG_DUT,"CheckServicesOn(\"rsnode\") failed\r\n"));
            LOGER((CLogger::DEBUG_DUT,"--ToolEnable():failed\r\n"));
            return false;
        }
    }
    LOGER((CLogger::DEBUG_DUT,"--ToolEnable():ok\r\n"));
    return true;
#else 
    return true;
#endif
}

bool CTarget::EnableIpcsd()
{
    char            *cmd;
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    std::vector<std::string> nodes;
    LOGER((CLogger::DEBUG_DUT,"++EnableIpcsd()\r\n"));

    if(m_bGtiPort)              return true; /*no need enable ipcsd **/
    if(!get_mode(&m_CurMode))   return false;
    if(MODE_PTEST != m_CurMode) {
        cmd  = "at@rkcom:start_ipcsd=\"true\"";
        size = dim(resp);
        gti_generic_ty(0, strlen(cmd), cmd, &size, resp, 0,1,1000);
        Sleep(2000);
        LOGER((CLogger::DEBUG_DUT,"send at@rkcom:start_ipcsd=\"true\" finish\r\n"));
    }
    LOGER((CLogger::DEBUG_DUT,"--EnableIpcsd()\r\n"));
    return true;
}
bool CTarget::Read2GSupportBand()
{
	char            *buf = "at@gcal:gcal_rf_cap(15)";
	char            resp[GTI_RESPONSE_SIZE];
	unsigned short  size;

	memset(m_b2GSupportBand, 0, sizeof(m_b2GSupportBand));
    size            = dim(resp);
	if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0))
	{
		int nValue = atoi(resp);

		m_b2GSupportBand[0] = ((nValue & 0x80) >> 7) == 1;
		m_b2GSupportBand[1] = ((nValue & 0x40) >> 6) == 1;
		m_b2GSupportBand[2] = ((nValue & 0x20) >> 5) == 1;
		m_b2GSupportBand[3] = ((nValue & 0x10) >> 4) == 1;
        LOGER((CLogger::DEBUG_DUT,"support bands:%s%s%s%s\r\n",
            m_b2GSupportBand[0]?"GSM850:":"",
            m_b2GSupportBand[1]?"GSM900:":"",
            m_b2GSupportBand[2]?"DCS1800:":"",
            m_b2GSupportBand[3]?"PCS1900":""));
        return true;
	}
	else
	{
		LOGER((CLogger::DEBUG_DUT,"Failed to read supported GSM bands\r\n"));
		return false;
	}
}
bool CTarget::ReadSWVer()
{
	char            *buf = "at@gticom:registry.Platform_ID.sw_version_main_image?";
	char            resp[GTI_RESPONSE_SIZE];
	unsigned short  size;
	memset(m_b2GSupportBand, 0, sizeof(m_b2GSupportBand));
    size            = dim(resp);
	if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0))
	{
        return true;
	} else {
        return false;
	}
}
bool CTarget::ReadDeviceInfo()
{
    char            *buf = "at@rkcom:AP_Info=?";
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    m_PowerPercentage   = 0;
    size                = dim(resp);
    if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        bool    PreGet = true;;
        char    token[256];
        StrTok  tok(resp, ",");
        if(PreGet&&tok.length()){
            tok.copyto(token,dim(token));
            m_PowerPercentage    = atoi(token);
            ++ tok;
        } else {
            m_PowerPercentage    = 0xffff;
            PreGet = false;
        };
        if(PreGet&&tok.length()){
            tok.copyto(token,dim(token));
            m_strDdrSize         = str2wstr(std::string(token));
            ++ tok;
        } else {
            m_DdrSize            = 0xffff;
            PreGet               = false;
        }
        if(PreGet&&tok.length()){
            tok.copyto(token,dim(token));
            m_strFlashSize       = str2wstr(std::string(token));
            ++ tok;
        } else {
            m_FlashSize          = 0xffff;
            PreGet               = false;
        }
        if(PreGet&&tok.length()){
            tok.copyto(token,dim(token));
            m_strSoftwareVersion = str2wstr(std::string(token));
            ++ tok;
        } else {
            m_strSoftwareVersion = TEXT("UNKNOWN");
            PreGet               = false;
        }
        return true;
    } else {
        LOGER((CLogger::DEBUG_DUT,"Failed to read device info\r\n"));
        return false;
    }
}
bool CTarget::ReadPowerPercentage()
{
    char            *buf = "at@rkcom:batterylevel=?";
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    m_PowerPercentage   = 0;
    size                = dim(resp);
    if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0))
    {
        m_PowerPercentage = atoi(resp);
        LOGER((CLogger::DEBUG_DUT,"Power Percentage=%d\r\n",m_PowerPercentage));
        return true;
    }
    else
    {
        LOGER((CLogger::DEBUG_DUT,"Failed to read Power Percentage\r\n"));
        return false;
    }
}
bool CTarget::ReadSoftwareVersion()
{
    char            *buf = "at@rkcom:batterylevel=?";
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    m_PowerPercentage   = 0;
    size                = dim(resp);
    if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        m_PowerPercentage = atoi(resp);
        LOGER((CLogger::DEBUG_DUT,"Power Percentage=%d\r\n",m_PowerPercentage));
        return true;
    } else {
        LOGER((CLogger::DEBUG_DUT,"Failed to read Power Percentage\r\n"));
        return false;
    }
}
bool CTarget::Recovery()
{
    char            *buf = "at@rkcom:recovery=\"true\"";
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(0)) return false;
    size                = dim(resp);
    if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        LOGER((CLogger::DEBUG_DUT,"Recovery success\r\n"));
        return true;
    } else {
        LOGER((CLogger::DEBUG_DUT,"Recovery fail\r\n"));
        return false;
    }
}
bool CTarget::InitNvm(char **initnvm,int nofnvm) 
{
    char            buf[1000];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    for(int i = 0; i < nofnvm; i ++ ) {
        size            = dim(resp);
        sprintf_s(buf,1000,initnvm[i]);
        LOGER((CLogger::DEBUG_DUT,"InitNvm %dst %s fail\r\n",i,buf));
        if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        } else {
            LOGER((CLogger::DEBUG_DUT,"InitNvm %s fail\r\n",buf));
            return false;
        }
    }
    return true;
};
bool CTarget::Read3GSupportBand()
{
	char            *buf = "at@ucal:ucal_get_3g_tx_config_table()";
	char            resp[GTI_RESPONSE_SIZE];
	unsigned short  size;
    int nof_parms = 10;
    int nof_values  = 8;	
    memset(m_b3GSupportBand, 0, sizeof(m_b3GSupportBand));
    size            = dim(resp);
    if (gti_generic(0, strlen(buf), buf, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        char *p = strstr(resp, "={");
        if (p == 0) {
            return false;
        }
        StrTok tok(p + 2, "} ,\n\r"); // Skip the "={" 2 characters
        for(int row_idx = 0; (row_idx < nof_values); row_idx++) {
            for(int parm_idx = 0; (parm_idx < nof_parms) && (tok.length() != 0); ++(tok), parm_idx++) {
                char token[GTI_RESPONSE_SIZE];
                tok.copyto(token,dim(token));
                if(parm_idx == 0) {//Band, Diversity, Band_group, center channel, max power
                    int value = atoi(token);
                    value-=1;
                    if(value>=0 &&value<8) m_b3GSupportBand[value] = true;
                }
                if(parm_idx == 1) {
                    //UE2available3GRdBands[row_idx] = atoi(token);
                }
                if(parm_idx == 2) {
                    //UE23Gband2group[Active3gbands[row_idx] - 1] = atoi(token);
                }
                if(parm_idx == 3) {
                    //UE23GTxCenterFreq[row_idx] = atoi(token);
                }
                if(parm_idx == 4) {
                    //UE23GTxMaxPower[row_idx] = atoi(token);
                }
            }
        }
        for(int j = 0;j< 8;j ++) {
            if(m_b3GSupportBand[j]) {
                LOGER((CLogger::DEBUG_DUT,"Supported WCDMA bands %d\r\n",j +1));
            }
        }        
    } else {
        LOGER((CLogger::DEBUG_DUT,"Failed to read supported WCDMA bands\r\n"));
        return false;
    }

	return true;
}


bool bImeiValid(char *resp ,std::string &imei1,std::string &imei2) 
{
    char   *p[4] = {0};
    bool    bRet;
    p[0] = strstr(resp,"imei[0]");
    if(p[0]) {
        p[1] = strstr(p[0],imei1.c_str());
    }
    bRet = p[0]&&p[1];
    if(bRet&&imei2.empty()) {
        bRet = ( p[0] < p[1] );
    } else if(bRet) {
        p[2] = strstr(resp,"imei[1]");
        if(p[2]) {
            p[3] = strstr(p[2],imei2.c_str());
        }
        bRet = p[2]&&p[3];
        if(bRet) {
            bRet = (p[0] < p[1]) && (p[1] < p[2]) && (p[2] < p[3]) ;
        }
    }
    return bRet;
}

bool CTarget::CheckImei(std::string imei1,std::string imei2)
{
    char            cmd[128];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    LOGER((CLogger::DEBUG_DUT,"++CheckImei\r\n"));
    if(!ConnectRouter(0)) return false;
    sprintf_s(cmd,sizeof(cmd)/sizeof(cmd[0]),"sec:imei_read(0)");
    size = dim(resp);
    if (gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        if(bImeiValid(resp,imei1,imei2)) return true;
    }
    return false;
}

bool CTarget::ReadImei(std::string &imei1,std::string &imei2)
{
    char            cmd[128];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(0)) return false;
    sprintf_s(cmd,sizeof(cmd)/sizeof(cmd[0]),"sec:imei_read(0)");
    size = dim(resp);
    if (gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        char *token1 = strstr(resp,"\"");
        char *token2;
        if(token1) {
            token1 ++;
            token2 = strstr(token1,"\"");
        }
        if(token1&&token2) {
            imei1.assign(token1,token2 - token1 );
        }
        if(token2) {
            token2 ++;
            token1 = strstr(token2,"\"");
            if(token1) {
                token1 ++;
                token2 = strstr(token1,"\"");
            } else {
                token2 = NULL;
            }
            if(token1&&token2) {
                imei2.assign(token1,token2 - token1 );
            }
        }
        return true;
    }
    return false;
}


bool CTarget::WriteWiFiMac(unsigned char wifi_mac[],bool *bskip_force_wifimac)
{
    bool            bforce_wifimac;
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size        = GTI_RESPONSE_SIZE;
    unsigned short  mac[6];
    bforce_wifimac          = *bskip_force_wifimac;
    LOGER((CLogger::DEBUG_DUT,"++ WriteWiFiMac()\r\n"));
    if(!bforce_wifimac) {
        if(gti_read_us("wlan_mvt:read_mac_address_command()",6,mac)) {
            *bskip_force_wifimac = true;
            return true;
        } 
    }

    /*write wifi mac **/
    sprintf_s(cmd,dim(cmd),"wlan_mvt:add_mac_address_command({%d,%d,%d,%d,%d,%d})",
        wifi_mac[0],wifi_mac[1],wifi_mac[2],
        wifi_mac[3],wifi_mac[4],wifi_mac[5]);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
	}
 
    /*read wifi mac and compare **/
    if(gti_read_us("wlan_mvt:read_mac_address_command()",6,mac)) {
        int i;
        for( i = 0;i <6 ; i++ ) {
            if (mac[i] != wifi_mac[i]) {
                break;
            }
        }
        if(6 == i) {
            *bskip_force_wifimac = false;
            return true;
        } 
    }
    return false;
}
bool CTarget::GetCalibartionStatus(unsigned int Status[])
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_1?");
    size = dim(resp);
    if (gti_generic(0, strlen(cmd), cmd, &size, resp, 0)&&(strstr(resp, "ERR") == 0)) {
        Status[0] = atoi(&resp[0]);
	} else {
	    return false;
    }

    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_2?");
    size = dim(resp);
    if (gti_generic(0, strlen(cmd), cmd, &size, resp, 0)&&(strstr(resp, "ERR") == 0)) {
        Status[1] = atoi(&resp[0]);
	} else {
        return false;
    }


    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_3?");
    size = dim(resp);
    if (gti_generic(0, strlen(cmd), cmd, &size, resp, 0)&&(strstr(resp, "ERR") == 0)) {
        Status[2] = atoi(&resp[0]);
	} else {
        return false;
    }
    return true;
}
bool CTarget::ReadWifiCalResult(int nof,unsigned short *data)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    sprintf_s(cmd,dim(cmd),"wlan_mvt:fe_loss_get()");
    size = dim(resp);
    if(gti_read_us(cmd, nof, data)) {
        return true;
    }
    return false;
}
bool CTarget::read_param(int params,unsigned short &value)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(0)) return false;
    sprintf_s(cmd,dim(cmd) ,"nvm:cal_prodparm.cust_parms.param_%d?",params);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitWifiCalResult-->get cust_parms.param_2 failed\r\n"));
	    return false;
    }
    value = atoi(resp);
    return true;
}
bool CTarget::write_param(int params,unsigned short value)
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(0)) return false;

    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_%d=%d",params,value);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitWifiCalResult-->get cust_parms.param_2 failed\r\n"));

	    return false;
    }
	sprintf_s(cmd,dim(cmd) ,"at@nvm:store_nvm_sync(cal_prodparm)");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitWifiCalResult-->store_nvm(cal_prodparm) failed\r\n"));

	    return false;
    }
#if 0
	sprintf_s(cmd,dim(cmd) ,"at@nvm:wait_nvm_idle()");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LDEGMSG(DEBUG_TARGET,("Error:CommitRfResult-->wait_nvm_idle failed\r\n"));
	    return false;
    }
#else 
    wait_nvm_stored();
#endif
	sprintf_s(cmd,dim(cmd) ,"nvm:cal_prodparm.cust_parms.param_%d?",params);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitWifiCalResult-->get cust_parms.param_2 failed\r\n"));
	    return false;
    }

    if (value == atoi(resp)) return true;
    return false;

}
bool CTarget::CommitWifiCalResult()
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(0)) return false;

    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_2=1");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitWifiCalResult-->get cust_parms.param_2 failed\r\n"));

	    return false;
    }
	sprintf_s(cmd,dim(cmd) ,"at@nvm:store_nvm_sync(cal_prodparm)");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitWifiCalResult-->store_nvm(cal_prodparm) failed\r\n"));

	    return false;
    }
#if 0
	sprintf_s(cmd,dim(cmd) ,"at@nvm:wait_nvm_idle()");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LDEGMSG(DEBUG_TARGET,("Error:CommitRfResult-->wait_nvm_idle failed\r\n"));
	    return false;
    }
#else 
    wait_nvm_stored();
#endif
	sprintf_s(cmd,dim(cmd) ,"at@nvm:cal_prodparm.cust_parms.param_2?");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitWifiCalResult-->get cust_parms.param_2 failed\r\n"));
	    return false;
    }
    if (1 == atoi(resp)) return true;
    return false;
}
bool CTarget::wait_nvm_stored()
{
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    size = dim(resp);
#if 0
    if (gti_generic(0, strlen("at@nvm:wait_nvm_stored()??"), "at@nvm:wait_nvm_stored()??", &size, resp, 0) 
        && (strstr(resp, "store requests") != 0)) {
        size = dim(resp);
        if (!gti_generic(0, strlen("at@nvm:wait_nvm_stored(nvm_calib)"), "at@nvm:wait_nvm_stored(nvm_calib)", &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
            LOGER((CLogger::DEBUG_DUT,"Error:CommitRfResult-->wait_nvm_stored failed\r\n"));
            return false;
        }
    } else
#endif
    {
        size = dim(resp);
        if (!gti_generic(0, strlen("at@nvm:wait_nvm_idle()"), "at@nvm:wait_nvm_idle()", &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
            LOGER((CLogger::DEBUG_DUT,"Error:CommitRfResult-->wait_nvm_idle failed\r\n"));
            return false;
        }
    }
    /* m_Conn.timeout(0); **/
    return true;
}
bool CTarget::CommitRfResult()
{
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(0)) return false;
    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_1=1");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitRfResult-->set cust_parms.param_1 failed\r\n"));
	    return false;
    }
    
	sprintf_s(cmd,dim(cmd) ,"at@nvm:store_nvm_sync(cal_prodparm)");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitRfResult-->store_nvm(cal_prodparm) failed\r\n"));
	    return false;
    }

#if 0
	sprintf_s(cmd,dim(cmd) ,"at@nvm:wait_nvm_idle()");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LDEGMSG(DEBUG_TARGET,("Error:CommitRfResult-->wait_nvm_idle failed\r\n"));
	    return false;
    }
#else 
    wait_nvm_stored();
#endif
    sprintf_s(cmd,dim(cmd),"nvm:cal_prodparm.cust_parms.param_1?");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitRfResult-->get cust_parms.param_1 failed\r\n"));
	    return false;
    }
    
    if (1 == atoi(resp)) return true;
	return false;
}
bool CTarget::store_nvm()
{
    char            cmd[2048];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    unsigned short  values[80] = {0};
    if(!ConnectRouter(0)) return false;
#if 0
    sprintf_s(cmd,dim(cmd) ,"at@nvm:cal_prodparm.cust_parms.flex_ver?");
    size = dim(resp);
    if (gti_generic(0, strlen(cmd), cmd, &size, resp, 0) && (strstr(resp, "ERR") == 0)) {
        std::string str = resp;
        trim(str,"\"");
        for(int j = 0; j < str.size();j ++) {
            values[j] = str[j];
        }
        char *p = cmd;
        p += sprintf(p,"cal_prodparm.cust_parms.flex_ver={%d",values[0]);
        for(int i= 1;i < 79 ; i ++) {
            p += sprintf(p,",%d",values[i]);
        }
        p += sprintf(p,",%d}",values[79]);
        size = dim(resp);
        if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
            LOGER((CLogger::DEBUG_DUT,"cal_prodparm.cust_parms.flex_ver failed\r\n"));
    	    return false;
        }        
	}
    /*write some more to save for data missing if in nand devices **/
	sprintf_s(cmd,dim(cmd) ,"at@nvm:cal_prodparm.cust_serial.dev=\"sofia3grrockchipsintelx86dev\"");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:store_nvm() failed\r\n"));
	    return false;
    }
	sprintf_s(cmd,dim(cmd) ,"at@nvm:cal_prodparm.cust_serial.dev=\"sofia3grrockchipsintelx86pcb\"");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:store_nvm() failed\r\n"));
	    return false;
    }

#endif
	sprintf_s(cmd,dim(cmd) ,"at@nvm:store_nvm_sync(cal_prodparm)");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:CommitRfResult-->store_nvm_sync(cal_prodparm) failed\r\n"));
	    return false;
    }
#if 1
	sprintf_s(cmd,dim(cmd) ,"at@nvm:wait_nvm_idle()");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0)||(strstr(resp, "ERR") != 0)) {
	    return false;
    }
#else 
    if(!wait_nvm_stored()) return false;
#endif

	return true;
}

bool CTarget::DoWifiSelfCal(int nEvmLimit[],int nFELossCH[])
{
    short  tnEvmLimit[10];
    short  tnFELossCH[13];
    char            cmd[1024];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size;
    if(!ConnectRouter(1)) return false;

    /*evm_limit_set **/
	sprintf_s(cmd, dim(cmd),"at@wlan_mvt:evm_limit_set({%d,%d,%d,%d,%d,%d,%d,%d,%d,%d})", nEvmLimit[0],
     nEvmLimit[1],nEvmLimit[2],nEvmLimit[3],nEvmLimit[4],nEvmLimit[5],nEvmLimit[6],nEvmLimit[7],
     nEvmLimit[8],nEvmLimit[9]);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->evm_limit_set failed\r\n"));
        return false;
	}

	sprintf_s(cmd,  dim(cmd),"at@wlan_mvt:evm_limit_get()");
    size = dim(resp);
    if (!gti_read_s(cmd,10,tnEvmLimit)  ) {
        LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->evm_limit_get failed\r\n"));
        return false;
	}
    for(int i = 0; i < dim(tnEvmLimit) ;i ++ ) {
        if(nEvmLimit[i] != tnEvmLimit[i]) {
            LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->evm_limit_get mismatch,failed\r\n"));
            return false;
        }
    }

    /*fe_loss_set **/
	sprintf_s(cmd, dim(cmd),"at@wlan_mvt:fe_loss_set({%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d})", nFELossCH[0],
     nFELossCH[1],nFELossCH[2],nFELossCH[3],nFELossCH[4],nFELossCH[5],nFELossCH[6],nFELossCH[7],
     nFELossCH[8],nFELossCH[9],nFELossCH[10],nFELossCH[11],nFELossCH[12]);
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->fe_loss_set failed\r\n"));
        return false;
	}

	sprintf_s(cmd,  dim(cmd),"at@wlan_mvt:fe_loss_get()");
    size = dim(resp);
    if (!gti_read_s(cmd,13,tnFELossCH)  ) {
        LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->fe_loss_get failed\r\n"));
        return false;
	}
    for(int i = 0; i < dim(tnFELossCH) ;i ++ ) {
        if(nFELossCH[i] != tnFELossCH[i]) {
            LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->fe_loss_set mismatch,failed\r\n"));
            return false;
        }
    }

	/*commit **/
	sprintf_s(cmd, dim(cmd),"at@wlan_mvt:nvm_commit()");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->nvm_commit failed\r\n"));
        return false;
	}

	sprintf_s(cmd, dim(cmd),"at@ROCS:LoadAndStartFlow(\"WLAN_SELF_CAL\")");
    size = dim(resp);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->LoadAndStartFlow failed\r\n"));
        return false;
	}
    std::string         readvalue(resp);
    std::vector<std::string> vecString;
    int                 pos     =0;
    int                 i       =0;
    readvalue.erase(0, readvalue.find_first_not_of(" ")); /*remove space **/
	vecString.clear();
    for ( i=0;i<readvalue.length();i++) {
        if (readvalue[i] == _T('\"')) {
            pos = i+1;
            while (readvalue[++i] != _T('\"')); 
            vecString.push_back(readvalue.substr(pos, i-pos));
            pos = ++i;
        } else if (readvalue[i] == _T(' ')) {
            /*Entering the previous work into the list **/
            vecString.push_back(readvalue.substr(pos, i-pos));
            pos = i + 1;     			
        }		
    }
    if (vecString.size()>3) {
        int i1 = atoi(vecString[0].c_str());
        int i2 = atoi(vecString[3].c_str());
        if ((atoi(vecString[0].c_str())==0)||(atoi(vecString[3].c_str())==0)) {
            LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->WLAN_SELF_CAL failed,err=%s\r\n",readvalue.c_str()));
            return false;
        }
    } else {
        LOGER((CLogger::DEBUG_DUT,"Error:DoWifiSelfCal-->WLAN_SELF_CAL failed,err=%s\r\n",readvalue.c_str()));
        return false;
    }
    return true;
}
bool CTarget::ReadMac(std::string & wifi_mac,std::string & bt_mac)
{
    char            strmac[20];
    unsigned char   mac[6];
    LOGER((CLogger::DEBUG_DUT,"ReadMac()\r\n"));
    if(!ConnectRouter(1)) return false;
    if(1) {
        memset(strmac,0,sizeof(strmac));
        if(!ReadWifiMac(mac)) {
            return false;
        }
        for(int i = 0; i < 6 ; i++ ) {
            strmac[2*i]     = value2hexchar((mac[i]>>4)&0xf);
            strmac[2*i +1 ] = value2hexchar(mac[i]&0xf);
        }
        wifi_mac.assign(strmac);
    }
    if(1) {
        memset(strmac,0,sizeof(strmac));
        if(!ReadBtMac(mac)) {
            return false;
        }
        for(int i = 0; i < 6 ; i++ ) {
            strmac[2*i]     = value2hexchar((mac[5 - i]>>4)&0xf);
            strmac[2*i +1 ] = value2hexchar(mac[5 - i]&0xf);
        }
        bt_mac.assign(strmac);
    }
    return true;
}
bool CTarget::ReadBtMac(unsigned char bt_mac[])
{
    unsigned short  mac[6];
    LOGER((CLogger::DEBUG_DUT,"++ReadBtMac()\r\n"));
    if(NULL == bt_mac ) return false;
    if(gti_read_us("BT:read_nvm_bdaddr()",6,mac)) {
        for(int i = 0; i < 6;i ++) {
            bt_mac[i] = mac[i];
        }
        return true;
    } 
    return false;
}
bool CTarget::ReadWifiMac(unsigned char wifi_mac[])
{
    unsigned short  mac[6];

    LOGER((CLogger::DEBUG_DUT,"++ReadWifiMac()\r\n"));
    if(NULL == wifi_mac ) return false;
    if(gti_read_us("wlan_mvt:read_mac_address_command()",6,mac)) {
        for(int i = 0; i < 6;i ++) {
            wifi_mac[i] = mac[i];
        }
        return true;
    } 
    return false;
}
bool CTarget::WriteBtMac(unsigned char bt_mac[],bool *bskip_force_btmac)
{
    bool            bforce_btmac;
    char            cmd[256];
    char            resp[GTI_RESPONSE_SIZE];
    unsigned short  size        = GTI_RESPONSE_SIZE;
    unsigned short  mac[6];
    bforce_btmac          = *bskip_force_btmac;
    LOGER((CLogger::DEBUG_DUT,"++ WriteBtMac()\r\n"));
    if(!bforce_btmac) {
        if(gti_read_us("BT:read_nvm_bdaddr()",6,mac)) {
            *bskip_force_btmac = true;
            return true;
        } 
    }

    /*write bt mac **/
    sprintf_s(cmd,dim(cmd),"BT:write_nvm_bdaddr({%d,%d,%d,%d,%d,%d})",
        bt_mac[5],bt_mac[4],bt_mac[3],
        bt_mac[2],bt_mac[1],bt_mac[0]);
    if (!gti_generic(0, strlen(cmd), cmd, &size, resp, 0) || (strstr(resp, "ERR") != 0)) {
        return false;
	}
 
    /*read bt mac and compare **/
    if(gti_read_us("BT:read_nvm_bdaddr()",6,mac)) {
        int i;
        for(i = 0;i < 6 ; i++ ) {
            if (mac[i] != bt_mac[5 - i]) {
                break;
            }
        }
        if(6 == i) {
            *bskip_force_btmac = false;
            return true;
        } 
    }
    return false;
}



bool CTarget::SendRecvCmd(const char * buf,unsigned short *nof_output_bytes, char *poutput )
{
    LOGER((CLogger::DEBUG_DUT,"++SendRecvCmd()\r\n"));
    if(NULL == buf || 2 >= strlen(buf)) return false;
    if (gti_generic(0, strlen(buf), buf, nof_output_bytes, poutput, 0) && (strstr(poutput, "ERR") == 0)) {
        return true;
    }
    return false;
}
/*
bool ConnectRouter(CTarget &dut,int iRoute)
{
    if(dut.ConnectRouter(iRoute)) return true;
    return false;
}
**/
bool ConnectDUT(CTarget &dut,int nPort,bool bGtiPort,CLogger *m_pLogger)
{
    int     iRetry;
    bool    handshakeok;

    LOGER((CLogger::DEBUG_DUT,"Connecting DUT(port=%d)...\r\n",nPort));
    if (!dut.Open(nPort)) {
        dut.Close();
        LOGER((CLogger::DEBUG_DUT,"Failed to connect DUT\r\n"));
        return false;
    }
    LOGER((CLogger::DEBUG_DUT,"DUT connected\r\n"));
    iRetry = 0;
    while( iRetry < 1) {
        iRetry ++;
        handshakeok = dut.Handshake(bGtiPort);
        if(handshakeok) break;
        Sleep(1000);
	}
    if (!handshakeok) {
        dut.Close();
        LOGER((CLogger::DEBUG_DUT,"Failed to handshake with DUT\r\n"));
	    return false;
    }
    LOGER((CLogger::DEBUG_DUT,"handshake with DUT ok %d\r\n",iRetry));
    LOGER((CLogger::DEBUG_DUT,"Connect DUT successfully\r\n"));
	return true;
}
bool DisConnectDUT(CTarget &dut)
{

    /*LOGER((CLogger::DEBUG_DUT,"DisConnectDUT DUT\r\n")); **/
    if (!dut.Close()) {
        /*LOGER((CLogger::DEBUG_DUT,"Failed to DisConnect DUT\r\n")); **/
        return false;
    }
    /*LOGER((CLogger::DEBUG_DUT,"DisConnectDUT DUT successfully\n")); **/
	return true;
}
bool ReBoot(int nCom)
{
    CTarget dut;
    if(ConnectDUT(dut, nCom,true)) {
        /*LOGER((CLogger::DEBUG_DUT,"ConnectDUT(%d) successfully\r\n",nCom)); **/
        if( dut.reboot()) {
            /*LOGER((CLogger::DEBUG_DUT,"reboot successfully\r\n")); **/
            return false;
        } else {
            /*LOGER((CLogger::DEBUG_DUT,"reboot failed\r\n")); **/
        }
    }
    return false;
}
/*end */

bool WriteCheckIMEI(std::wstring &szImei1,std::wstring &szImei2,int nCom,std::wstring &dir,bool bIs6321Dev,bool bUseGtiCom,CLogger *m_pLogger)
{
    bool            bRet     = false;
    bool            result   = false;
    /*int             iret     = -1; **/
    CSpawn          ShellSpawn;
    wchar_t           cmd[256];
    char            buf[8192];
    const wchar_t   *path;
    char            *newokmsg = "Operation completed with success.";
    /*DWORD           dwExitCode; **/
    DWORD           dwLen   = sizeof(buf)/sizeof(buf[0]);
    CTarget         dut;
    if(szImei1.empty()) {    
        return false;
    }
    path = dir.c_str();
    if(szImei2.empty()) {
        if( bIs6321Dev){
			wsprintf(cmd,TEXT("\"%ssec_provision.exe\" -c%d -i1 %s -k \"%sOldSprk2048dev.key\" -u \"%sunlockcodes.txt\""),
				path, nCom,szImei1.c_str(),path,path);
        } else {
            wsprintf(cmd,TEXT("\"%ssec_provision.exe\" -c%d -i1 %s -k \"%sSprk2048dev.key\""),
                path, nCom,szImei1.c_str(),path);
        } 
    } else {
        if(bIs6321Dev ) {
			wsprintf(cmd,TEXT("\"%ssec_provision.exe\" -c%d -i2 %s %s -k \"%sOldSprk2048dev.key\" -u \"%sunlockcodes.txt\""),
				path, nCom,szImei1.c_str(),szImei2.c_str(),path,path);    
        } else  {
            wsprintf(cmd,TEXT("\"%ssec_provision.exe\" -c%d -i2 %s %s -k \"%sSprk2048dev.key\""),
                path, nCom,szImei1.c_str(),szImei2.c_str(),path);
  
        } 
    }
    if(m_pLogger) {
        m_pLogger->PrintMSGW(CLogger::DEBUG_DUT,cmd );
    }
    /*LDEGMSGW((CLogger::DEBUG_DUT,TEXT("%s\r\n"),cmd)); **/
    memset(buf,0,sizeof(buf));
    if(ShellSpawn.Exe(cmd,25000,true)) {
        char        *strRet = ShellSpawn.GetResultStr();
        unsigned int nRet   = ShellSpawn.GetResult();
        if(strRet) {
            LOGER((CLogger::DEBUG_DUT    ,"I:\r\n%s\r\nexit code=%d\r\n",     strRet  ,nRet));
            if( strstr(strRet,newokmsg) ) {
                /*LOGER((CLogger::DEBUG_DUT,"O:\r\n%s\r\nexit code=%d\r\n",  newokmsg,nRet)); **/
            } else {
                /*LOGER((CLogger::DEBUG_DUT,"E:\r\n%s\r\nexit code=%d\r\n",  newokmsg,nRet)); **/
                goto writecheckimei_exit;
            }
        } else {
            LOGER((CLogger::DEBUG_DUT,    "E:exit code=%d\r\n",nRet));
            goto writecheckimei_exit;
        }
    }
    dut.StartLog(m_pLogger);
    if(!ConnectDUT(dut, nCom,bUseGtiCom)) {
        LOGER((CLogger::DEBUG_DUT,"ConnectDUT(%d) failed\r\n",nCom));
        goto writecheckimei_exit;

    }
    if( dut.CheckImei(CTarget::wstr2str(szImei1),CTarget::wstr2str(szImei2))) {
        LOGER((CLogger::DEBUG_DUT,"CheckImei successfully\r\n"));
    } else {
        LOGER((CLogger::DEBUG_DUT,"CheckImei failed\r\n"));
        goto writecheckimei_exit;
    }
    result = true;
writecheckimei_exit:
    return result;
}

void CTarget::trim(std::string& input ,std::string whitespace )
{
	int start_pos   = 0;
	int end_pos     = 0;
	for (unsigned int i = 0; i < input.size(); ++i)
	{
		if (whitespace.find(input[i]) != std::string::npos)
		{
			start_pos++;
		}
		else
		{
			break;
		}
	}
	for (int i = input.size() - 1; i >= 0; --i)
	{
		if (whitespace.find(input[i]) != std::string::npos)
		{
			end_pos++;
		}
		else
		{
			break;
		}
	}
    input = input.substr(start_pos, input.size() - start_pos - end_pos);
}
// Convert wstring to string
std::string CTarget::wstr2str(const std::wstring& arg)
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
// Convert string to wstring
std::wstring CTarget::str2wstr(const std::string& arg)
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



