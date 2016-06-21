
// wvpctoolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wvpctool.h"
#include "wvpctoolDlg.h"
#include <Dbt.h>
#include "..\targetdevice\EnumSerial.h"
#include "stdint.h"
#include ".\file\FileIO.h"
#define SUB_MENU_SETTING 1
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang);
// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
private:
    CLabel          m_rklink;
    CLabel          m_AppName;
    CIniSettingBase &m_Configs;
    CIniLocalLan    &m_LocalLang;
// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    std::wstring GetLocalString(std::wstring strKey)
    {
        return m_LocalLang.GetLanStr(strKey);
    }
};

CAboutDlg::CAboutDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang) : CDialog(CAboutDlg::IDD)
    ,m_Configs(Configs),m_LocalLang(LocalLang)
{
}
void CAboutDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    OnOK();
}
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_rklink.SetTransparent(TRUE);
	m_rklink.SetFontSize(8);
	m_rklink.SetFontBold(TRUE);
    m_rklink.SetText(TEXT("福州瑞芯微电子有限公司"), 0xFF1111);
    m_rklink.SetLink(TRUE,FALSE);
    m_rklink.SetHyperLink(CString(TEXT("http://www.rock-chips.com/")));
    /*m_rklink.SetLinkCursor( (HCURSOR)IDC_IBEAM); **/
    /*m_rklink.FlashText(TRUE);**/
    m_AppName.SetTransparent(TRUE);
    m_AppName.SetFontSize(8);
    m_AppName.SetFontBold(TRUE);
    m_AppName.SetText((GetLocalString(TEXT("IDS_TEXT_APPNAME"))+ TEXT(APP_VERSION)).c_str(), 0xFF1111);
    /*GetDlgItem(IDC_STATIC_APPNAME)->SetWindowText((m_LocalLang.GetStr(TEXT("APPNAME")) + TEXT(APP_VERSION)).c_str());**/
    return FALSE;
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_RK, m_rklink);
    DDX_Control(pDX, IDC_STATIC_APPNAME, m_AppName);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CwvpctoolDlg dialog
UINT ScanDeviceThread(LPVOID lpParam)
{
	CwvpctoolDlg *pDlg = (CwvpctoolDlg *)lpParam;
	pDlg->ScanThreadProc();
    /*pDlg->m_hScanThread = NULL; **/
	return 0;
}

UINT TestDeviceThread(LPVOID lpParam)
{
	CwvpctoolDlg *pDlg  = (CwvpctoolDlg*)lpParam;
    pDlg->WorkThreadProc();

	return 0;
}
BOOL CwvpctoolDlg::EnableIpcsd(int nAtPort,CLogger *logger)
{
    CTarget dut;
    dut.StartLog(logger);
    if(!ConnectDUT(dut,nAtPort,false)) {
        return false;
    }
    return dut.EnableIpcsd();
}
BOOL CwvpctoolDlg::RetryAndOpen(CTarget &dut,int nGtiCom,int nAtCom)
{
    int iRetry = 0;
    if( -1 == nAtCom ||  -1 == nGtiCom )  return false;
    while( iRetry < 6 ) {
        iRetry++;
        if (m_bUserStop) {
            goto retry_and_open;
        }
        if(!EnableIpcsd(nAtCom,m_pLog)) {
            continue;
        }
        if(ConnectDUT(dut, nGtiCom,true)) {
            return true;
        }
        Sleep(1000);
    }
retry_and_open:
    return false;
}
/*#define NO_DEVICE **/

BOOL CwvpctoolDlg::parseresult(std::string str,unsigned int *fus_action,unsigned int *fus_value,unsigned int *result_cause)
{
    LDEGMSG((CLogger::DEBUG_DUT,"parseresult %s\r\n",str.c_str()));
    std::size_t len         = str.size();
    std::size_t funpos      = str.find("fus_action = ");
    std::size_t valuepos    = str.find("fus_value = ");
    std::size_t resultpos   = str.find("result_cause = ");
    if((std::string::npos != funpos)&&(std::string::npos != resultpos )&&(std::string::npos != valuepos ) &&(funpos + 13 < len)&&(valuepos + 12 < len)&&(resultpos + 15 < len)) {
        *fus_action     = atof(str.substr(funpos    + 13).c_str());
        *fus_value      = atof(str.substr(valuepos  + 12).c_str());
        *result_cause   = atof(str.substr(resultpos + 15).c_str());
        LDEGMSG((CLogger::DEBUG_DUT,"nfunid=%x,nvalue=%x,nresult=%x\r\n",*fus_action,*fus_value,*result_cause));
        return TRUE;
    }
    return FALSE;
}

