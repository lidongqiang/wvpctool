
// WvProjectBillDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include ".\CGridListCtrlEx\CGridListCtrlGroups.h"
#include ".\CGridListCtrlEx\CGridColumnTraitDateTime.h"
#include ".\CGridListCtrlEx\CGridColumnTraitEdit.h"
#include ".\CGridListCtrlEx\CGridColumnTraitCombo.h"
#include ".\CGridListCtrlEx\CGridColumnTraitHyperLink.h"
#include ".\CGridListCtrlEx\CGridRowTraitXP.h"
#include ".\CGridListCtrlEx\ViewConfigSection.h"
#include "./XListBox/XListBox.h"
#include "afxwin.h"
#include "./ado/SqlApi.h"
// CWvProjectBillDlg dialog
class CWvProjectBillDlg : public CDialog
{
    friend UINT GenThread(LPVOID lpParam);
    friend BOOL WINAPI EnumerateFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles,void* pUserData);
    friend BOOL WINAPI PreEnumerateFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles,void* pUserData);
// Construction
public:
	CWvProjectBillDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WVPROJECTBILL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
    CIniSettingBase     m_Configs;
    CIniLocalLan        m_LocalLang;
    CWvToolDB			m_WvToolDB;

    CString             m_strModulePath;

    CLogger             *m_pLog;
    char                *m_pBuffer;
    void    UpdateTestStatus(int Msg, int MsgId,VOID *strDesc);
    BOOL        m_bUserAbort;

    BOOL    Update2Config();
    BOOL    AppExit(BOOL bSaveConfig = TRUE);
    VOID    AddPrompt(CString strPrompt,int flag);
    UINT    GenThread(LPVOID lpParam);
    BOOL    EnumFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles);
    BOOL    PreEnumFiles(LPCTSTR lpFileOrPath,BOOL bEnumFiles);
    CWinThread  *m_pGenThread;
    BOOL IsConfigurable()
    {
        return NULL == m_pGenThread;
    }

    BOOL         LoadFromFile(CString strFileName,char *bsae64,int *len,BOOL isBin);
    BOOL		 LoadXmlFromFile(CString strXmlFileName,char *bsae64,int *len);
    BOOL         ParseXml(const char *szXml,int nXmlLen,char *bsae64,int *len);
    BOOL         ParseBin(const char *szBin,int nXmlLen,char *bsae64,int *len);
    BOOL         ParseFile(LPCTSTR szFileName,CWvToolDB *pDB,DWORD &dwCnt);
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
    afx_msg LRESULT OnHandleUpdateStatusMsg(WPARAM wParam,LPARAM lParam);
    afx_msg LRESULT OnHandleUpdateMsg(WPARAM wParam,LPARAM lParam);
    afx_msg void OnBnClickedButtonFolder();
    afx_msg void OnClose();
    afx_msg void OnSettings();
    afx_msg void OnBnClickedButtonGen();
    afx_msg void OnBnClickedCheckNewtable();
    CXListBox m_listInfo;
    afx_msg void OnHelpLogfolder();
};
