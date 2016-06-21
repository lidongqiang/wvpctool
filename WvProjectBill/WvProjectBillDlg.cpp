
// WvProjectBillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WvProjectBill.h"
#include "WvProjectBillDlg.h"
#include "WvDBConfig.h"
#include "common.h"
#include "stdint.h"
#include ".\file\FileIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
BOOL WINAPI EnumerateFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles,void* pUserData)
{
    if(pUserData){
        ((CWvProjectBillDlg *)pUserData)->EnumFiles(lpFileOrPath,bEnumFiles);
    }
    return TRUE;
}
BOOL WINAPI PreEnumerateFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles,void* pUserData)
{
    if(pUserData){
        ((CWvProjectBillDlg *)pUserData)->PreEnumFiles(lpFileOrPath,bEnumFiles);
    }
    return TRUE;
}
UINT GenThread(LPVOID lpParam)
{
    CWvProjectBillDlg * ProjBill;
    if(lpParam){
        ProjBill = ( CWvProjectBillDlg *)lpParam;
        ProjBill->GenThread(lpParam);
        ProjBill->m_pGenThread = NULL;
    }
    return 0;
}
BYTE hexstr2ushort(const char * str,int len)
{
    BYTE            usRet   = 0;
    int             j;
    int             nlen;
    if(0 > len ) {
        nlen = strlen(str);
    } else {
        nlen = len;
    }
    if( 0 == nlen ) return usRet;
    if (2 < nlen) {
        j = 2;
    }
    for(int i = 0 ; i < nlen ;i ++ ) {
        BYTE val = 0;  
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

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CWvProjectBillDlg dialog




CWvProjectBillDlg::CWvProjectBillDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWvProjectBillDlg::IDD, pParent),m_pGenThread(NULL),m_pLog(NULL),m_pBuffer(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWvProjectBillDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_MSG, m_listInfo);
}

BEGIN_MESSAGE_MAP(CWvProjectBillDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_FOLDER, &CWvProjectBillDlg::OnBnClickedButtonFolder)
    ON_WM_CLOSE()
    ON_COMMAND(ID_SETTINGS, &CWvProjectBillDlg::OnSettings)
    ON_BN_CLICKED(IDC_BUTTON_GEN, &CWvProjectBillDlg::OnBnClickedButtonGen)
    ON_MESSAGE(WM_UPDATE_MSG,&CWvProjectBillDlg::OnHandleUpdateStatusMsg)
    ON_BN_CLICKED(IDC_CHECK_NEWTABLE, &CWvProjectBillDlg::OnBnClickedCheckNewtable)
    ON_COMMAND(ID_HELP_LOGFOLDER, &CWvProjectBillDlg::OnHelpLogfolder)
END_MESSAGE_MAP()


// CWvProjectBillDlg message handlers

BOOL CWvProjectBillDlg::OnInitDialog()
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
#if 0
    m_LocalLang.LoadToolSetting((LPCTSTR)(m_strModulePath +TEXT("Lan\\") + m_Configs.szLan.c_str()));
    m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,this->IDD,true);
    GetWindowText(m_DlgTitle);
#endif

    SetDlgItemText(IDC_EDIT_FOLDER,m_Configs.strXmlPath.c_str());
    ((CButton * )GetDlgItem(IDC_CHECK_NEWTABLE))->SetCheck(m_Configs.bCreateNewTable?BST_CHECKED:BST_UNCHECKED);
    SetDlgItemText(IDC_EDIT_NEWTABLE_NAME,m_Configs.strNewTableName.c_str());
    GetDlgItem(IDC_EDIT_NEWTABLE_NAME)->EnableWindow(m_Configs.bCreateNewTable);
    CFont font;
    font.CreateFont(-13,10,0,0,FW_NORMAL,FALSE,FALSE,0,  
        ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,  
        DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,_T("ËÎÌו"));
    m_listInfo.SetFont(&font);
    font.Detach();
    /*m_listInfo.SetWindowBKColor(RGB(0,0,0)); **/


    if(m_Configs.nLogLevel != DLEVEL_NONE ) {
        CLogger::DEBUG_LEVEL level = m_Configs.nLogLevel == DLEVEL_DEBUG?CLogger::DEBUG_ALL:
            (m_Configs.nLogLevel  == DLEVEL_INFO ?CLogger::DEBUG_INFO:CLogger::DEBUG_ERROR);
        if(m_Configs.strLogPath.empty()) {
            m_Configs.strLogPath = (LPCTSTR)(m_strModulePath + TEXT("log\\"));
        }
        m_pLog = CLogger::StartLog((m_Configs.strLogPath + CLogger::TimeStr(true, true)).c_str(), level);    
    }
    LDEGMSG((CLogger::DEBUG_INFO,"APP START ..."));



	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWvProjectBillDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
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

