#pragma once
#include ".\CGridListCtrlEx\CGridListCtrlGroups.h"
#include ".\CGridListCtrlEx\CGridColumnTraitDateTime.h"
#include ".\CGridListCtrlEx\CGridColumnTraitEdit.h"
#include ".\CGridListCtrlEx\CGridColumnTraitCombo.h"
#include ".\CGridListCtrlEx\CGridColumnTraitHyperLink.h"
#include ".\CGridListCtrlEx\CGridRowTraitXP.h"
#include ".\CGridListCtrlEx\ViewConfigSection.h"

// CWvDBConfig dialog

class CWvDBConfig : public CDialog
{
	DECLARE_DYNAMIC(CWvDBConfig)

public:
	CWvDBConfig(CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent = NULL);   // standard constructor
	virtual ~CWvDBConfig();

// Dialog Data
	enum { IDD = IDD_CONFIG_DIALOG };
private:
    CIniSettingBase &m_Configs;
    CIniLocalLan    &m_LocalLang;
    CString         m_strModulePath;

    BOOL    OnUpdateTable(BOOL Update2Table = TRUE,BOOL bInitial = TRUE);
    BOOL    SaveConfig();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    
    // Generated message map functions
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
    CGridListCtrlGroups m_ListTable;
    afx_msg void OnNMRClickListInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnPopmenuDelete();
    afx_msg void OnPopmenuAdd();
    afx_msg void OnPopmenuInsert();
    afx_msg void OnBnClickedButtonOk();
    afx_msg void OnBnClickedButtonLoad();
};
