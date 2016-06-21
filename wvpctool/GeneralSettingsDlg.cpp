// GeneralSettings.cpp : implementation file
//

#include "stdafx.h"
#include "wvpctool.h"
#include "GeneralSettingsDlg.h"
struct 
{
    TCHAR * strMode;
    int     type;
} RebootMode[] = 
{
    {TEXT("PTEST")      ,MODE_PTEST},
    {TEXT("NORMAL")     ,MODE_NORMAL,},
    {TEXT("AIRPLANE")   ,MODE_AIRPLANE},
    {TEXT("NONE")       ,MODE_NONE  },
};

// CGeneralSettings dialog

IMPLEMENT_DYNAMIC(CGeneralSettingsDlg, CDialog)

CGeneralSettingsDlg::CGeneralSettingsDlg( CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent /*=NULL*/)
	: CDialog(CGeneralSettingsDlg::IDD, pParent), m_Configs(Configs),m_LocalLang(LocalLang)
{

}

CGeneralSettingsDlg::~CGeneralSettingsDlg()
{
}

void CGeneralSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGeneralSettingsDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_KEYBOXBIN, &CGeneralSettingsDlg::OnBnClickedButtonKeyboxbin)
    ON_BN_CLICKED(IDC_BUTTON_SCRIPT, &CGeneralSettingsDlg::OnBnClickedButtonScript)
    ON_BN_CLICKED(IDC_CHECK_USEDB, &CGeneralSettingsDlg::OnBnClickedCheckUsedb)
    ON_BN_CLICKED(IDC_CHECK_WV, &CGeneralSettingsDlg::OnBnClickedCheckWv)
    ON_BN_CLICKED(IDC_CHECK_FUSE, &CGeneralSettingsDlg::OnBnClickedCheckFuse)
    ON_MESSAGE(WM_SAVE_CONFIG,&CGeneralSettingsDlg::OnSaveConfig)
    ON_BN_CLICKED(IDC_CHECK_XML, &CGeneralSettingsDlg::OnBnClickedCheckXml)
    ON_BN_CLICKED(IDC_CHECK_BIN, &CGeneralSettingsDlg::OnBnClickedCheckBin)
    ON_BN_CLICKED(IDC_CHECK_REBOOT, &CGeneralSettingsDlg::OnBnClickedCheckReboot)
END_MESSAGE_MAP()


// CGeneralSettings message handlers
BOOL CGeneralSettingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,this->IDD,false);

    ((CButton*)GetDlgItem(IDC_CHECK_AUTO        ))->SetCheck(m_Configs.bAutoTest    ?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_ONLYAT      ))->SetCheck(m_Configs.bOnlyAt      ?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_FUSE        ))->SetCheck(m_Configs.bFuse        ?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_WV          ))->SetCheck(m_Configs.bWideVine    ?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_WVREAD      ))->SetCheck(m_Configs.bWideVineRead?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_USEDB       ))->SetCheck(m_Configs.bUseDB       ?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_FORCEWRITE  ))->SetCheck(m_Configs.bForceWrite  ?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_REBOOT      ))->SetCheck(m_Configs.bReboot      ?BST_CHECKED:BST_UNCHECKED);

    ((CButton*)GetDlgItem(IDC_CHECK_READ_FUSE   ))->SetCheck(m_Configs.bFuseRead    ?BST_CHECKED:BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHECK_SCRIPT_BIN  ))->SetCheck(m_Configs.bIsBinScript ?BST_CHECKED:BST_UNCHECKED);

    /*set mode **/
    CComboBox* hCtl =  (CComboBox*)GetDlgItem(IDC_COMBO_MODE);
    INT CurSel          = -1;
    for(int i = 0 ; i < dim(RebootMode); i ++ ) {
        hCtl->AddString(RebootMode[i].strMode);
        if(m_Configs.nMode == RebootMode[i].type) {
            CurSel = i;
        }
    }
    if(-1 != CurSel) {
        hCtl->SetCurSel(CurSel); 
    }
    /*set mode end ***/



    if( m_Configs.bIsBin) {
        ((CButton*)GetDlgItem(IDC_CHECK_BIN))->SetCheck(BST_CHECKED);
        ((CButton*)GetDlgItem(IDC_CHECK_XML))->SetCheck(BST_UNCHECKED);
    } else {
        ((CButton*)GetDlgItem(IDC_CHECK_BIN))->SetCheck(BST_UNCHECKED);
        ((CButton*)GetDlgItem(IDC_CHECK_XML))->SetCheck(BST_CHECKED);
    }
    InitCtrl();
    return TRUE;
}