void CWvProjectBillDlg::OnPaint()
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
HCURSOR CWvProjectBillDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CWvProjectBillDlg::UpdateTestStatus(int Msg, int MsgId,VOID *strDesc)
{
    PostMessage(Msg,(WPARAM)MsgId,(LPARAM)strDesc);
}
VOID CWvProjectBillDlg::AddPrompt(CString strPrompt,int flag)
{
    PSTRUCT_LIST_LINE   pLine=NULL;
    SYSTEMTIME          curTime;
    LDEGMSGW((CLogger::DEBUG_INFO,(LPCTSTR)strPrompt));
    GetLocalTime( &curTime );
    pLine       = new STRUCT_LIST_LINE;
    if (!pLine) {
        return;
    }
    wsprintf(pLine->pszLineText,TEXT("%02d:%02d:%02d %03d\t%s "),
        curTime.wHour,curTime.wMinute,curTime.wSecond,curTime.wMilliseconds,
        (LPCTSTR)strPrompt);
    pLine->flag = flag;
    if(0 == PostMessage(WM_UPDATE_MSG,UPDATE_LIST,(LPARAM)pLine)) {
        delete pLine;
    }
    return;

}
LRESULT CWvProjectBillDlg::OnHandleUpdateMsg(WPARAM wParam,LPARAM lParam)
{
    /*TCHAR szPrompt[128]; **/
    switch(wParam) {
    case 0:
#if 0
    case PROCESS_PERCENT:
        m_AddProgress.SetPos((int)lParam);
        swprintf(szPrompt,128,TEXT("%d%%"),lParam);
        SetDlgItemText(IDC_STATIC_PERCENT,szPrompt);
#endif
        break;
    }

    return 0;
}
void CWvProjectBillDlg::OnBnClickedButtonFolder()
{
    // TODO: Add your control notification handler code here
    BOOL        bRet;
    CString     strDir;
    CString     strInitDir;
    GetDlgItemText(IDC_EDIT_FOLDER,strInitDir);
    bRet = BrowseDialog(strDir,_T(""),strInitDir);
	if (bRet) {
        strDir.Append(TEXT("\\"));
        SetDlgItemText(IDC_EDIT_FOLDER,strDir);
	}
}
BOOL CWvProjectBillDlg::Update2Config()
{
    CString strText;
    GetDlgItemText(IDC_EDIT_FOLDER,strText);
    m_Configs.strXmlPath = (LPCTSTR)strText;

    m_Configs.bCreateNewTable   = ((CButton * )GetDlgItem(IDC_CHECK_NEWTABLE))->GetCheck()==BST_CHECKED;
    GetDlgItemText(IDC_EDIT_NEWTABLE_NAME,strText);
    m_Configs.strNewTableName = (LPCTSTR)strText;
    return TRUE;
}
BOOL CWvProjectBillDlg::AppExit(BOOL bSaveConfig)
{
    if(bSaveConfig) {
        Update2Config();
        m_Configs.SaveToolSetting(TEXT(""));
    }
    if(m_pLog)      delete m_pLog;
    if(m_pBuffer)   delete[] m_pBuffer;
    return TRUE;
}
void CWvProjectBillDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default

    if(!IsConfigurable()) {
        AfxMessageBox(TEXT("Application is busy!!!"));
        return;
    }
    if(AppExit()){
        CDialog::OnClose();
    }
}
void CWvProjectBillDlg::OnSettings()
{
    // TODO: Add your command handler code here
    if(IsConfigurable()) {
        CWvDBConfig wvDBCongids(m_Configs,m_LocalLang);
        if(IDOK == wvDBCongids.DoModal()) {
            m_Configs.SaveToolSetting(TEXT(""));
        }
    }
}

