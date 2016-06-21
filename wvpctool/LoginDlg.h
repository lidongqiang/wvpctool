#pragma once


// CLoginDlg dialog

class CLoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_LOGIN };
private:
    CIniSettingBase     &m_Configs;
    CIniLocalLan        &m_LocalLang;
    std::wstring GetLocalString(std::wstring strKey)
    {
        return m_LocalLang.GetLanStr(strKey);
    }
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
};
