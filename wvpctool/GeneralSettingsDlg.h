#pragma once

#include "afxcmn.h"
#include "afxwin.h"
#include "./label/label.h"
#include "./XListBox/XListBox.h"
#include "./settings/SettingBase.h"
#include ".\CGridListCtrlEx\CGridListCtrlGroups.h"
#include ".\CGridListCtrlEx\CGridColumnTraitDateTime.h"
#include ".\CGridListCtrlEx\CGridColumnTraitEdit.h"
#include ".\CGridListCtrlEx\CGridColumnTraitCombo.h"
#include ".\CGridListCtrlEx\CGridColumnTraitHyperLink.h"
#include ".\CGridListCtrlEx\CGridRowTraitXP.h"
#include ".\CGridListCtrlEx\ViewConfigSection.h"
#include "./ado/SqlApi.h"

// CGeneralSettings dialog

class CGeneralSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CGeneralSettingsDlg)

public:
	CGeneralSettingsDlg(CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent = NULL);
	virtual ~CGeneralSettingsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_GENERAL };
private:
	CIniSettingBase &m_Configs;
    CIniLocalLan    &m_LocalLang;
    BOOL                isVaildFile(CString strXmlFileName);
    CString             BrowseAndGetSingleFile(CString strInitialPath,CString strFilter);
    void                UpdateUIStatus(DWORD dwMask);
    BOOL                SaveConfig();
    void                InitCtrl();
    std::wstring        GetLocalString(std::wstring strKey);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonKeyboxbin();
    afx_msg void OnBnClickedButtonScript();
    afx_msg void OnBnClickedCheckUsedb();
    /*CLabel lbKeyBoxBin; **/
    /*CLabel lbFuseScript;**/
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnBnClickedCheckWv();
    afx_msg void OnBnClickedCheckFuse();
    afx_msg LRESULT OnSaveConfig(WPARAM wParam,LPARAM lParam);
    afx_msg void OnBnClickedCheckXml();
    afx_msg void OnBnClickedCheckBin();
    afx_msg void OnBnClickedCheckReboot();
};