void CWvProjectBillDlg::OnBnClickedButtonGen()
{
    // TODO: Add your control notification handler code here
    // TODO: Add your control notification handler code here
    TCHAR szPrompt[128];
    if(!IsConfigurable()) {
        AfxMessageBox(TEXT("Application is busy!!!"));
        return ;
    }

    Update2Config();
    if(m_Configs.bCreateNewTable&&m_Configs.strNewTableName.empty()) {
        AfxMessageBox(TEXT("New table name can not be empty!!!"));
        return ;
    }
#if 0
    for(int i = FLAG_SN ; i < FLAG_CNT ; i ++ ) {
        if(!CheckItemValid(m_Configs.strItemStart[i - FLAG_SN].c_str(),i )) {
            swprintf(szPrompt,128,TEXT("Item %s's start invalid!!!"),m_Configs.strItemName[i].c_str());
            AfxMessageBox(szPrompt);
            return ;
        }
    }
    for(int i = 0; i < FLAG_CNT; i++ ) {
        int span = m_Configs.nItemSpan[i];
        if( 0 > span ||10 < span) {
            swprintf(szPrompt,128,TEXT("Item %s's span invalid!!!"),m_Configs.strItemName[i].c_str());
            AfxMessageBox(szPrompt);
            return ;
        }
    }
    if(m_Configs.bGenWoid) {
        if(m_Configs.strWoid.empty()) {
            swprintf(szPrompt,128,TEXT("Item %s is null!!!\r\nClick yes continue"),
                m_Configs.strTableColumnName[COL_WOID].c_str());
            if(IDYES != AfxMessageBox(szPrompt,MB_YESNO)){
                return ;
            }
        }
    }
#endif
    m_pGenThread = AfxBeginThread(::GenThread,(LPVOID)this);
}
BOOL CWvProjectBillDlg::EnumFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles)
{

    LDEGMSGW((CLogger::DEBUG_INFO,lpFileOrPath));





    return TRUE;
}
BOOL CWvProjectBillDlg::PreEnumFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles)
{

    LDEGMSGW((CLogger::DEBUG_INFO,lpFileOrPath));





    return TRUE;
}
TCHAR *TableFormat = TEXT("CREATE TABLE [dbo].[%s]([dev_sn] [nvarchar](32) NULL,")
                TEXT("[wvkey] [nvarchar](320) NULL,")
                TEXT("[wvflag] [nvarchar](10) NULL,")
                TEXT("[woid] [nvarchar](20) NULL")
                TEXT(") ON [PRIMARY]");