LRESULT CGeneralSettingsDlg::OnSaveConfig(WPARAM wParam,LPARAM lParam)
{
    return SaveConfig();
}

void CGeneralSettingsDlg::InitCtrl()
{
#if 0
    lbKeyBoxBin.SetTextColor(RGB(0,0,255));
    lbKeyBoxBin.SetFontSize(10);
    lbKeyBoxBin.SetFontBold(TRUE);

    lbFuseScript.SetTextColor(RGB(0,0,255));
    lbFuseScript.SetFontSize(10);
    lbFuseScript.SetFontBold(TRUE);
#endif
    SetDlgItemText(IDC_EDIT_SCRIPT    , m_Configs.strFuseScriptFileName.c_str());
    SetDlgItemText(IDC_EDIT_KEYBOXBIN , m_Configs.strXmlFileName.c_str());
    UpdateUIStatus(MASK_FUSE|MASK_WV|MASK_WV);

    
}

void CGeneralSettingsDlg::UpdateUIStatus(DWORD dwMask)
{
    /*basic **/
    if(dwMask&MASK_BASIC) {
    

    }

    /*widevine **/
    if(dwMask&MASK_WV){
        if(m_Configs.bWideVine) {
            GetDlgItem(IDC_CHECK_XML   )->EnableWindow(TRUE);
            GetDlgItem(IDC_CHECK_BIN   )->EnableWindow(TRUE);
            GetDlgItem(IDC_CHECK_USEDB )->EnableWindow(TRUE);
            GetDlgItem(IDC_CHECK_WVREAD)->EnableWindow(TRUE);
            if(m_Configs.bUseDB) {
                GetDlgItem(IDC_EDIT_KEYBOXBIN  )->EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_KEYBOXBIN)->EnableWindow(FALSE);
                GetDlgItem(IDC_STATIC_XML)->EnableWindow(FALSE);
                /*lbKeyBoxBin.EnableWindow(FALSE); **/
            } else {
                GetDlgItem(IDC_EDIT_KEYBOXBIN  )->EnableWindow(TRUE);
                GetDlgItem(IDC_BUTTON_KEYBOXBIN)->EnableWindow(TRUE);
                GetDlgItem(IDC_STATIC_XML)->EnableWindow(TRUE);
                /*lbKeyBoxBin.EnableWindow(TRUE); **/
            }
        } else {
            GetDlgItem(IDC_CHECK_XML        )->EnableWindow(FALSE);
            GetDlgItem(IDC_CHECK_BIN        )->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_KEYBOXBIN   )->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_KEYBOXBIN )->EnableWindow(FALSE);
            GetDlgItem(IDC_STATIC_XML       )->EnableWindow(FALSE);
            /*lbKeyBoxBin.EnableWindow(FALSE); **/
            GetDlgItem(IDC_CHECK_USEDB      )->EnableWindow(FALSE);
            GetDlgItem(IDC_CHECK_WVREAD     )->EnableWindow(FALSE);

        }
    }
    /*fuse **/
    if(dwMask&MASK_FUSE){
        BOOL enable;
        if(m_Configs.bFuse) {
            enable = TRUE;
        } else {
            enable = FALSE;
        }

        GetDlgItem(IDC_CHECK_READ_FUSE  )->EnableWindow(enable);
        GetDlgItem(IDC_CHECK_SCRIPT_BIN )->EnableWindow(enable);
        GetDlgItem(IDC_EDIT_SCRIPT      )->EnableWindow(enable);
        GetDlgItem(IDC_STATIC_SCRIPT    )->EnableWindow(enable);
        GetDlgItem(IDC_BUTTON_SCRIPT    )->EnableWindow(enable);
    }
}