BOOL CwvpctoolDlg::LoadScript(CTarget &dut,SCRIPTARRY &scriptarry)
{
    BOOL            bRet        = FALSE;
    int             AtCmdIdx;
    char            AtCmd[128];
    int             NofCmd;
    NofCmd          = scriptarry.size();

    sprintf(AtCmd,"at@");
    if(!dut.SendAtCmd(AtCmd)) {
        goto exit;
    }
  
    sprintf(AtCmd,"at@sec:new(\"%%&lu[%d]:_io.fuse\",1)",NofCmd*4);
    if(!dut.SendAtCmd(AtCmd)) {
        goto exit;
    }

    for(AtCmdIdx = 0;AtCmdIdx < NofCmd ;AtCmdIdx ++ ) {
        sprintf(AtCmd,"at@sec:_io.fuse[%d,%d]={0x%08x,0x%08x,0x%08x,0x%08x}",AtCmdIdx*4,AtCmdIdx*4 + 3,
            scriptarry[AtCmdIdx].function,scriptarry[AtCmdIdx].regid,scriptarry[AtCmdIdx].bitmask,scriptarry[AtCmdIdx].value);        
        if(!dut.SendAtCmd(AtCmd)) {
            goto exit;
        }
    }
    bRet = TRUE;
exit:
    return bRet;
}
BOOL CwvpctoolDlg::DoAttachWithValue(CTarget &dut,SCRIPTARRY &scriptarry)
{
    T_FUS_SCRIPT    tscript;
    unsigned int    fus_action;
    unsigned int    fus_value;
    unsigned int    result_cause;
    BOOL            bRet        = FALSE;
    int             AtCmdIdx;
    unsigned short  nof_output_bytes;
    char            poutput[512];
    scriptarry.clear();

    nof_output_bytes    = sizeof(poutput)/sizeof(poutput[0]);
    if(dut.SendAtCmdWithResp("at@sec:fus_script(_io.fuse)",&nof_output_bytes,poutput)) {
        if(parseresult(poutput,&fus_action,&fus_value,&result_cause)) {
            if(FUS_RES_SUCCESS == result_cause) {
                if(dut.SendAtCmd("at@sec:!")) {
                    tscript.bitmask = fus_value;
                    scriptarry.push_back(tscript);
                    goto step2;
                }
            } else if (FUS_RES_SCRIPT_PENDING == result_cause) {
                Sleep(500);
                
            }
        }
        dut.SendAtCmd("at@sec:!");
    }
    goto exit;
step2:
    while(1) {

        nof_output_bytes = sizeof(poutput)/sizeof(poutput[0]);
        if(dut.SendAtCmdWithResp("at@sec:fus_attach()",&nof_output_bytes,poutput)) {
            if(parseresult(poutput,&fus_action,&fus_value,&result_cause)) {
                if(ZERO != fus_action && FUS_RES_SUCCESS == result_cause) {
                    if(dut.SendAtCmd("at@sec:!")) {
                        tscript.bitmask= fus_value;
                        scriptarry.push_back(tscript);
                        continue;   /*all command ok ,just continue **/
                    }
                } else if(ZERO  == fus_action && FUS_RES_SUCCESS == result_cause)  {
                    bRet = TRUE;
                }
                dut.SendAtCmd("at@sec:!");
            } 
        } 
        break;
    }
exit:
    return bRet;

}
BOOL CwvpctoolDlg::DoAttach(CTarget &dut,const SCRIPTARRY &scriptarry)
{
    T_FUS_SCRIPT    tscript;
    unsigned int    fus_action;
    unsigned int    fus_value;
    unsigned int    result_cause;
    BOOL            bRet        = FALSE;
    int             AtCmdIdx;
    unsigned short  nof_output_bytes;
    char            poutput[512];
    nof_output_bytes    = sizeof(poutput)/sizeof(poutput[0]);
    if(dut.SendAtCmdWithResp("at@sec:fus_script(_io.fuse)",&nof_output_bytes,poutput)) {
        if(parseresult(poutput,&fus_action,&fus_value,&result_cause)) {
            if(FUS_RES_SUCCESS == result_cause) {
                if(dut.SendAtCmd("at@sec:!")) {
                    goto step2;
                }
            } else if (FUS_RES_SCRIPT_PENDING == result_cause) {
                Sleep(500);
                
            }
        }
        dut.SendAtCmd("at@sec:!");
    }
    goto exit;
step2:
    while(1) {

        nof_output_bytes = sizeof(poutput)/sizeof(poutput[0]);
        if(dut.SendAtCmdWithResp("at@sec:fus_attach()",&nof_output_bytes,poutput)) {
            if(parseresult(poutput,&fus_action,&fus_value,&result_cause)) {
                if(ZERO != fus_action && FUS_RES_SUCCESS == result_cause) {
                    if(dut.SendAtCmd("at@sec:!")) {
                        continue;   /*all command ok ,just continue **/
                    }
                } else if(ZERO  == fus_action && FUS_RES_SUCCESS == result_cause)  {
                    bRet = TRUE;
                }
                dut.SendAtCmd("at@sec:!");
            } 
        } 
        break;
    }
exit:
    return bRet;
}
BOOL CwvpctoolDlg::WorkThreadProc()
{
    BOOL            bFuseRead,bReadCheck;
    BOOL            bVerbose;
    unsigned short  value;
    BOOL            bRet;
	string          strsn,strsn_ignore;
    CString         strPrompt;  
    CTarget         dut;
    int             nAtCom      = _ttoi(m_ports.IntelPort[PORT_AT].c_str());
    int             nCom        = _ttoi(m_ports.IntelPort[PORT_GTI].c_str());
    BOOL            bSuccsee    = FALSE;
    if (m_listInfo.GetCount()>0) {
        PostMessage(WM_UPDATE_MSG,UPDATE_LIST,LIST_EMPTY);
    }

    /*start...**/
    dut.StartLog(m_pLog);
    dut.SetDeviceType(DEVTYPE_SOFIA3GR);

    /*step 1 connect device **/
    strPrompt.Format(GetLocalString(_T("IDS_INFO_CONN_DEVICE")).c_str(),nCom);
    AddPrompt(strPrompt,LIST_INFO);
#ifndef NO_DEVICE
    if(!RetryAndOpen(dut,nCom, nAtCom)) {
        LDEGMSG((CLogger::DEBUG_INFO,"ConnectDUT(%d) failed\r\n",nCom));
        AddPrompt(GetLocalString(TEXT("IDS_ERROR_CONNECT_FAIL")).c_str(),LIST_ERR);
        goto exitworkthreadproc;
    }
    LDEGMSG((CLogger::DEBUG_INFO,"ConnectDUT(%d) successfully\r\n",nCom));

    /*step 2 read sn **/
    /*{**/
    if(2 == m_Configs.nSnType) { /*hw_sn **/
        AddPrompt(GetLocalString(TEXT("IDS_INFO_READHWSN")).c_str(),LIST_INFO);
        bRet = dut.ReadHwId(strsn);
    } else if(1 == m_Configs.nSnType){
        AddPrompt(GetLocalString(TEXT("IDS_INFO_READPCBSN")).c_str(),LIST_INFO);
        bRet = dut.ReadSn(strsn, strsn_ignore);
    } else {
        AddPrompt(GetLocalString(TEXT("IDS_INFO_READDEVSN")).c_str(),LIST_INFO);
        bRet = dut.ReadSn(strsn_ignore, strsn);
    }
    if(!bRet) {
        LDEGMSG((CLogger::DEBUG_INFO,"ReadSn fail\r\n"));
        AddPrompt(GetLocalString(TEXT("IDS_ERROR_READSN")).c_str(),LIST_ERR);
        goto exitworkthreadproc;
    }
	strPrompt.Format(TEXT("SN:%s"),CSettingBase::str2wstr(strsn).c_str());
    AddPrompt(strPrompt,LIST_INFO);
#else 
    strsn = "16F06AA000001";
#endif
    /*}**/

    /*doing widevine **/
    if(m_Configs.bWideVine&&!m_Configs.bWideVineRead) {
        char szBase64[256];
        int  Base64len;
        BOOL bWvResult;
        AddPrompt(GetLocalString(_T("IDS_INFO_KEYBOX_PREPARE")).c_str(),LIST_INFO);

        /*step + 1 check whether it's already write keybox or not **/
        if(!m_Configs.bForceWrite) {
            AddPrompt(GetLocalString(TEXT("IDS_INFO_WRITE_CHECK")).c_str(),LIST_INFO);
            if(!dut.read_param(4,value)) {
                AddPrompt(GetLocalString(TEXT("IDS_ERROR_WRITE_CHECK")).c_str(),LIST_ERR);
                goto exitworkthreadproc;
            }
            if(1 == (value&1)) {
                AddPrompt(GetLocalString(TEXT("IDS_INFO_WRITE_SKIP")).c_str(),LIST_INFO);
                goto tag_fuse;
            }
        }

        memset(szBase64,0,nof(szBase64));
        Base64len = -1;
        if(m_Configs.bUseDB) {
            AddPrompt(GetLocalString(_T("IDS_INFO_GETWVKEY_FROMDB")).c_str(),LIST_INFO);
            if(!LoadFromDB(szBase64,&Base64len,NULL,CSettingBase::str2wstr(strsn).c_str(),m_Configs.bIsBin)) {
                AddPrompt(GetLocalString(TEXT("IDS_ERROR_LOADXML")).c_str(),LIST_ERR);
                goto exitworkthreadproc;
            }
        } else {
            AddPrompt(GetLocalString(_T("IDS_INFO_GETWVKEY_FROMFILE")).c_str(),LIST_INFO);
            if(!LoadFromFile(m_Configs.strXmlFileName.c_str(),szBase64,&Base64len,m_Configs.bIsBin)) {
                AddPrompt(GetLocalString(TEXT("IDS_ERROR_LOADXML")).c_str(),LIST_ERR);
                goto exitworkthreadproc;
            }
        }
        szBase64[Base64len] = '\0';
#ifndef NO_DEVICE
        AddPrompt(GetLocalString(_T("IDS_INFO_KEYBOX_WRITING")).c_str(),LIST_INFO);
        bWvResult = dut.DoWvKeyBox(szBase64);
#else 
        bWvResult = TRUE;
#endif
        if(!bWvResult) {
            LDEGMSG((CLogger::DEBUG_INFO,"DoWvKeyBox failed\r\n"));
            AddPrompt(GetLocalString(_T("IDS_ERROR_KEYBOX_FAIL")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }
        AddPrompt(GetLocalString(TEXT("IDS_INFO_WRITE_SAVE")).c_str(),LIST_INFO);
        if(!dut.write_param(4,1)) {
            AddPrompt(GetLocalString(TEXT("IDS_ERROR_WRITE_SAVE")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }

        if(m_Configs.bUseDB) { /*success or fail ,must commit to used or free wvkey **/
            AddPrompt(GetLocalString(_T("IDS_INFO_COMMIT_TODB")).c_str(),LIST_INFO);
		    if(!m_WvToolDB.CommitWVkey(bWvResult, CSettingBase::str2wstr(strsn)) ){
                AddPrompt(GetLocalString(_T("IDS_ERROR_COMMIT_FAIL")).c_str(),LIST_INFO);
                goto exitworkthreadproc;
            }
        }
        AddPrompt(GetLocalString(TEXT("IDS_INFO_WVBOXBIN_PASS")).c_str(),LIST_INFO);

    } else if(m_Configs.bWideVine&&m_Configs.bWideVineRead) {
        AddPrompt(GetLocalString(_T("IDS_INFO_GET_KEYBOXSTATUS")).c_str(),LIST_INFO);
        if(!dut.read_param(4,value)) {
            AddPrompt(GetLocalString(TEXT("IDS_ERROR_GET_KEYBOXSTATUS")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }
        if(1 == (value&1)) {
            AddPrompt(GetLocalString(TEXT("IDS_INFO_KEYBOX_YES")).c_str(),LIST_TIME);
            goto tag_fuse;
        } else if(0 == (value&1)) {
            AddPrompt(GetLocalString(TEXT("IDS_INFO_KEYBOX_NO")).c_str(),LIST_WARN);
            goto tag_fuse;
        }
    }
tag_fuse:
    bFuseRead = m_Configs.bFuseRead;
    bReadCheck= FALSE;
    if(m_Configs.bFuse&&(!bFuseRead)) { /*do fuse and just write **/
        AddPrompt(GetLocalString(_T("IDS_INFO_FUSE_PREPARE")).c_str(),LIST_INFO);

        AddPrompt(GetLocalString(_T("IDS_INFO_LOAD_SCRIPT")).c_str(),LIST_INFO);
        if(!LoadScript(dut,m_ScriptArry)) {
            AddPrompt(GetLocalString(_T("IDS_ERROR_LOAD_SCRIPT")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }

        AddPrompt(GetLocalString(_T("IDS_INFO_FUSE_WAIT_ATTACH")).c_str(),LIST_INFO);
        if(!DoAttach(dut,m_ScriptArry)) {
            /*AddPrompt(GetLocalString(_T("IDS_ERROR_FUSE_ATTACH_FAIL")).c_str(),LIST_ERR); **/
            AddPrompt(GetLocalString(_T("IDS_INFO_FUSE_CHECK")).c_str(),LIST_INFO);
            bReadCheck = TRUE; /*fuse error occur,try to read back to check  **/
        } else {
            AddPrompt(GetLocalString(TEXT("IDS_INFO_FUSE_ATTACH_OK")).c_str(),LIST_INFO);
        }
    }
    if(bFuseRead || bReadCheck ) {
        SCRIPTARRY scriptarry;
        if(bFuseRead) {
            AddPrompt(GetLocalString(_T("IDS_INFO_READ_PREPARE")).c_str(),LIST_INFO);
        } else {
            /*AddPrompt(GetLocalString(_T("IDS_INFO_FUSE_CHECK")).c_str(),LIST_INFO); **/
        }
        for(int st = 0; st < sizeof(readscript)/sizeof(readscript[0]);st ++ ){
            scriptarry.push_back(readscript[st]);
        }
        AddPrompt(GetLocalString(_T("IDS_INFO_LOAD_SCRIPT")).c_str(),LIST_INFO);
        if(!LoadScript(dut,scriptarry)) {
            AddPrompt(GetLocalString(_T("IDS_ERROR_LOAD_SCRIPT")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }

        AddPrompt(GetLocalString(_T("IDS_INFO_READ_VALUE")).c_str(),LIST_INFO);
        if(!DoAttachWithValue(dut,scriptarry)) {
            AddPrompt(GetLocalString(_T("IDS_ERROR_READ_VALUE")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }

        if(scriptarry.size()!= sizeof(readscript)/sizeof(readscript[0])) {
            AddPrompt(GetLocalString(_T("IDS_ERROR_SIZE_VALUE")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }

        for(int st = 0; st < sizeof(readscript)/sizeof(readscript[0]); st++ ) {
            int ss     = 0;
            int nofcmd = m_ScriptArry.size();
            LDEGMSG((CLogger::DEBUG_INFO,"readscript 0x%08X  0x%08X\r\n",readscript[st].regid,scriptarry[st].bitmask));
            for(; ss < nofcmd ; ss ++ ) {
                LDEGMSG((CLogger::DEBUG_INFO,"    m_ScriptArry 0x%08X 0x%08X  0x%08X\r\n",m_ScriptArry[ss].function,m_ScriptArry[ss].regid,m_ScriptArry[ss].bitmask));
                /*the write 1 bits,the same register,and the same index**/
                if((FUS_FUNC_TEST4_1BITS      == m_ScriptArry[ss].function)&&
                    (readscript[st].regid     == m_ScriptArry[ss].regid)&&
                    (scriptarry[st].bitmask   == m_ScriptArry[ss].bitmask )) {
                    
                    break;
                } if((FUS_FUNC_TEST4_0BITS      == m_ScriptArry[ss].function)
                    &&(readscript[st].regid     == m_ScriptArry[ss].regid)
                    &&(scriptarry[st].bitmask   == ~m_ScriptArry[ss].bitmask )) {
                    break;
                }
            }
            if(ss == nofcmd) {
                AddPrompt(GetLocalString(_T("IDS_ERROR_MISMATCH_VALUE")).c_str(),LIST_ERR);
                goto exitworkthreadproc;
            }
        }
        if(bReadCheck) {
            AddPrompt(GetLocalString(_T("IDS_INFO_CHECK_PASS")).c_str(),LIST_INFO);
        } else {
            AddPrompt(GetLocalString(_T("IDS_INFO_READ_PASS")).c_str(),LIST_INFO);
        }
    }

    if(MODE_NONE != m_Configs.nMode ) {
        if(MODE_PTEST == m_Configs.nMode) {
            AddPrompt(GetLocalString(TEXT("IDS_INFO_SET_PTEST_MODE")).c_str(),LIST_INFO);
        } else {
            AddPrompt(GetLocalString(TEXT("IDS_INFO_SET_NORMAL_MODE")).c_str(),LIST_INFO);
        }
        if(!dut.SetMode(m_Configs.nMode)) {
            AddPrompt(GetLocalString(TEXT("IDS_ERROR_SET_MODE_FAIL")).c_str(),LIST_ERR);
            goto exitworkthreadproc;
        }
    }
    if(m_Configs.bReboot) {
        AddPrompt(GetLocalString(TEXT("IDS_INFO_REBOOT")).c_str(),LIST_INFO);
        if(MODE_NONE != m_Configs.nMode ) 
            Sleep(200);
        dut.reboot();
    }
    bSuccsee = TRUE;
exitworkthreadproc:
    if(bSuccsee) {
        AddPrompt(GetLocalString(_T("IDS_INFO_ALL_TEST_PASS")).c_str(),LIST_INFO);
    } else {
        AddPrompt(GetLocalString(_T("IDS_INFO_ALL_TEST_FAIL")).c_str(),LIST_ERR);
    }
    m_pWorkThread = NULL;
    /*exit will set button status,so we must set m_pWorkThread = NULL before **/
    if (!m_Configs.bAutoTest||m_bUserStop) {
        if (m_bUserStop) {
            m_bUserStop = FALSE;
        }
        PostMessage(WM_UPDATE_MSG,UPDATE_TEST_EXIT);
    }

    return TRUE;
}

CwvpctoolDlg::CwvpctoolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CwvpctoolDlg::IDD, pParent),m_pLog(NULL),m_pWorkThread(NULL),m_bStarWrite(FALSE),m_bUserStop(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CwvpctoolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_GTI, lbGti);
    DDX_Control(pDX, IDC_STATIC_AT, lbAt);
    DDX_Control(pDX, IDC_STATIC_ACTION, lbAction);
    DDX_Control(pDX, IDC_LIST_REPORT, m_listInfo);
    DDX_Control(pDX, IDC_STATIC_MSG, lbMsg);
}

BEGIN_MESSAGE_MAP(CwvpctoolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_UPDATE_COM,&CwvpctoolDlg::OnHandleComChange)
    ON_MESSAGE(WM_UPDATE_MSG,&CwvpctoolDlg::OnHandleUpdateMsg)
    ON_BN_CLICKED(IDC_BUTTON_SIGN, &CwvpctoolDlg::OnBnClickedButtonSign)
    ON_WM_DEVICECHANGE()
    ON_COMMAND(ID_HELP_ABOUT, &CwvpctoolDlg::OnHelpAbout)
    ON_COMMAND(ID_HELP_LOGFOLDER, &CwvpctoolDlg::OnHelpLogfolder)
	ON_COMMAND(ID_SETTING, &CwvpctoolDlg::OnSetting)
	ON_UPDATE_COMMAND_UI(ID_SETTING, &CwvpctoolDlg::OnUpdateSetting)
    ON_COMMAND(ID_SETTING_LOGIN, &CwvpctoolDlg::OnSettingLogin)
    ON_COMMAND(ID_USER_LOGOUT, &CwvpctoolDlg::OnUserLogout)
    ON_UPDATE_COMMAND_UI(ID_USER_CHANGEPASSWORD, &CwvpctoolDlg::OnUpdateUserChangepassword)
    ON_COMMAND(ID_USER_CHANGEPASSWORD, &CwvpctoolDlg::OnUserChangepassword)
END_MESSAGE_MAP()


// CwvpctoolDlg message handlers

BOOL CwvpctoolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
    
	// TODO: Add extra initialization here
    TCHAR MyexeFullPath[MAX_PATH] = TEXT("C:\\");
    if(GetModuleFileName(NULL,MyexeFullPath,MAX_PATH)) {
        TCHAR *pos = _tcsrchr(MyexeFullPath, TEXT('\\'));
        if(pos) {
            pos[1] = 0;
            m_strModulePath = MyexeFullPath;
        }
    }
    m_Configs.LoadToolSetting((LPCTSTR)(m_strModulePath+ TEXT("config.ini")));
    if(m_Configs.szLan.empty()) {
        m_Configs.szLan = TEXT("1.txt");
    }
#if 1
    /*load language resource **/
    m_LocalLang.LoadToolSetting((LPCTSTR)(m_strModulePath +TEXT("Lan\\") + m_Configs.szLan.c_str()));
    m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,this->IDD,true);
    if(!m_Configs.bDebug) {
        WalkMenu(GetMenu(),TEXT("MENU"));
    }
    GetWindowText(m_DlgTitle);
#endif

#ifdef USER_LOGIN
    bAdmin = m_Configs.bLogin;
    if(bAdmin) {
        SetWindowText(((LPCTSTR)m_DlgTitle + GetLocalString(TEXT("IDS_TEXT_ADMIN"))).c_str());
    } else {
        SetWindowText(((LPCTSTR)m_DlgTitle + GetLocalString(TEXT("IDS_TEXT_USER"))).c_str());
    }
#else 
    bAdmin = TRUE;
#endif

    /*
    m_LocalLang.LoadToolSetting((LPCTSTR)(m_strModulePath+ TEXT("Lang\\") + m_Configs.szLan.c_str() ));
    m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,IDD_WVPCTOOL_DIALOG,true);
    **/

    lbGti.SetTextColor(RGB(0,255,00));
    lbGti.SetFontSize(10);
    lbGti.SetFontBold(TRUE);
    
    lbAt.SetTextColor(RGB(0,255,00));
    lbAt.SetFontSize(10);
    lbAt.SetFontBold(TRUE);
    
    lbAction.SetTextColor(RGB(0,0,255));
    lbAction.SetFontSize(10);
    lbAction.SetFontBold(TRUE);

#if 0
    lbMsg.SetTextColor(RGB(0,0,255));
    lbMsg.SetFontSize(10);
    lbMsg.SetFontBold(TRUE);
#else 
    lbMsg.SetTextColor(RGB(0,255,00));
    lbMsg.SetFontSize(10);
    lbMsg.SetFontBold(TRUE);
#endif





    if(m_Configs.nLogLevel != DLEVEL_NONE ) {
        CLogger::DEBUG_LEVEL level = m_Configs.nLogLevel == DLEVEL_DEBUG?CLogger::DEBUG_ALL:
            (m_Configs.nLogLevel  == DLEVEL_INFO ?CLogger::DEBUG_INFO:CLogger::DEBUG_ERROR);
        if(m_Configs.strLogPath.empty()) {
            m_Configs.strLogPath = (LPCTSTR)(m_strModulePath + TEXT("log\\"));
        }
        m_pLog = CLogger::StartLog((m_Configs.strLogPath + CLogger::TimeStr(true, true)).c_str(), level);    
    }
    LDEGMSG((CLogger::DEBUG_INFO,"APP START ..."));
    m_WvToolDB.SetConfig(&m_Configs,&m_LocalLang,m_pLog);
    UpdateMenuItem();
    UpdateUIStatus(0);
	m_hDevScanEvent     = CreateEvent(NULL,FALSE,FALSE,TEXT("DEVICE_SCAN_EVENT"));
    m_hDevScanExitEvent = CreateEvent(NULL,FALSE,FALSE,TEXT("DEVICE_SCAN_EXIT_EVENT"));
    if(m_hDevScanEvent&&m_hDevScanExitEvent) {
        m_hScanThread   = AfxBeginThread(::ScanDeviceThread,(LPVOID)this);
        SetEvent(m_hDevScanEvent);
    }


    RegDevNotification(m_hWnd);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CwvpctoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout(m_Configs,m_LocalLang);
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CwvpctoolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CwvpctoolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CwvpctoolDlg::UpdateUIStatus(DWORD dwMask)
{
    wstring strMsg,strWidevine;
    if(m_Configs.bFuse) {
        strMsg = TEXT("Fuse");
    }
	if(m_Configs.bFuseRead) {
			strMsg = TEXT("FuseRead");
	}
    if(m_Configs.bWideVine) {
        if(m_Configs.bUseDB) {
            strWidevine = TEXT("WideVine(") + m_Configs.strDataBaseName + TEXT(":")  + m_Configs.strDataBaseTable + TEXT(")");
        } else {
            strWidevine = TEXT("WideVine(Local)");
        }
    }
    if(strMsg.empty()) {
        strMsg = strWidevine;
    } else {
        strMsg += (TEXT(";") + strWidevine);
    }
    lbMsg.SetText(strMsg.c_str());

    if(m_Configs.bAutoTest){
		SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_START_BUTTON")).c_str());
    } else {
		SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_TEST_BUTTON")).c_str());
    }
#if 0
    if(dwMask&&MASK_FUSE) {
        int show = SW_HIDE;
        if(m_Configs.bFuse) {
            show = SW_SHOW;
        }
        GetDlgItem(IDC_EDIT_SCRIPT      )->ShowWindow(show);
        GetDlgItem(IDC_STATIC_SCRIPT    )->ShowWindow(show);
        GetDlgItem(IDC_BUTTON_BROWSCRIPT)->ShowWindow(show);

    }
    if(dwMask&&MASK_WV) {
        int show = SW_HIDE;
        if(m_Configs.bWideVine) {
            GetDlgItem(IDC_STATIC_XML       )->ShowWindow(SW_SHOW);
            GetDlgItem(IDC_EDIT_KEYBOXBIN   )->ShowWindow(SW_SHOW);
            if(m_Configs.bUseDB) {
                lbKeyBoxBin.SetText(TEXT("Server:"));
				GetDlgItem(IDC_EDIT_KEYBOXBIN   )->EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_BROWBIN   )->ShowWindow(SW_HIDE);
                SetDlgItemText(IDC_EDIT_KEYBOXBIN, (m_Configs.strServer+TEXT(":") + m_Configs.strDataBaseName).c_str());
            } else {
                SetDlgItemText(IDC_EDIT_KEYBOXBIN, m_Configs.strXmlFileName.c_str());
                lbKeyBoxBin.SetText(TEXT("WvKeyBin:"));
				GetDlgItem(IDC_EDIT_KEYBOXBIN   )->EnableWindow(TRUE);
                GetDlgItem(IDC_BUTTON_BROWBIN   )->ShowWindow(SW_SHOW);
            }
		} else {
            GetDlgItem(IDC_STATIC_XML       )->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_EDIT_KEYBOXBIN   )->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BUTTON_BROWBIN   )->ShowWindow(SW_HIDE);
		}
    }
#endif
}

void CwvpctoolDlg::OnBnClickedButtonSign()
{
    // TODO: Add your control notification handler code here

     if( m_Configs.bAutoTest ) {
            if( m_bStarWrite ) {
                if( m_pWorkThread) { 
                    m_bUserStop     = TRUE; /*just set m_bUserStop,and wait for thread exit **/
				SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_STOPING_BUTTON")).c_str());
                } else {
                    m_bStarWrite    = FALSE;
				SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_START_BUTTON")).c_str());                
                }
            } else {
                /*m_bUserStop     = FALSE; **/
                m_bStarWrite    = TRUE;
			SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_STOP_BUTTON")).c_str());
                OnStartTest(TRUE);
            }
        } else {
            if(m_pWorkThread) {         /*test is running **/
                m_bUserStop     = TRUE; /*just set m_bUserStop,and wait for thread exit **/
			SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_STOPING_BUTTON")).c_str());
            } else {
                /*m_bUserStop = FALSE; **/
                if(OnStartTest(FALSE)) {
				SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_STOP_BUTTON")).c_str()); 
                }
            }
        }
}
BOOL CwvpctoolDlg::AppExit(BOOL bSaveConfig)
{
    if(bSaveConfig) m_Configs.SaveToolSetting(std::wstring(TEXT("")));
    if (NULL != m_hScanThread) {
        SetEvent(m_hDevScanExitEvent);
        if(WAIT_OBJECT_0 != WaitForSingleObject(m_hScanThread->m_hThread,1000)) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("deviceScanThread no exit\r\n")));
        } else {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("deviceScanThread exit\r\n")));
        }
    }
    if (NULL != m_hDevScanEvent) {
        CloseHandle(m_hDevScanEvent);
    }
    if (NULL != m_hDevScanExitEvent) {
        CloseHandle(m_hDevScanExitEvent);
    }
    if(m_pLog)      delete m_pLog;
    return TRUE;
}
void CwvpctoolDlg::OnClose()
{
    if(!IsEnableExit()) return ;
    if(AppExit(TRUE)) {
        CDialog::OnOK();
    }
}
void CwvpctoolDlg::OnCancel()
{
    if(!IsEnableExit()) return ;
    if(AppExit(TRUE)) {
        CDialog::OnOK();
    }
}

CString CwvpctoolDlg::BrowseAndGetSingleFile(CString strInitialPath,CString strFilter)
{
    CString         strFileName(TEXT(""));
    CFileDialog     filedlg(TRUE,NULL,NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,strFilter);
    TCHAR           pBuffer[MAX_PATH];
    memset(pBuffer,0,sizeof(TCHAR)*(MAX_PATH));
    swprintf(pBuffer,nof(pBuffer),(LPCTSTR)strInitialPath);
    filedlg.m_ofn.lpstrFile = pBuffer;
    filedlg.m_ofn.nMaxFile  = MAX_PATH;
    if( IDOK == filedlg.DoModal() ) {
        POSITION pos = filedlg.GetStartPosition();  
		while (pos != NULL)   {  
			strFileName =  filedlg.GetNextPathName(pos);
		}  
    } 
    return strFileName;
}


BOOL CwvpctoolDlg::OnDeviceChange(UINT nEventType,DWORD_PTR dwData)
{
    PDEV_BROADCAST_HDR  lpdb        = (PDEV_BROADCAST_HDR)dwData;
    UINT                uiEvent     = nEventType;
    if ((uiEvent==DBT_DEVICEARRIVAL)||(uiEvent==DBT_DEVICEREMOVECOMPLETE)||(uiEvent==0x7)) {
        SetEvent(m_hDevScanEvent);
    }
    return 0;
}
void CwvpctoolDlg::ScanThreadProc()
{
    DWORD       dwResult;
    BOOL        bChange;
#if ENUM_SER
    EnumSerial  *mpEnumSerial;
#else 
    INTELCOMPORT ports;
#endif
    HANDLE      hEventArray[2];
    hEventArray[0]   = m_hDevScanEvent;
    hEventArray[1]   = m_hDevScanExitEvent;
    while (TRUE) {
        dwResult = WaitForMultipleObjects(2,hEventArray,FALSE,INFINITE);
        if(WAIT_OBJECT_0 == dwResult ) {
            bChange = FALSE;
            if(CheckIntelUSBComPort(&ports,m_pLog)) {
                /*LDEGMSGW((CLogger::DEBUG_INFO,TEXT("GTI = %s"),ports.IntelPort[PORT_GTI].c_str())); **/
                /*LDEGMSGW((CLogger::DEBUG_INFO,TEXT("AT  = %s"),ports.IntelPort[PORT_AT].c_str())); **/
            }

            m_csPortChange.Lock();
            for(int i = 0 ; i < PORT_PORT_NUM;i++  ) {
                /*LDEGMSGW((CLogger::DEBUG_INFO,TEXT("m_ports %s;ports %s"),m_ports.IntelPort[i].c_str(),ports.IntelPort[i].c_str())); **/
                if(0 != m_ports.IntelPort[i].compare(ports.IntelPort[i])) {
                    m_ports.IntelPort[i]= ports.IntelPort[i];
                    bChange             = TRUE;
                    /*LDEGMSGW((CLogger::DEBUG_INFO,TEXT("Change"))); **/
                } else {
                    /*LDEGMSGW((CLogger::DEBUG_INFO,TEXT("No Change"))); **/
                }
            }
            m_csPortChange.Unlock();
            if(!(m_ports.IntelPort[PORT_GTI].empty()||m_ports.IntelPort[PORT_AT].empty())){
                if (bChange&&m_Configs.bAutoTest&&m_bStarWrite) {
                    OnStartTest(TRUE);
                }
            }
            if(bChange) PostMessage(WM_UPDATE_COM,WM_COM_ADD,0);

        } else  if ((WAIT_OBJECT_0 + 1 )== dwResult) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("ScanThreadProc exit")));
            break;
        } else {
        }
    }
#if ENUM_SER
    if (NULL != mpEnumSerial) {
        delete mpEnumSerial;
        mpEnumSerial = NULL;
    }
#endif
}
LRESULT CwvpctoolDlg::OnHandleUpdateMsg(WPARAM wParam,LPARAM lParam)
{
    switch(wParam) {
    case UPDATE_TEST_EXIT:
        /*if(m_Configs.bStressTest ||m_Configs.bAutoTest) **/
            m_bStarWrite = FALSE;
		SetDlgItemText(IDC_BUTTON_SIGN,GetLocalString(_T("IDS_TEXT_TEST_BUTTON")).c_str());
        break;
    case UPDATE_LIST:
        if (lParam == LIST_EMPTY) {
            m_listInfo.ResetContent();
        } else {
            PSTRUCT_LIST_LINE pLine = (PSTRUCT_LIST_LINE)lParam;
            if (pLine->flag == LIST_INFO) {
                m_listInfo.AddLine(CXListBox::White,CXListBox::Black,pLine->pszLineText); 
            } else if (pLine->flag == LIST_TIME) {
                m_listInfo.AddLine(CXListBox::White,CXListBox::Purple,pLine->pszLineText);
            } else if (pLine->flag == LIST_WARN){ 
                m_listInfo.AddLine(CXListBox::Black,CXListBox::Yellow,pLine->pszLineText);
            } else {
                m_listInfo.AddLine(CXListBox::White,CXListBox::Red,pLine->pszLineText);
            }
            m_listInfo.SetCurSel(m_listInfo.GetCount()-1);
            delete pLine;
        }
        break;

    }
    return 0;
}
LRESULT CwvpctoolDlg::OnHandleComChange(WPARAM wParam,LPARAM lParam)
{
    m_csPortChange.Lock();

    SetDlgItemText(IDC_EDIT_GTI,m_ports.IntelPort[PORT_GTI].c_str());
    SetDlgItemText(IDC_EDIT_AT,m_ports.IntelPort[PORT_AT].c_str());
    m_csPortChange.Unlock();

    return 0;

}
BOOL CwvpctoolDlg::IsEnableConfig()
{
    if(m_pWorkThread) return FALSE;
	if(m_Configs.bAutoTest&&m_bStarWrite) return FALSE;
    if(!bAdmin) return FALSE;
    return TRUE;
}
BOOL CwvpctoolDlg::IsEnableExit()
{
    if(m_pWorkThread) return FALSE;
	if(m_Configs.bAutoTest&&m_bStarWrite) return FALSE;
    return TRUE;
}
void CwvpctoolDlg::UpdateMenuItem()
{
#if 0
    CMenu	*pMenu	= GetMenu();
    if (!pMenu)	return;
    CMenu	*pAutoTest 	= pMenu->GetSubMenu(SUB_MENU_SETTING);
    if(m_Configs.bAutoTest ) {
        pAutoTest->CheckMenuItem( ID_SETTING_AUTO   , MF_CHECKED  |MF_BYCOMMAND);
    } else {
        pAutoTest->CheckMenuItem( ID_SETTING_AUTO   , MF_UNCHECKED|MF_BYCOMMAND);
    }  

    if(m_Configs.bWideVine ) {
        pAutoTest->CheckMenuItem( ID_SETTING_WV     , MF_CHECKED  |MF_BYCOMMAND);
    } else {
        pAutoTest->CheckMenuItem( ID_SETTING_WV     , MF_UNCHECKED|MF_BYCOMMAND);
    }  
    if(m_Configs.bFuse ) {
        pAutoTest->CheckMenuItem( ID_SETTING_FUSE     , MF_CHECKED  |MF_BYCOMMAND);
    } else {
        pAutoTest->CheckMenuItem( ID_SETTING_FUSE     , MF_UNCHECKED|MF_BYCOMMAND);
    } 

    if(m_Configs.bOnlyAt ) {
        pAutoTest->CheckMenuItem( ID_SETTING_ONLYAT   , MF_CHECKED  |MF_BYCOMMAND);
    } else {
        pAutoTest->CheckMenuItem( ID_SETTING_ONLYAT   , MF_UNCHECKED|MF_BYCOMMAND);
    } 
#endif
}
void CwvpctoolDlg::ClearAtCmd()
{

    m_vecAtCmd.clear();

}