#define MAX_BLOCK 1024*1024*10
UINT CWvProjectBillDlg::GenThread(LPVOID lpParam)
{
    /*FileEnum(m_Configs.strXmlPath.c_str(),TRUE,TRUE,PreEnumerateFiles,this,TEXT("*.XML")); **/
    BOOL            bSuccess    = FALSE;
    CString         strPrompt;
    DWORD           dwXmlFileCnt;
    


    /*1 find file count **/
    AddPrompt(TEXT("Finding file..."),LIST_INFO);
    LPCTSTR         partten;
    LPCTSTR         lpPath;
    partten         = TEXT("*.XML");
    lpPath          = m_Configs.strXmlPath.c_str();
    dwXmlFileCnt    = 0;
    FINDFILE_START
    /*
    FindFileName find file name
    bIsDirectory is directory o not 
    **/
    if(!bIsDirectory) {
        dwXmlFileCnt++;
    }
    FINDFILE_END
    strPrompt.Format(TEXT("XML file count %u"),dwXmlFileCnt);
    AddPrompt(strPrompt,LIST_INFO);
    if(0 == dwXmlFileCnt) {
        goto genthread_exit;
    }

    /*2 connect data base **/
    AddPrompt(TEXT("Connect to DataBase..."),LIST_INFO);
    m_WvToolDB.SetConfig(&m_Configs,&m_LocalLang,m_pLog);
    if(m_Configs.bCreateNewTable) {
        TCHAR szCmd[5000];
        TCHAR szCmdthis[256];
        swprintf(szCmd,sizeof(szCmd)/sizeof(szCmd[0]),TableFormat,m_Configs.strNewTableName.c_str());
        if(!m_WvToolDB.Open(TRUE)) {
            strPrompt.Format(TEXT("Connect database(%s)fail"),m_Configs.strDataBaseName.c_str());
            AddPrompt(strPrompt,LIST_ERR);
            goto genthread_exit;
        }
        if(m_WvToolDB.CreateTable(szCmd)){
        } else {
            strPrompt.Format(TEXT("Create Table(%s) on Database(%s) Fail"),m_Configs.strNewTableName.c_str(),m_Configs.strDataBaseName.c_str());
            AddPrompt(strPrompt,LIST_ERR);
            goto genthread_exit;
        }
        m_Configs.strDataBaseTable = m_Configs.strNewTableName;
    }
    /*add record **/

    if(!m_WvToolDB.OpenTableForAddNew(m_Configs.strDataBaseTable)){
        strPrompt.Format(TEXT("Open Table(%s) on Database(%s) Fail"),m_Configs.strNewTableName.c_str(),m_Configs.strDataBaseName.c_str());
        AddPrompt(strPrompt,LIST_ERR);
        goto genthread_exit;
    }

    strPrompt.Format(TEXT("Upload to Database..."));
    AddPrompt(strPrompt,LIST_INFO);
    BOOL bParseXml  = TRUE;
    partten         = TEXT("*.XML");
    lpPath          = m_Configs.strXmlPath.c_str();
    dwXmlFileCnt    =0;
    FINDFILE_START
    /*
    FindFileName find file name
    bIsDirectory is directory o not 
    **/
    if(!bIsDirectory) {
        BOOL bRet = ParseFile(FindFileName,&m_WvToolDB,dwXmlFileCnt);
        if(!bRet) {
            strPrompt.Format(TEXT("Open file(%s) Fail"),FindFileName);
            AddPrompt(strPrompt,LIST_ERR);
            FINDFILE_STOP
            bParseXml = FALSE;
        }
    }
    FINDFILE_END
    if(bParseXml) {
        strPrompt.Format(TEXT("Total Upload %u"),dwXmlFileCnt);
        AddPrompt(strPrompt,LIST_INFO);
        strPrompt.Format(TEXT("Upload to Database finish"));
        AddPrompt(strPrompt,LIST_INFO);
    } else {
        strPrompt.Format(TEXT("Total Upload %u"),dwXmlFileCnt);
        AddPrompt(strPrompt,LIST_INFO);
        strPrompt.Format(TEXT("Upload to Database abort"));
        AddPrompt(strPrompt,LIST_ERR);
        goto genthread_exit;
    }
    bSuccess = TRUE;
genthread_exit:
    return bSuccess;
}
BOOL CWvProjectBillDlg::ParseFile(LPCTSTR szFileName,CWvToolDB *pDB,DWORD &dwCnt)
{
    char        keybox[1024],*readpos;
    FIOH        hfile       = NULL;
    uint32_t    cfile       = 0;
    BOOL        bRet        = FALSE;
    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("ParseFile %s"),szFileName));
    if(!m_pBuffer) {
        m_pBuffer = new char[MAX_BLOCK + 1];
    }
    if(!m_pBuffer) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("ParseFile new char[MAX_BLOCK] FAIL")));
        return FALSE;
    }
    hfile = FILEW_open(szFileName, FILE_open_mode_read);
    if(!hfile) {
        LDEGMSGW((CLogger::DEBUG_INFO,TEXT("FILEW_open %s fail"),szFileName));
        return FALSE;
    }
    cfile = FILEW_size(hfile);
    if(0 == cfile) {
        goto parsefile_exit;
    }   

    ULONGLONG   TotalReadLength,HaveReadLength,NeedReadLength,ThisReadLength;
    TotalReadLength     = cfile;
    readpos             = m_pBuffer;
    uint32_t iBufSize   = TotalReadLength>MAX_BLOCK?MAX_BLOCK:TotalReadLength; /*max buffer size **/
    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("iBufSize %u"),iBufSize));

    HaveReadLength = 0;
    NeedReadLength = TotalReadLength; /*need read all contents in a file **/
    for(;NeedReadLength ;) {
        uint32_t iThisRead = (NeedReadLength > iBufSize + m_pBuffer - readpos )?(iBufSize + m_pBuffer - readpos):(NeedReadLength);
        if(FILE_status_ok != FILEW_read(hfile,(uint8_t *)readpos,iThisRead)) {
            LDEGMSGW((CLogger::DEBUG_INFO,TEXT("FILEW_read fail size %u"),iThisRead));
            goto parsefile_exit;
        }
        readpos         += iThisRead;
        readpos[0]      = '\0';
        NeedReadLength  -= iThisRead;
        HaveReadLength  += iThisRead;
        char     *pKeybox1;
        char     *pKeybox2;
        char     *pStartPos;
        int      len;
        pStartPos = m_pBuffer; /*start point **/
        do{
/*
<Keybox DeviceID="InFocus_MC2CN_1000100000">
	<Key>57004B2D8D5960924BCFDBAEAF6347F3</Key>
	<ID>000000020000170527B1FF295667FC648DCFABCC2AE0140EC56DA60F918069ED86CF7E72EA4213364B8ABDCF8D07C4701B75436657A0F3301E3A7A3B995B16B535F165566A1B3A00</ID>
	<Magic>6b626f78</Magic>
	<CRC>07EF3836</CRC>
</Keybox> **/
            pKeybox1 = strstr(pStartPos,"<Keybox");
            if(!pKeybox1) {
                /*actually , must pKeybox1 ==  pStartPos **/
                break;
            }
            pKeybox2 = strstr(pStartPos,"</Keybox>");
            if(!pKeybox2) {
                break;
            }
            pKeybox2 += 9;
            len = pKeybox2 - pKeybox1;

            if(0 > len||len > 1024) { /*</Keybox> is to large or misorder **/
                LDEGMSGW((CLogger::DEBUG_INFO,TEXT("error file exit")));
                goto parsefile_exit;
            }

            memcpy(keybox,pKeybox1,len);
            keybox[len] = 0;
            {
#if 1
                RECORDA  rcd;
                rcd.m_strItem[FLAG_DEVSN].clear();
                rcd.m_strItem[FLAG_WVKEY] = keybox;
                rcd.m_strItem[FLAG_WVFLG] = CSettingBase::wstr2str(m_Configs.strFlagUnsd);
                rcd.m_strItem[FLAG_WOID].clear();
                if(!pDB->AddSingleItem2Database(rcd) ) {
                    LDEGMSGW((CLogger::DEBUG_INFO,TEXT("AddSingleItem2Database fail")));
                    goto parsefile_exit;
                }
                dwCnt++;
#endif
            }
            LDEGMSG((CLogger::DEBUG_INFO,keybox));
            pStartPos = pKeybox2;
        } while(1);

        char *      p = pStartPos;
        int         i = 0;
        for(;p < readpos;p ++,i++) {  /*reset buffer move from pStartPos to m_pBuffer, then we get the readpos**/
            m_pBuffer[i] = p[0];
        }
        readpos = m_pBuffer + i;
    }
    bRet = TRUE;
