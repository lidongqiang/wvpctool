// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wvpctool.h"
#ifdef USER_LOGIN
#include "LoginDlg.h"


// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialog)

CLoginDlg::CLoginDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent):CDialog(CLoginDlg::IDD, pParent), 
			m_Configs(Configs),m_LocalLang(LocalLang)
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CLoginDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CLoginDlg message handlers

void CLoginDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    CString strPassword;
    BOOL    bChecked;
    GetDlgItemText(IDC_EDIT_PWD,strPassword);
    if(strPassword == m_Configs.strLoginPwd.c_str()) {
        bChecked = (((CButton*)GetDlgItem(IDC_CHECK_REM))->GetCheck() == BST_CHECKED);
        if(bChecked) {
            m_Configs.bLogin = true;
        }
        OnOK();
    } else {
        AfxMessageBox(GetLocalString(TEXT("IDS_ERROR_PASSWORD")).c_str());
    }

}

void CLoginDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    OnCancel();
}
BOOL CLoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,this->IDD,false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
















#endif