BOOL CGeneralSettingsDlg::isVaildFile(CString strXmlFileName)
{
    TCHAR *szType[] = {
        TEXT(".xml"),
        TEXT(".XML"),
        TEXT(".dat"),
        TEXT(".DAT")
    };
    int nDex = m_Configs.bIsBin?1:0;
    int extpos = strXmlFileName.Find(szType[nDex*2 + 0]);
    if(-1 == extpos ) {
        extpos = strXmlFileName.Find(szType[nDex*2 + 1]);
    }
    if( -1 == extpos ) {
        return FALSE;
    }
    return TRUE;
}
void CGeneralSettingsDlg::OnBnClickedButtonKeyboxbin()
{
    // TODO: Add your control notification handler code here
    CString strFileName;
    CString strText;
    strText.Empty();
    GetDlgItemText(IDC_EDIT_KEYBOXBIN,strText);
    if(m_Configs.bIsBin ) {
        strFileName = BrowseAndGetSingleFile(strText,TEXT("PrgFile(*.dat)|*.dat|All File(*.*)|*.*||"));
    } else {
        strFileName = BrowseAndGetSingleFile(strText,TEXT("PrgFile(*.xml)|*.xml|All File(*.*)|*.*||"));
    }
    if(isVaildFile(strFileName)) {
#if 0
        m_Configs.strXmlFileName = (LPCTSTR)strFileName;
        SetDlgItemText(IDC_EDIT_KEYBOXBIN, m_Configs.strXmlFileName.c_str());
#else 
        SetDlgItemText(IDC_EDIT_KEYBOXBIN, strFileName);
#endif
    }
}

void CGeneralSettingsDlg::OnBnClickedButtonScript()
{
   // TODO: Add your control notification handler code here
    CString strText;
    strText.Empty();
    GetDlgItemText(IDC_EDIT_SCRIPT,strText);
    CString strFileName = BrowseAndGetSingleFile(strText,TEXT(/*"PrgFile(*.prg)|*.prg|*/"All File(*.*)|*.*||"));
    if(!strFileName.IsEmpty()) {
#if 0
        m_Configs.strFuseScriptFileName = (LPCTSTR)strFileName;
        SetDlgItemText(IDC_EDIT_SCRIPT, m_Configs.strFuseScriptFileName.c_str());
#else 
        SetDlgItemText(IDC_EDIT_SCRIPT, strFileName);
#endif
    }
}

CString CGeneralSettingsDlg::BrowseAndGetSingleFile(CString strInitialPath,CString strFilter)
{
    CString         strFileName(TEXT(""));
    CFileDialog     filedlg(TRUE,NULL,NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,strFilter);
    TCHAR           pBuffer[MAX_PATH];
    memset(pBuffer,0,sizeof(TCHAR)*(MAX_PATH));
    swprintf(pBuffer,nof(pBuffer),(LPCTSTR)strInitialPath);
    filedlg.m_ofn.lpstrFile = pBuffer;
    filedlg.m_ofn.nMaxFile  = MAX_PATH;
    if( IDOK == filedlg.DoModal() ) {
        POSITION pos = filedlg.GetStartPosition();  
		while (pos != NULL)   {  
			strFileName =  filedlg.GetNextPathName(pos);
		}  
    } 
    return strFileName;
}
void CGeneralSettingsDlg::OnBnClickedCheckUsedb()
{
    // TODO: Add your control notification handler code here
    m_Configs.bUseDB = (((CButton*)GetDlgItem(IDC_CHECK_USEDB))->GetCheck() == BST_CHECKED);
    UpdateUIStatus(MASK_WV);
}

void CGeneralSettingsDlg::OnBnClickedCheckWv()
{
    // TODO: Add your control notification handler code here
    m_Configs.bWideVine = (((CButton*)GetDlgItem(IDC_CHECK_WV))->GetCheck() == BST_CHECKED);
    UpdateUIStatus(MASK_WV);
}

void CGeneralSettingsDlg::OnBnClickedCheckFuse()
{
    // TODO: Add your control notification handler code here
    m_Configs.bFuse = (((CButton*)GetDlgItem(IDC_CHECK_FUSE))->GetCheck() == BST_CHECKED);
    UpdateUIStatus(MASK_FUSE);
}
void CGeneralSettingsDlg::OnBnClickedCheckXml()
{
    // TODO: Add your control notification handler code here
    m_Configs.bIsBin = (((CButton*)GetDlgItem(IDC_CHECK_XML))->GetCheck() == BST_UNCHECKED);
    if( m_Configs.bIsBin) {
        ((CButton*)GetDlgItem(IDC_CHECK_BIN))->SetCheck(BST_CHECKED);
    } else {
        ((CButton*)GetDlgItem(IDC_CHECK_BIN))->SetCheck(BST_UNCHECKED);
    }
}