void trim(std::string &str, const std::string& chars = " " )
{
    str.erase(0, str.find_first_not_of(chars));
    str.erase(str.find_last_not_of(chars)+1);
}


double hexstr2double(const char * str,int len)
{
    bool            bMeetSpaceExit  = false;
    double          usRet           = 0;
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
        if (str[i] == ' ') {
            if(bMeetSpaceExit) {
                break;
            } else {
                continue;
            }
        }
        bMeetSpaceExit = true;
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

BOOL CwvpctoolDlg::ParseAtCmd(std::string s,T_FUS_SCRIPT *pScriptArry)
{
    unsigned int            value[IDX_CNT];
    int                     j,iLen    = IDX_CNT;
    std::string            subs;
    std::string::size_type  posl,posr;
    if (s.empty()) return FALSE;
    posl    = 0;
    for( j = 0;j  < iLen ;) {
        posr = s.find_first_of(",",posl);
        if(std::string::npos != posr&&posr > posl) { /*  ,value, **/
            subs = s.substr(posl,posr - posl );
            posl = posr + 1;
            if((0 == subs.find("0x"))||(0 == subs.find("0X"))) { /*hex **/
                subs.erase(0,2);
                value[j++]  = hexstr2double(subs.c_str(),subs.length());
            } else {
                value[j++]  = (unsigned int)atof(subs.c_str());
            }
        } else if (std::string::npos == posr&&std::string::npos != posl){ /*  ,value **/
            subs = s.substr(posl);
            if((0 == subs.find("0x"))||(0 == subs.find("0X"))) { /*hex **/
                subs.erase(0,2);
                value[j++]  = hexstr2double(subs.c_str(),subs.length());
            } else {
                value[j++]  = (unsigned int)atof(subs.c_str());
            }
            break;
        } else if (std::string::npos != posr&&posr == posl) { /*** ,,***/
            value[j++] = 0;
        }
    }
    if(IDX_CNT == j) {
        pScriptArry->function   = value[IDX_FUN];
        pScriptArry->regid      = value[IDX_RID];
        pScriptArry->bitmask    = value[IDX_BMK];
        pScriptArry->value      = value[IDX_VLU];

        return TRUE;
    }
    return FALSE;
}
BOOL CwvpctoolDlg::ParseAtCmd(std::wstring s,T_FUS_SCRIPT *pScriptArry)
{
    unsigned int            value[IDX_CNT];
    int                     j,iLen    = IDX_CNT;
    std::wstring            subs;
    std::wstring::size_type  posl,posr;
    if (s.empty()) return FALSE;
    posl    = 0;
    for( j = 0;j  < iLen ;) {
        posr = s.find_first_of(TEXT(","),posl);
        if(std::string::npos != posr&&posr > posl) { /*  ,value, **/
            subs = s.substr(posl,posr - posl );
            posl = posr + 1;
            if((0 == subs.find(TEXT("0x")))||(subs.find(TEXT("0X")))) { /*hex **/
                subs.erase(0,2);
                value[j++]  = _tcstol(subs.c_str(),NULL,16);
            } else {
                value[j++] = (unsigned int)_wtof(subs.c_str());
            }
        } else if (std::string::npos == posr&&std::string::npos != posl){ /*  ,value **/
            subs = s.substr(posl);
            if((0 == subs.find(TEXT("0x")))||(subs.find(TEXT("0X")))) { /*hex **/
                subs.erase(0,2);
                value[j++]  = _tcstol(subs.c_str(),NULL,16);
            } else {
                value[j++] = (unsigned int)_wtof(subs.c_str());
            }
            break;
        } else if (std::string::npos != posr&&posr == posl) { /*** ,,***/
            value[j++] = 0;
        }
    }
    if(IDX_CNT == j) {
        pScriptArry->function   = value[IDX_FUN];
        pScriptArry->regid      = value[IDX_RID];
        pScriptArry->bitmask    = value[IDX_BMK];
        pScriptArry->value      = value[IDX_VLU];

        return TRUE;
    }
    return FALSE;

}

BOOL CwvpctoolDlg::LoadFusingScript(SCRIPTARRY &ScriptArry,LPCTSTR szFileName,BOOL bIsBin)
{
    BOOL    bResult      = FALSE;
    FIOH    load_file    = NULL;
    FILE    *hScriptFile = NULL;
    LDEGMSG((CLogger::DEBUG_INFO,"LoadFusingScript"));
    if(bIsBin) {

        uint8_t         *BinBuf;
        uint32_t        fLength;
        T_FUS_SCRIPT    *pScriptArry;
        int             NofCmd;
        int             i;
        load_file = FILEW_open((LPCTSTR)szFileName, FILE_open_mode_read);
        if(!load_file) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("FILEW_open(%s) fail"),(LPCTSTR)szFileName));
            return bResult;
        }
        fLength = FILEW_size(load_file);
        if(0 != fLength%SCRIPT_ITEM_SIZE) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("FILEW_size(%d) fail,error format"),fLength));
            goto lodexit;
        } else {
            BinBuf = new uint8_t[fLength];
        }
        if(NULL == BinBuf) goto lodexit;
        FILEW_seek(load_file,0);
        FILEW_read(load_file,BinBuf, fLength);
        ScriptArry.clear();
        pScriptArry =(T_FUS_SCRIPT*)BinBuf;
        NofCmd      = fLength/SCRIPT_ITEM_SIZE;
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("LoadFusingScript NofCmd=%d"),NofCmd));
        for(i = 0; i < NofCmd;i++) {
            LDEGMSG((CLogger::DEBUG_INFO,"cmd 0x%x,0x%x,0x%x,0x%x\r\n\r\n",
                        pScriptArry[i].function,pScriptArry[i].regid,pScriptArry[i].bitmask,pScriptArry[i].value));
            ScriptArry.push_back(pScriptArry[i]);
        } 
        if(BinBuf) delete[] BinBuf;
    } else {
        T_FUS_SCRIPT    tScriptArry;
        char            buffer[128];   
        hScriptFile = fopen ( CSettingBase::wstr2str(std::wstring(szFileName)).c_str(), "r");
        if(!hScriptFile) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("fopen(%s) fail"),(LPCTSTR)szFileName));
            return bResult;
        }
        if(fseek( hScriptFile, 0 , SEEK_SET)) goto lodexit; 
        while(NULL != fgets( buffer, sizeof(buffer)/sizeof(buffer[0]), hScriptFile )){
            std::string str = buffer;
            trim(str," \r\n");
            if( str.empty()) continue;
            if( '#' == str[0] || '!' == str[0]) continue;
            if(0 == str.find("at@sec:_io.fuse")) {
                std::string::size_type  posl,posr;
                posl = str.find("{");
                posr = str.find("}");
                if(std::string::npos != posl && std::string::npos != posr &&posr > posl) {
                    posl ++;
                    std::string strarry = str.substr(posl,posr - posl);
                    LDEGMSG((CLogger::DEBUG_INFO,"cmd %s",strarry.c_str()));
                    if(ParseAtCmd(strarry,&tScriptArry)) {
                        LDEGMSG((CLogger::DEBUG_INFO,"cmd 0x%x,0x%x,0x%x,0x%x\r\n\r\n",tScriptArry.function,tScriptArry.regid,tScriptArry.bitmask,tScriptArry.value));
                        ScriptArry.push_back(tScriptArry);
                    }
                }
            }
        }
    }
    if(0 == ScriptArry.size()) goto lodexit; 
    bResult = TRUE;
