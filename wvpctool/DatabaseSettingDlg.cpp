// DatabaseSettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wvpctool.h"
#include "DatabaseSettingDlg.h"

// CDatabaseSettingDlg dialog

IMPLEMENT_DYNAMIC(CDatabaseSettingDlg, CDialog)

CDatabaseSettingDlg::CDatabaseSettingDlg( CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent /*=NULL*/)
	: CDialog(CDatabaseSettingDlg::IDD, pParent), m_Configs(Configs),m_LocalLang(LocalLang),m_WvToolDB(&Configs,&LocalLang)
{

}

CDatabaseSettingDlg::~CDatabaseSettingDlg()
{
}

void CDatabaseSettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_INFO, m_ListTable);
}


BEGIN_MESSAGE_MAP(CDatabaseSettingDlg, CDialog)
    ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(IDC_BT_OK, &CDatabaseSettingDlg::OnBnClickedBtOk)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, &CDatabaseSettingDlg::OnBnClickedButtonCreate)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BT_CANCEL, &CDatabaseSettingDlg::OnBnClickedBtCancel)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_INFO, &CDatabaseSettingDlg::OnLvnItemchangedListInfo)
    ON_NOTIFY(NM_RCLICK, IDC_LIST_INFO, &CDatabaseSettingDlg::OnNMRClickListInfo)
    ON_COMMAND(ID_POPMENU_DELETE, &CDatabaseSettingDlg::OnPopmenuDelete)
    ON_COMMAND(ID_POPMENU_ADD, &CDatabaseSettingDlg::OnPopmenuAdd)
    ON_COMMAND(ID_POPMENU_INSERT, &CDatabaseSettingDlg::OnPopmenuInsert)
    ON_MESSAGE(WM_SAVE_CONFIG,&CDatabaseSettingDlg::OnSaveConfig)
END_MESSAGE_MAP()

BOOL CDatabaseSettingDlg::OnInitDialog()
{
	// TODO: Add extra initialization here
    CDialog::OnInitDialog();
    m_LocalLang.TreeControls(m_hWnd,m_Configs.bDebug?TRUE:FALSE,this->IDD,false);
    DWORD dwAddr = m_Configs.GetIpv4();
    SendDlgItemMessage(IDC_IPADDRESS_DBSERVER,IPM_SETADDRESS,(WPARAM)0,(LPARAM)dwAddr);
    SetDlgItemText(IDC_EDIT_USER        ,m_Configs.strUserName.c_str());
    SetDlgItemText(IDC_EDIT_PWD         ,m_Configs.strPassword.c_str());
    SetDlgItemText(IDC_EDIT_DATABASENAME,m_Configs.strDataBaseName.c_str());
    SetDlgItemText(IDC_EDIT_TABLE       ,m_Configs.strDataBaseTable.c_str());


    /*Database key map **/
    for(int i = FLAG_DEVSN;i < FLAG_WVCNT; i++ ) {
        SetDlgItemText(IDC_EDIT_TABLE_DEVSN + i,m_Configs.strItemName[i].c_str());
    }
#if 1
    CComboBox *hComSupportDatabase = (CComboBox *)GetDlgItem(IDC_COMBO_DBTYPE);
    for(int i = 0; i < m_Configs.nSupportDatabse; i++) {
        hComSupportDatabase->AddString(m_Configs.strSupportDatabse[i].c_str());
    }
    hComSupportDatabase->SetCurSel(0);
#endif

    CComboBox *hComSnType = (CComboBox *)GetDlgItem(IDC_COMBO_SNTYPE);
    hComSnType->AddString(TEXT("DEV_SN"));
    hComSnType->AddString(TEXT("PCB_SN"));
    hComSnType->AddString(TEXT("HW_SN"));
    hComSnType->SetCurSel(m_Configs.nSnType);
    




    OnUpdateTable();
    return TRUE;  // return TRUE  unless you set the focus to a control
    }