void CGeneralSettingsDlg::OnBnClickedCheckBin()
{
    // TODO: Add your control notification handler code here
    m_Configs.bIsBin = (((CButton*)GetDlgItem(IDC_CHECK_BIN))->GetCheck() == BST_CHECKED);
    if( m_Configs.bIsBin) {
        ((CButton*)GetDlgItem(IDC_CHECK_XML))->SetCheck(BST_UNCHECKED);
    } else {
        ((CButton*)GetDlgItem(IDC_CHECK_XML))->SetCheck(BST_CHECKED);
    }
}

BOOL CGeneralSettingsDlg::SaveConfig()
{
    if(m_Configs.bWideVine||m_Configs.bFuse) {
    } else {
        AfxMessageBox(TEXT("Must Select a operation"));
        return FALSE;
    }
    m_Configs.bFuse         = (((CButton*)GetDlgItem(IDC_CHECK_FUSE         ))->GetCheck() == BST_CHECKED);
    m_Configs.bWideVine     = (((CButton*)GetDlgItem(IDC_CHECK_WV           ))->GetCheck() == BST_CHECKED);
    m_Configs.bWideVineRead = (((CButton*)GetDlgItem(IDC_CHECK_WVREAD       ))->GetCheck() == BST_CHECKED);
    m_Configs.bUseDB        = (((CButton*)GetDlgItem(IDC_CHECK_USEDB        ))->GetCheck() == BST_CHECKED);
    m_Configs.bAutoTest     = (((CButton*)GetDlgItem(IDC_CHECK_AUTO         ))->GetCheck() == BST_CHECKED);
    m_Configs.bOnlyAt       = (((CButton*)GetDlgItem(IDC_CHECK_ONLYAT       ))->GetCheck() == BST_CHECKED);
    m_Configs.bIsBin        = (((CButton*)GetDlgItem(IDC_CHECK_BIN          ))->GetCheck() == BST_CHECKED);
    m_Configs.bForceWrite   = (((CButton*)GetDlgItem(IDC_CHECK_FORCEWRITE   ))->GetCheck() == BST_CHECKED);
    m_Configs.bReboot       = (((CButton*)GetDlgItem(IDC_CHECK_REBOOT       ))->GetCheck() == BST_CHECKED);
    m_Configs.bFuseRead     = (((CButton*)GetDlgItem(IDC_CHECK_READ_FUSE    ))->GetCheck() == BST_CHECKED);
    m_Configs.bIsBinScript  = (((CButton*)GetDlgItem(IDC_CHECK_SCRIPT_BIN   ))->GetCheck() == BST_CHECKED);

    DWORD sel = ((CComboBox*)GetDlgItem(IDC_COMBO_MODE))->GetCurSel();
    if((sel >= 0 )&&(sel < dim(RebootMode))) {
        m_Configs.nMode = RebootMode[sel].type;
    } else {
        m_Configs.nMode = MODE_NONE;
    }



    if(m_Configs.bWideVine&&(!m_Configs.bUseDB)) {
        CString strText;
        GetDlgItemText(IDC_EDIT_KEYBOXBIN,strText);
        m_Configs.strXmlFileName = (LPCTSTR)strText;
    }
    if(m_Configs.bFuse) {
        CString strText;
        GetDlgItemText(IDC_EDIT_SCRIPT,strText);
        m_Configs.strFuseScriptFileName = (LPCTSTR)strText;
    }
    return TRUE;
}
BOOL CGeneralSettingsDlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
    if ( pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_ESCAPE )
    {
        return TRUE;
    }
    if ( pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN )
    {
        return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}
std::wstring CGeneralSettingsDlg::GetLocalString(std::wstring strKey)
{
    return m_LocalLang.GetLanStr(strKey);
}





void CGeneralSettingsDlg::OnBnClickedCheckReboot()
{
    // TODO: Add your control notification handler code here
}