lodexit:
    if (load_file) {
        FILEW_close(load_file);
        load_file = 0;
    }
    if(hScriptFile) {
        fclose ( hScriptFile );
        hScriptFile = NULL;
    }
    return bResult;
}

BOOL CwvpctoolDlg::LoadAtCmds()
{
    char    buffer[256];   
    BOOL    bResult = FALSE;
    FILE    *hScriptFile;
    ClearAtCmd();
    hScriptFile = fopen ( CSettingBase::wstr2str(m_Configs.strFuseScriptFileName).c_str(), "r");
    if(NULL != hScriptFile) {
        if(fseek( hScriptFile, 0 , SEEK_SET)) goto lodexit; 
        while(NULL != fgets( buffer, sizeof(buffer)/sizeof(buffer[0]), hScriptFile )){
            std::string str = buffer;
            trim(str," \r\n");
            if( str.empty()) continue;
            if( '#' == str[0] || '!' == str[0]) continue;
            if(0 == str.find("at@")&&3<=str.length()) {
                m_vecAtCmd.push_back(str);
            }
            /*if vaild at command **/
        }
        if(hScriptFile) {
            fclose ( hScriptFile );
            hScriptFile = NULL;
        }
    }
    if(0 == m_vecAtCmd.size()) goto lodexit; 
    bResult = TRUE;
lodexit:
    return bResult;
}

