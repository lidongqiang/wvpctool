// WvConfig.cpp : implementation file
//

#include "stdafx.h"
#include "wvpctool.h"
#include "WvConfigDlg.h"


// CWvConfig dialog
IMPLEMENT_DYNAMIC(CWvConfigDlg, CDialog)
CWvConfigDlg::CWvConfigDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent):CDialog(CWvConfigDlg::IDD, pParent), 
			m_GneralSettings(Configs,LocalLang,this),m_Configs(Configs),m_LocalLang(LocalLang),
			m_DBSettings(Configs,LocalLang,this)
{

}

CWvConfigDlg::~CWvConfigDlg()
{
}

void CWvConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_SETTINGS, m_TabSettings);
}


BEGIN_MESSAGE_MAP(CWvConfigDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SETTINGS, &CWvConfigDlg::OnTcnSelchangeTabSettings)
	ON_BN_CLICKED(IDOK, &CWvConfigDlg::OnBnClickedOk)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CWvConfig message handlers
BOOL CWvConfigDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_TabSettings.InsertItem(0,_T("Basic"));
    m_TabSettings.InsertItem(1,_T("DataBase"));

    m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,this->IDD,false);

	CRect rect;
	m_TabSettings.GetClientRect(&rect);
    rect.InflateRect(-4,-22,-4,-4);
    m_GneralSettings.Create(IDD_DIALOG_GENERAL, GetDlgItem(IDC_TAB_SETTINGS));
    m_DBSettings.Create(IDD_DIALOG_DATABASE, GetDlgItem(IDC_TAB_SETTINGS));
    /*m_DBSettings.EnableWindow(FALSE); **/
    m_GneralSettings.MoveWindow(&rect);
	m_DBSettings.MoveWindow(&rect);

    m_pChildWnd[0] = &m_GneralSettings;
    m_pChildWnd[1] = &m_DBSettings;

    m_iCurWnd = -1;
    m_TabSettings.SetCurSel(0);
    ShowWnd(0);


	return TRUE;
}
void CWvConfigDlg::ShowWnd(int iCurWnd) 
{
    if((0 > iCurWnd)||(CHILDWNDCNT <= iCurWnd)) return ;
    if(0 <= m_iCurWnd&&CHILDWNDCNT > m_iCurWnd) {
        if(m_pChildWnd[m_iCurWnd]) {
            m_pChildWnd[m_iCurWnd]->ShowWindow(SW_HIDE);
        }
        m_iCurWnd = iCurWnd;
        if(m_pChildWnd[m_iCurWnd]) {
            m_pChildWnd[m_iCurWnd]->ShowWindow(SW_SHOW);
        }
    } else {
        m_iCurWnd = iCurWnd;
        if(m_pChildWnd[m_iCurWnd]) {
            m_pChildWnd[m_iCurWnd]->ShowWindow(SW_SHOW);
        }
    }
}
void CWvConfigDlg::OnTcnSelchangeTabSettings(NMHDR *pNMHDR, LRESULT *pResult)
{

    int nCurSel = m_TabSettings.GetCurSel();
    ShowWnd(nCurSel);
	if(pResult) *pResult = 0;
}

void CWvConfigDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
    if(!SaveConfig()) {
        /*AfxMessageBox(TEXT("Save config fail")); **/
        return ;
    }

    for(int i = 0; i < CHILDWNDCNT; i ++ ) {
        if(m_pChildWnd[i]) {
            m_pChildWnd[i]->DestroyWindow();
            m_pChildWnd[i] = NULL;
        }
    }
    OnOK();
}

BOOL CWvConfigDlg::SaveConfig()
{
    if(!m_GneralSettings.SendMessage(WM_SAVE_CONFIG,0,0)) 
        return FALSE;
    if(m_Configs.bUseDB&&m_Configs.bWideVine) {
        if(!m_DBSettings.SendMessage(WM_SAVE_CONFIG,0,0)) 
            return FALSE;
    }
    return TRUE;
}
void CWvConfigDlg::OnCancel()
{
    // TODO: Add your message handler code here and/or call default
    CDialog::OnCancel();
}

void CWvConfigDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default
    CDialog::OnClose();
}
std::wstring CWvConfigDlg::GetLocalString(std::wstring strKey)
{
    return m_LocalLang.GetLanStr(strKey);
}


