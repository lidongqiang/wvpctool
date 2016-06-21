
// wvpctoolDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "./label/label.h"
#include "./XListBox/XListBox.h"
#include "afxcmn.h"
#include <string>
#include <vector>
#include "DatabaseSettingDlg.h"
#include "WvConfigDlg.h"
#ifdef USER_LOGIN
#include "LoginDlg.h"
#include "ChangePasswordDlg.h"
#endif
#include "fusingdefs.h"

typedef std::vector<T_FUS_SCRIPT>   SCRIPTARRY;
typedef std::vector<std::string>    ATCMDSET;
typedef struct  
{
    int flag;
    TCHAR   pszLineText[MAX_PATH];
}STRUCT_LIST_LINE,*PSTRUCT_LIST_LINE;

const T_FUS_SCRIPT readscript[4] = {
    {0x30000200,0x10000101,0xffffffff,0xffffffff},
    {0x30000200,0x10000102,0xffffffff,0xffffffff},
    {0x30000200,0x10000103,0xffffffff,0xffffffff},
    {0x30000200,0x10000104,0xffffffff,0xffffffff}
};

// CwvpctoolDlg dialog
class CwvpctoolDlg : public CDialog
{
    friend UINT TestDeviceThread (LPVOID lpParam);
// Construction
public:
	CwvpctoolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WVPCTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
private:
    HANDLE              m_hDevScanEvent;
    HANDLE              m_hDevScanExitEvent;
    CWinThread          *m_hScanThread;
    CWinThread          *m_pWorkThread;
    BOOL                m_bStarWrite;
    BOOL                m_bUserStop;

    char                m_szBase64[256];
    int                 m_Base64len;

	CWvToolDB			m_WvToolDB;

    ATCMDSET            m_vecAtCmd;
    SCRIPTARRY          m_ScriptArry;

    CCriticalSection    m_csPortChange;
    INTELCOMPORT        m_ports;

    CIniSettingBase     m_Configs;
    CIniLocalLan        m_LocalLang;
    CString             m_strModulePath;
    CString             m_DlgTitle;
    BOOL                bAdmin;
    CLogger             *m_pLog;
    CString             BrowseAndGetSingleFile(CString strInitialPath,CString strFilter);
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    BOOL         AppExit(BOOL bSaveConfig);
    void         ScanThreadProc();
    BOOL         WorkThreadProc();
    LRESULT      OnHandleComChange(WPARAM wParam,LPARAM lParam);
    LRESULT      OnHandleUpdateMsg(WPARAM wParam,LPARAM lParam);
    void         UpdateUIStatus(DWORD);
    VOID         WalkMenu(CMenu *pMenu,CString strMainKeyPart);
    void         UpdateMenuItem();
    BOOL         OnStartTest(BOOL bAuto);
    BOOL         IsEnableConfig();
    BOOL         IsEnableExit();
    void         AddPrompt(CString strPrompt,int flag);
    BOOL         LoadFromFile(CString strFileName,char *bsae64,int *len,BOOL isBin);
    BOOL		 LoadXmlFromFile(CString strXmlFileName,char *bsae64,int *len);
    BOOL         LoadFromDB(char *bsae64,int *len,CWvToolDB * pWvToolDB = NULL,LPCTSTR szSn = NULL,BOOL isBin  = FALSE);
    BOOL         ParseXml(const char *szXml,int nXmlLen,char *bsae64,int *len);
    BOOL         ParseBin(const char *szBin,int nXmlLen,char *bsae64,int *len);

    BOOL         isVaildXmlFile(CString strXmlFileName);

    BOOL         RetryAndOpen(CTarget &dut,int nGtiCom,int nAtCom);
    BOOL         EnableIpcsd(int nAtPort,CLogger *logger);
    std::wstring GetLocalString(std::wstring strKey);

    void         ClearAtCmd();
    BOOL         LoadAtCmds();
    BOOL         LoadFusingScript(SCRIPTARRY &ScriptArry,LPCTSTR szFileName,BOOL bIsBin);
    BOOL         ParseAtCmd(std::wstring s,T_FUS_SCRIPT *pScriptArry);
    BOOL         ParseAtCmd(std::string s,T_FUS_SCRIPT *pScriptArry);
    BOOL         parseresult(std::string str,unsigned int *fus_action,unsigned int *fus_value,unsigned int *result_cause);

    BOOL         LoadScript         (CTarget &dut,SCRIPTARRY &scriptarry);
    BOOL         DoAttach           (CTarget &dut,const SCRIPTARRY &scriptarry);
    BOOL         DoAttachWithValue  (CTarget &dut,SCRIPTARRY &scriptarry);

    afx_msg void OnBnClickedButtonSign();
    afx_msg void OnClose();
    afx_msg void OnCancel();
    afx_msg BOOL OnDeviceChange(UINT nEventType,DWORD_PTR dwData);
    CLabel lbGti;
    CLabel lbAt;
    CLabel lbAction;
    CLabel lbMsg;
    CXListBox m_listInfo;
    afx_msg void OnHelpAbout();
    afx_msg void OnHelpLogfolder();
	afx_msg void OnSetting();
	afx_msg void OnUpdateSetting(CCmdUI *pCmdUI);
    afx_msg void OnSettingLogin();
    afx_msg void OnUserLogout();
    afx_msg void OnUpdateUserChangepassword(CCmdUI *pCmdUI);
    afx_msg void OnUserChangepassword();
};