BOOL CwvpctoolDlg::OnStartTest(BOOL bAuto)
{
    CString szPromt;
    BOOL    isDevExist = FALSE;
    if(m_pWorkThread) {
        return FALSE; /*app is running **/
    }
    if(!(m_Configs.bWideVine||m_Configs.bFuse)) {
        szPromt = TEXT("no select any operation !!!");
        goto onstarttestexit;
    }

    if(m_Configs.bFuse) {
#if 1
        if(0 == m_ScriptArry.size()) {
            if(!LoadFusingScript(m_ScriptArry,m_Configs.strFuseScriptFileName.c_str(),m_Configs.bIsBinScript)) {
#else 
            if(!LoadAtCmds()){

#endif
                szPromt = TEXT("Load fuse script fail!!!");
                goto onstarttestexit;
            }
        }
        

    }

    m_csPortChange.Lock();
    if(!(m_ports.IntelPort[PORT_GTI].empty()||m_ports.IntelPort[PORT_AT].empty())){
        isDevExist = TRUE;
    }
    m_csPortChange.Unlock();

    if(!isDevExist) {
        szPromt = TEXT("Device no exist!!!");
        goto onstarttestexit;
    }

    m_pWorkThread = AfxBeginThread(TestDeviceThread,this);
    if(NULL == m_pWorkThread) goto onstarttestexit;
    return TRUE;
onstarttestexit:
    if(!szPromt.IsEmpty()) {
        if(bAuto) {
            AddPrompt(szPromt,LIST_ERR);
        } else {
            AfxMessageBox(szPromt);
        }
    }
    return FALSE;
}

void CwvpctoolDlg::AddPrompt(CString strPrompt,int flag)
{
	PSTRUCT_LIST_LINE   pLine=NULL;
	SYSTEMTIME          curTime;

    LDEGMSGW((CLogger::DEBUG_INFO,(LPCTSTR)strPrompt));
	GetLocalTime( &curTime );
	pLine       = new STRUCT_LIST_LINE;
	if (!pLine) {
		return;
	}
    wsprintf(pLine->pszLineText,
        _T("%02d:%02d:%02d %03d\t%s "),
        curTime.wHour,curTime.wMinute,curTime.wSecond,curTime.wMilliseconds,
        (LPCTSTR)strPrompt);
    pLine->flag = flag;
    if(0 == PostMessage(WM_UPDATE_MSG,UPDATE_LIST,(LPARAM)pLine)) {
        delete pLine;
    }
    return;

}
BOOL CwvpctoolDlg::isVaildXmlFile(CString strXmlFileName)
{
    int extpos = strXmlFileName.Find(TEXT(".xml"));
    if(-1 == extpos ) {
        extpos = strXmlFileName.Find(TEXT(".XML"));
    }
    if( -1 == extpos ) {
        return FALSE;
    }
    return TRUE;
}
#define DEVICEID_BIN_LEN    32
#define ELEMENT_KEY_LEN     32
#define ELEMENT_ID_LEN      144
#define ELEMENT_MAGIC_LEN   8
#define ELEMENT_CRC_LEN     8
BOOL CwvpctoolDlg::ParseBin(const char *szBin,int nHexlen,char *bsae64,int *len)
{
    BOOL		bRet = FALSE;
    if(128 == nHexlen) {
        *len = base64_encode((unsigned char *)bsae64,NULL,(unsigned char *)szBin,nHexlen);
        LDEGMSG((CLogger::DEBUG_ERROR,"ParseBin##base64 len=%d str=%s",*len,bsae64));
        bRet = TRUE;
    } 
    return bRet;
}
BOOL CwvpctoolDlg::ParseXml(const char *szXml,int nXmlLen,char *bsae64,int *len)
{
	BOOL		bRet = FALSE;
    const char	*p   = NULL;
    int        	plen;
    BYTE		hex[128]; 
	/*open xml file **/
    TiXmlDocument * pDocXml = new TiXmlDocument("");
    if (!pDocXml) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("fail create TiXmlDocument")));
        goto bParseXmlErr;
    }
    if (!pDocXml->LoadBuf(szXml,nXmlLen)) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("TiXmlDocument load file fail")));
        goto bParseXmlErr;
    }

	TiXmlElement  *pElement = NULL;
    TiXmlNode     *pNode    = pDocXml->FirstChild("Keybox");
    int           hexlen    = 0;
    if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
        pElement  = (TiXmlElement *)pNode;
        bool bfindDeviceID = false;
        for(const TiXmlAttribute* attr = pElement->FirstAttribute();attr;attr = attr->Next()) {
            if(0 == strcmp(attr->Name(),"DeviceID")) {
                p = attr->Value();
                strcpy((char*)hex,p);
                hexlen = strlen(p);
                for(;hexlen < DEVICEID_BIN_LEN; hexlen ++) {
                    hex[hexlen] = 0;
                }
                bfindDeviceID = true;
                break;
            }

        }
        if(!bfindDeviceID) {
            LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("can not find DeviceID")));
            goto bParseXmlErr;
        }
