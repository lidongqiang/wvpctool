#pragma once


// CChangePasswordDlg dialog

class CChangePasswordDlg : public CDialog
{
	DECLARE_DYNAMIC(CChangePasswordDlg)

public:
	CChangePasswordDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent = NULL);   // standard constructor
	virtual ~CChangePasswordDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CHANGE_PWD };
private:
    CIniSettingBase     &m_Configs;
    CIniLocalLan        &m_LocalLang;
    std::wstring        GetLocalString(std::wstring strKey);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
};