// CDatabaseSettingDlg message handlers
LRESULT CDatabaseSettingDlg::OnSaveConfig(WPARAM wParam,LPARAM lParam)
{
    return SaveConfig();
}
void CDatabaseSettingDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    if(bShow) UpdateCtrlStatus();
}
BOOL CDatabaseSettingDlg::UpdateCtrlStatus()
{

    BOOL enable     = m_Configs.bUseDB&&m_Configs.bWideVine;
    BOOL bReadOnly  = !enable;
    ((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS_DBSERVER))->EnableWindow(enable);

    ((CEdit*)GetDlgItem(IDC_EDIT_USER))->SetReadOnly(bReadOnly);
    ((CEdit*)GetDlgItem(IDC_EDIT_PWD))->SetReadOnly(bReadOnly);
    ((CEdit*)GetDlgItem(IDC_COMBO_DBTYPE))->SetReadOnly(bReadOnly);
    ((CEdit*)GetDlgItem(IDC_EDIT_DATABASENAME))->SetReadOnly(bReadOnly);
    ((CEdit*)GetDlgItem(IDC_EDIT_TABLE))->SetReadOnly(bReadOnly); 

    ((CEdit*)GetDlgItem(IDC_EDIT_TABLE_WVKEY))->SetReadOnly(bReadOnly);
    ((CEdit*)GetDlgItem(IDC_EDIT_TABLE_WVFLG))->SetReadOnly(bReadOnly);
    ((CEdit*)GetDlgItem(IDC_EDIT_TABLE_DEVSN))->SetReadOnly(bReadOnly);
    ((CEdit*)GetDlgItem(IDC_EDIT_TABLE_WOID ))->SetReadOnly(bReadOnly);

    ((CComboBox*)GetDlgItem(IDC_COMBO_DBTYPE))->EnableWindow(enable);
    ((CComboBox*)GetDlgItem(IDC_COMBO_SNTYPE))->EnableWindow(enable);
    ((CButton*)GetDlgItem(IDC_BUTTON_CREATE))->EnableWindow(enable);

    return TRUE;
}

BOOL CDatabaseSettingDlg::OnUpdateTable(vector<CADOFieldInfo> * pvInfo,BOOL bInitial)
{
    if(bInitial) {
#if 1
        m_ListTable.SetExtendedStyle(m_ListTable.GetExtendedStyle()|LVS_EX_GRIDLINES);
        m_ListTable.InsertColumn(0,TEXT("Item"),0,140);
        m_ListTable.InsertColumn(1,TEXT("Type"),0,140);
#else
        m_ListTable.SetCellMargin(1.2);
        CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;
        m_ListTable.SetDefaultRowTrait(pRowTrait);
        // Create Columns
        // m_ListTable.InsertHiddenLabelColumn();  
        // Requires one never uses column 0
        CGridColumnTrait *pTraitColumn0 = new CGridColumnTraitEdit;
        if(pTraitColumn0) {
            m_ListTable.InsertColumnTrait(1, TEXT("Column Name"), LVCFMT_LEFT, 150,0, pTraitColumn0);
        }
        CGridColumnTrait *pTraitColumnx = new CGridColumnTraitEdit;
        if(pTraitColumn0) {
            m_ListTable.InsertColumnTrait(2, TEXT("Column Name"), LVCFMT_LEFT, 150,0, pTraitColumnx);
        }	
#endif
    }
	if(pvInfo) {
		int nItem = 0;
		int iFieldCount = pvInfo->size();
		m_ListTable.DeleteAllItems();
		for(int i = 0; i < iFieldCount; i ++ ) {
	        int nIndex = m_ListTable.InsertItem(nItem, pvInfo[0][i].m_strName);
	        /*m_ListTable.SetItemText(nIndex,0, m_Configs.strTableColumnName[nItem].c_str());**/
			m_ListTable.SetItemText(nIndex,1, adoTypeTostr(pvInfo[0][i].m_nType).c_str());
			nItem ++;
		}
	}
    return TRUE;
}
void CDatabaseSettingDlg::OnBnClickedBtOk()
{
    // TODO: Add your control notification handler code here
    if(SaveConfig()) {
        OnOK();
    }
}

BOOL CDatabaseSettingDlg::SaveConfig()
{
    DWORD   dwAddr;
    TCHAR   szUserName[64]      = {0};
    TCHAR   szPassword[64]      = {0};
    TCHAR   szDBTable[64]       = {0};
    TCHAR   szDBName[64]        = {0};

    /*Database connect option **/
    BOOL IsBlank = SendDlgItemMessage(IDC_IPADDRESS_DBSERVER,IPM_ISBLANK,(WPARAM)0,(LPARAM)0);
    if(IsBlank) {
        AfxMessageBox(TEXT("IP  Address can not be blank"));
        return FALSE;
    }
    GetDlgItemText(IDC_EDIT_USER         ,szUserName,64);
    GetDlgItemText(IDC_EDIT_PWD          ,szPassword,64);
    GetDlgItemText(IDC_EDIT_DATABASENAME ,szDBName  ,64);
    GetDlgItemText(IDC_EDIT_TABLE        ,szDBTable ,64);

    /*Database key map **/
    CString strItemText;
    for(int i = FLAG_DEVSN;i < FLAG_WVCNT; i++ ) {
        GetDlgItemText(IDC_EDIT_TABLE_DEVSN + i,strItemText);
        m_Configs.strItemName[i] = (LPCTSTR)strItemText;
    }

    CComboBox *hComSnType = (CComboBox *)GetDlgItem(IDC_COMBO_SNTYPE);
    m_Configs.nSnType = hComSnType->GetCurSel();
    
    SendDlgItemMessage(IDC_IPADDRESS_DBSERVER,IPM_GETADDRESS,(WPARAM)0,(LPARAM)&dwAddr);
    m_Configs.SetIpv4(dwAddr);
    m_Configs.strUserName       = szUserName;
    m_Configs.strPassword       = szPassword;
    m_Configs.strDataBaseName   = szDBName;
    m_Configs.strDataBaseTable  = szDBTable;
    return TRUE;/* OnUpdateTable(FALSE); **/
}
void CDatabaseSettingDlg::OnBnClickedButtonCreate()
{
	vector<CADOFieldInfo>       vField;
    if(!SaveConfig()) {
        AfxMessageBox(TEXT("Your Database Config Error!!!"));
        return;
    }	
	if(!m_WvToolDB.Open(m_Configs.strServer.c_str(),
		m_Configs.strDataBaseName.c_str(),
		m_Configs.strUserName.c_str(),
		m_Configs.strPassword.c_str(),
		m_Configs.strDataBaseTable.c_str(),
		TRUE)){
		AfxMessageBox(GetLocalString(TEXT("IDS_ERROR_OPEN_DB")).c_str());
        return;
    }	
	vField.clear();
    if(!m_WvToolDB.GetFieldInfo(vField)){
        /*SWNMessageBox(hWnd,GetLocalString(TEXT("IDS_DB_CONNECT_FAIL")).c_str(),
            GetLocalString(_T("ERROR")).c_str(),
            SW_OK); **/
        AfxMessageBox(GetLocalString(TEXT("IDS_ERROR_GET_DBINFO")).c_str());

        return;
    }
	OnUpdateTable(&vField,FALSE);
}

void CDatabaseSettingDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default

    CDialog::OnClose();
}

void CDatabaseSettingDlg::OnBnClickedBtCancel()
{
    // TODO: Add your control notification handler code here
    CDialog::OnCancel();
}

void CDatabaseSettingDlg::OnLvnItemchangedListInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;
    /*if(IDC_LIST_INFO == pNMLV->hdr.idFrom) {
        int nCount = m_ListTable.GetItemCount();
        if((nCount - 1) == pNMLV->iItem) {
            CString strName = m_ListTable.GetItemText(pNMLV->iItem, 0);
            CString strType = m_ListTable.GetItemText(pNMLV->iItem, 2);
            if((strName.IsEmpty())&&(strType.IsEmpty())) {
                return;
            }
            int nIndex = m_ListTable.InsertItem(nCount, TEXT(""));
            m_ListTable.SetItemText(nIndex,1,TEXT(""));
            m_ListTable.SetItemText(nIndex,2,TEXT(""));
        }
    }
    **/
}

void CDatabaseSettingDlg::OnNMRClickListInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: Add your control notification handler code here
    CMenu Menu, *pPopup;
    if(!Menu.LoadMenu(IDR_ADD_DELETE)) {
        return;
    }
    if(!(pPopup = Menu.GetSubMenu(0))) {
        return;
    }
    CPoint myPoint;
    ClientToScreen(&myPoint);
    GetCursorPos(&myPoint);
    pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, myPoint.x, myPoint.y,this);
    *pResult = 0;
}

void CDatabaseSettingDlg::OnPopmenuDelete()
{
    // TODO: Add your command handler code here
    POSITION pos = m_ListTable.GetFirstSelectedItemPosition();
    if(pos) {
        int nItem = m_ListTable.GetNextSelectedItem(pos);

        m_ListTable.DeleteItem(nItem);
    }
}

void CDatabaseSettingDlg::OnPopmenuAdd()
{
    // TODO: Add your command handler code here
    POSITION pos = m_ListTable.GetFirstSelectedItemPosition();
    if(pos) {
        int nItem = m_ListTable.GetNextSelectedItem(pos);
        if(-1 != nItem) {
            nItem = m_ListTable.InsertItem(nItem+1,TEXT(""));
            if(-1 != nItem) {
                m_ListTable.SetItemText(nItem,1,TEXT(""));
                m_ListTable.SetItemText(nItem,2,TEXT(""));
            }
        }

    }
}

void CDatabaseSettingDlg::OnPopmenuInsert()
{
    // TODO: Add your command handler code here
    POSITION pos = m_ListTable.GetFirstSelectedItemPosition();
    if(pos) {
        int nItem = m_ListTable.GetNextSelectedItem(pos);
        if(-1 != nItem) {
            nItem = m_ListTable.InsertItem(nItem,TEXT(""));
            if(-1 != nItem) {
                m_ListTable.SetItemText(nItem,1,TEXT(""));
                m_ListTable.SetItemText(nItem,2,TEXT(""));
            }
        }
    }
}
BOOL CDatabaseSettingDlg::PreTranslateMessage(MSG* pMsg)
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

std::wstring CDatabaseSettingDlg::GetLocalString(std::wstring strKey)
{
    return m_LocalLang.GetLanStr(strKey);
}