/*
    <Key>123451234581dadd4fcfeb37c6dc204f</Key>
	<ID>0000000500000130e5f2c11d5783eb3bd02a60c80a9a222223333335ff35c1164bd8379ed4429c1234571be9d612345b2cd78071234569cc62926c11a931ecc0fa180da1809e8091</ID>
	<Magic>6b626f78</Magic>
	<CRC>f9e88225</CRC>
**/
        TiXmlElement *pKey      = NULL;
        TiXmlElement *pID       = NULL;
        TiXmlElement *pMagic    = NULL;
        TiXmlElement *pCRC      = NULL;
        pNode = pElement->FirstChild("Key");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pKey  = (TiXmlElement *)pNode;
            p       = pKey->FirstChild()->Value();
            plen    = strlen(p); 
            if(0 != plen%2){ /*ELEMENT_KEY_LEN = 32 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("Key format error!!!")));
                goto bParseXmlErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }

        }
        pNode = pElement->FirstChild("ID");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pID  = (TiXmlElement *)pNode;
            p       = pID->FirstChild()->Value();
            plen    = strlen(p); 
            if(0 != plen%2){    /*ELEMENT_ID_LEN = 144 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("ID format error!!!")));
                goto bParseXmlErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }
        }
        pNode = pElement->FirstChild("Magic");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pMagic  = (TiXmlElement *)pNode;
            p       = pMagic->FirstChild()->Value();
            plen    = strlen(p);  
            if(0 != plen%2){ /*ELEMENT_MAGIC_LEN = 8 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("Magic format error!!!")));
                goto bParseXmlErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }
        }
        pNode = pElement->FirstChild("CRC");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pCRC  = (TiXmlElement *)pNode;
            p       = pCRC->FirstChild()->Value();
            plen    = strlen(p); 
            if(0 != plen%2){ /*ELEMENT_CRC_LEN = 8 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("CRC format error!!!")));
                goto bParseXmlErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }
        }
        if(!pKey||!pID||!pMagic||!pCRC) {
            LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("XML format error!!!")));
            goto bParseXmlErr;
        }
        LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("hexlen %d !!!"),hexlen));
        if(128 == hexlen) {
            *len = base64_encode((unsigned char *)bsae64,NULL,hex,128);
            LDEGMSG((CLogger::DEBUG_ERROR,"ParseXml##base64 len=%d str=%s",*len,bsae64));
            bRet = TRUE;
        } 
    } else {
        LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("open Keybox error!!!")));
    }
bParseXmlErr:
    if (pDocXml) {
        delete pDocXml;
    }
    return bRet;
}
BOOL CwvpctoolDlg::LoadFromDB(char *bsae64,int *len,CWvToolDB * pWvToolDB,LPCTSTR szSn,BOOL isBin)
{
    std::wstring    strSn;
    CWvToolDB       *pDB;
    if(pWvToolDB) {
        pDB = pWvToolDB;
    } else {
        pDB = &m_WvToolDB;
    }
    if(szSn) {
        strSn = szSn;
    } else {
        strSn.clear();
    }
	wstring strWvKey;
	if(!pDB->Open(m_Configs.strServer.c_str(),m_Configs.strDataBaseName.c_str(),m_Configs.strUserName.c_str(),m_Configs.strPassword.c_str(),m_Configs.strDataBaseTable.c_str(),TRUE)) 
		return FALSE;
	if(!pDB->GetWVkeyAndUpdateFlag(strWvKey,strSn))
		return FALSE;
    if(isBin) {
        if(ParseBin(CSettingBase::wstr2str(strWvKey).c_str(),strWvKey.length(),bsae64,len))	{
            return TRUE;
        }else {
            /* pDB->CommitWVkey(FALSE,TEXT("")); **/
        } 
    } else {
        if(ParseXml(CSettingBase::wstr2str(strWvKey).c_str(),strWvKey.length(),bsae64,len))	{
            return TRUE;
        }else {
            /*pDB->CommitWVkey(FALSE,TEXT("")); **/
        }    
    }

    return FALSE;
}
BOOL CwvpctoolDlg::LoadFromFile(CString strFileName,char *bsae64,int *len,BOOL isBin)
{
    BOOL    bRet    = FALSE;
    FIOH load_file = FILEW_open((LPCTSTR)strFileName, FILE_open_mode_read);
    if(!load_file) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("FILEW_open(%s) fail"),(LPCTSTR)strFileName));
        return bRet;
    }
    if(isBin) {
        uint8_t  *BinBuf;
        uint32_t fLength = FILEW_size(load_file);
        if(128 != fLength) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("FILEW_size(%d) fail"),fLength));
            goto LoadFromFileErr;
        } else {
            BinBuf = new uint8_t[fLength];
        }
        if(NULL == BinBuf) goto LoadFromFileErr;
        FILEW_seek(load_file,0);
        FILEW_read(load_file,BinBuf, 128);
        if(!ParseBin((const char *)BinBuf,fLength,bsae64,len) ){
            delete[] BinBuf;
            BinBuf = NULL;
        }
    } else {
        uint8_t *BinBuf = NULL;
        uint32_t fLength = FILEW_size(load_file);
        if(0 < fLength) {
            BinBuf = new uint8_t[fLength];
        }
        if(NULL == BinBuf) goto LoadFromFileErr;
        FILEW_seek(load_file,0);
        FILEW_read(load_file,BinBuf, fLength);
        if(!ParseXml((const char *)BinBuf,fLength,bsae64,len) ){
            delete[] BinBuf;
            BinBuf = NULL;
        }
    }
    bRet = TRUE;
