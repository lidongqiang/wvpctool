// ChangePasswordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wvpctool.h"
#include "ChangePasswordDlg.h"


// CChangePasswordDlg dialog

IMPLEMENT_DYNAMIC(CChangePasswordDlg, CDialog)
CChangePasswordDlg::CChangePasswordDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent):CDialog(CChangePasswordDlg::IDD, pParent), 
			m_Configs(Configs),m_LocalLang(LocalLang)
{

}

CChangePasswordDlg::~CChangePasswordDlg()
{
}

void CChangePasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChangePasswordDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CChangePasswordDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChangePasswordDlg message handlers
BOOL CChangePasswordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,this->IDD,false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CChangePasswordDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    CString pwd,pwdcfm;
    GetDlgItemText(IDC_EDIT_NEW_PWD,pwd);
    GetDlgItemText(IDC_EDIT_CONFIRM_PWD,pwdcfm);
    if(pwd == pwdcfm) {
        m_Configs.strLoginPwd = (LPCTSTR)pwd;
        OnOK();
    } else {
        AfxMessageBox(GetLocalString(TEXT("IDS_ERROR_CHGPWD_INCORRECT")).c_str());
    }
}
std::wstring CChangePasswordDlg::GetLocalString(std::wstring strKey)
{
    return m_LocalLang.GetLanStr(strKey);
}