parsefile_exit:
    if(hfile) {
        FILEW_close(hfile);
    }
    return bRet;

}
LRESULT CWvProjectBillDlg::OnHandleUpdateStatusMsg(WPARAM wParam,LPARAM lParam)
{

    switch(wParam) {
#if 0
        case UPDATE_TEST_EXIT:
            if(m_Start){
                /*SetRunStatus(GetLocalString(TEXT("IDS_DETECTING")).c_str(),RGB(0,255,0)); **/
            } else {
                /*SetRunStatus(GetLocalString(TEXT("IDS_IDLE")).c_str(),RGB(0,255,0)); **/
            }
            break;
        case UPDATE_PROMPT:
            if (lParam == PROMPT_TESTING) {
                SetRunStatus(GetLocalString(TEXT("TEST")).c_str(),RGB(0,255,0));
            } else if (lParam==PROMPT_PASS) {
                SetRunStatus(GetLocalString(TEXT("PASS")).c_str(),RGB(0,255,0));
            } else if(lParam==PROMPT_FAIL) {
                SetRunStatus(GetLocalString(TEXT("FAIL")).c_str(),RGB(255,0,0));
            } else if(lParam==PROMPT_ABORT) {
                SetRunStatus(GetLocalString(TEXT("ABORT")).c_str(),RGB(255,0,0));
            } else {
                SetRunStatus(GetLocalString(TEXT("")).c_str(),RGB(255,0,0));
            }
            break;
        case UPDATE_TEST_ENTER:
            /*SetRunStatus(GetLocalString(TEXT("IDS_RUNNING")).c_str(),RGB(0,255,0)); **/
            break;
#endif
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
void CWvProjectBillDlg::OnBnClickedCheckNewtable()
{
    // TODO: Add your control notification handler code here
    GetDlgItem(IDC_EDIT_NEWTABLE_NAME)->EnableWindow(
        ((CButton * )GetDlgItem(IDC_CHECK_NEWTABLE))->GetCheck()==BST_CHECKED);
}
BOOL CWvProjectBillDlg::LoadFromFile(CString strFileName,char *bsae64,int *len,BOOL isBin)
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
#define DEVICEID_BIN_LEN    32
#define ELEMENT_KEY_LEN     32
#define ELEMENT_ID_LEN      144
#define ELEMENT_MAGIC_LEN   8
#define ELEMENT_CRC_LEN     8

BOOL CWvProjectBillDlg::ParseBin(const char *szBin,int nHexlen,char *bsae64,int *len)
{
    BOOL		bRet = FALSE;
    if(128 == nHexlen) {
        *len = base64_encode((unsigned char *)bsae64,NULL,(unsigned char *)szBin,nHexlen);
        LDEGMSG((CLogger::DEBUG_ERROR,"ParseBin##base64 len=%d str=%s",*len,bsae64));
        bRet = TRUE;
    } 
    return bRet;
}
BOOL CWvProjectBillDlg::ParseXml(const char *szXml,int nXmlLen,char *bsae64,int *len)
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
void CWvProjectBillDlg::OnHelpLogfolder()
{
    // TODO: Add your command handler code here
    ShellExecute(NULL, _T("open"), m_Configs.strLogPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