LoadFromFileErr:
    if (load_file) {
        FILEW_close(load_file);
        load_file = 0;
    }
    return bRet;
}
BOOL CwvpctoolDlg::LoadXmlFromFile(CString strXmlFileName,char *bsae64,int *len)
{
    BOOL                bRet                = FALSE;
    const char          *p                  = NULL;
    int                 plen;
    BYTE                hex[128]; 
    char                szXmlName[MAX_PATH];
    /*convert file name **/
    if(!WideCharToMultiByte( CP_ACP,0,(LPCTSTR)strXmlFileName,-1,szXmlName,nof(szXmlName),NULL,NULL )) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("WideCharToMultiByte(%s) fail"),(LPCTSTR)strXmlFileName));
        return FALSE;
    }
    /*open xml file **/
    TiXmlDocument * pDocXml = new TiXmlDocument(szXmlName);
    if (!pDocXml) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("open %s fail"),(LPCTSTR)strXmlFileName));
        goto bLoadXmlFileErr;
    }
    if (!pDocXml->LoadFile()) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("Load %s fail"),(LPCTSTR)strXmlFileName));
        goto bLoadXmlFileErr;
    }

    TiXmlElement  *pElement = NULL;
    TiXmlNode     *pNode    = pDocXml->FirstChild("Keybox");
    int           hexlen    = 0;
    if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
        pElement  = (TiXmlElement *)pNode;
        bool bfindDeviceID = false;
        for(const TiXmlAttribute* attr = pElement->FirstAttribute();attr;attr = attr->Next()) {
            if(0 == strcmp(attr->Name(),"DeviceID")) {
                p = attr->Value();
                strcpy((char*)hex,p);
                hexlen = strlen(p);
                for(;hexlen < DEVICEID_BIN_LEN; hexlen ++) {
                    hex[hexlen] = 0;
                }
                bfindDeviceID = true;
                break;
            }

        }
        if(!bfindDeviceID) {
            LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("can not find DeviceID")));
            goto bLoadXmlFileErr;
        }
/*
    <Key>123451234581dadd4fcfeb37c6dc204f</Key>
	<ID>0000000500000130e5f2c11d5783eb3bd02a60c80a9a222223333335ff35c1164bd8379ed4429c1234571be9d612345b2cd78071234569cc62926c11a931ecc0fa180da1809e8091</ID>
	<Magic>6b626f78</Magic>
	<CRC>f9e88225</CRC>
**/
        TiXmlElement *pKey      = NULL;
        TiXmlElement *pID       = NULL;
        TiXmlElement *pMagic    = NULL;
        TiXmlElement *pCRC      = NULL;
        pNode = pElement->FirstChild("Key");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pKey  = (TiXmlElement *)pNode;
            p       = pKey->FirstChild()->Value();
            plen    = strlen(p); 
            if(0 != plen%2){ /*ELEMENT_KEY_LEN = 32 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("Key format error!!!")));
                goto bLoadXmlFileErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }

        }
        pNode = pElement->FirstChild("ID");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pID  = (TiXmlElement *)pNode;
            p       = pID->FirstChild()->Value();
            plen    = strlen(p); 
            if(0 != plen%2){    /*ELEMENT_ID_LEN = 144 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("ID format error!!!")));
                goto bLoadXmlFileErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }
        }
        pNode = pElement->FirstChild("Magic");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pMagic  = (TiXmlElement *)pNode;
            p       = pMagic->FirstChild()->Value();
            plen    = strlen(p);  
            if(0 != plen%2){ /*ELEMENT_MAGIC_LEN = 8 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("Magic format error!!!")));
                goto bLoadXmlFileErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }
        }
        pNode = pElement->FirstChild("CRC");
        if(pNode&& TiXmlNode::TINYXML_ELEMENT == (TiXmlNode::NodeType)pNode->Type()) {
            pCRC  = (TiXmlElement *)pNode;
            p       = pCRC->FirstChild()->Value();
            plen    = strlen(p); 
            if(0 != plen%2){ /*ELEMENT_CRC_LEN = 8 **/
                LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("CRC format error!!!")));
                goto bLoadXmlFileErr;
            }
            for(int i = 0; i < plen;i+=2 ) {
                hex[hexlen] = hexstr2ushort(&p[i],2);
                hexlen ++;
            }
        }
        if(!pKey||!pID||!pMagic||!pCRC) {
            LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("XML format error!!!")));
            goto bLoadXmlFileErr;
        }
        LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("hexlen %d !!!"),hexlen));
        if(128 == hexlen) {
            *len = base64_encode((unsigned char *)bsae64,NULL,hex,128);
            LDEGMSG((CLogger::DEBUG_ERROR,"base64 len=%d str=%s",*len,bsae64));
            bRet = TRUE;
        } 
    } else {
        LDEGMSGW((CLogger::DEBUG_ERROR,TEXT("open Keybox error!!!")));
    }

bLoadXmlFileErr:
    if (pDocXml) {
        delete pDocXml;
    }
    return bRet;
}

std::wstring CwvpctoolDlg::GetLocalString(std::wstring strKey)
{
#if 0
    return strKey;
#else
    return m_LocalLang.GetLanStr(strKey);
#endif
}
void CwvpctoolDlg::OnHelpAbout()
{
    // TODO: Add your command handler code here
    CAboutDlg dlgAbout(m_Configs,m_LocalLang);
    dlgAbout.DoModal();
}

void CwvpctoolDlg::OnHelpLogfolder()
{
    // TODO: Add your command handler code here
    ShellExecute(NULL, _T("open"), m_Configs.strLogPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
void CwvpctoolDlg::OnSetting()
{
	// TODO: Add your command handler code here
	CWvConfigDlg wvConfigDlg(m_Configs,m_LocalLang);
	if(IDOK==wvConfigDlg.DoModal()){
        UpdateUIStatus(0);
        m_ScriptArry.clear();

    }
}

void CwvpctoolDlg::OnUpdateSetting(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(IsEnableConfig());
}

void CwvpctoolDlg::OnSettingLogin()
{
    // TODO: Add your command handler code here
#ifdef USER_LOGIN
    CLoginDlg login(m_Configs,m_LocalLang);
    if(login.DoModal() == IDOK) {
        bAdmin = TRUE;
        if(bAdmin) {
            SetWindowText(((LPCTSTR)m_DlgTitle + GetLocalString(TEXT("IDS_TEXT_ADMIN"))).c_str());
        } else {
            SetWindowText(((LPCTSTR)m_DlgTitle + GetLocalString(TEXT("IDS_TEXT_USER"))).c_str());
        }
    }
#else 
#endif
}
VOID CwvpctoolDlg::WalkMenu(CMenu *pMenu,CString strMainKeyPart)
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
        /*If the specified item is a pop-up menu (as opposed to an item within the pop-up menu), the return value is –1 **/
            pMenu->ModifyMenu(i, MF_BYPOSITION, i, m_LocalLang.GetStr((LPCTSTR)strKey).c_str());
        	WalkMenu(pMenu->GetSubMenu(i),strKey);
        } else {
            pMenu->ModifyMenu(id, MF_BYCOMMAND, id,m_LocalLang.GetStr((LPCTSTR)strKey).c_str());
        }
    }
}

void CwvpctoolDlg::OnUserLogout()
{
    // TODO: Add your command handler code here
    if(bAdmin) {
        bAdmin              = FALSE;
        m_Configs.bLogin    = false;
        if(bAdmin) {
            SetWindowText(((LPCTSTR)m_DlgTitle + GetLocalString(TEXT("IDS_TEXT_ADMIN"))).c_str());
        } else {
            SetWindowText(((LPCTSTR)m_DlgTitle + GetLocalString(TEXT("IDS_TEXT_USER"))).c_str());
        }
    }
}

void CwvpctoolDlg::OnUpdateUserChangepassword(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here
    pCmdUI->Enable(bAdmin == TRUE);

}

void CwvpctoolDlg::OnUserChangepassword()
{
    // TODO: Add your command handler code here
     
#ifdef USER_LOGIN
    CChangePasswordDlg login(m_Configs,m_LocalLang);
    if(login.DoModal() == IDOK) {
        AfxMessageBox(GetLocalString(TEXT("IDS_INFO_CHGPWD_OK")).c_str());
    }
#else 
#endif
}
