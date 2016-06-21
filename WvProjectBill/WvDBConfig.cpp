// WvDBConfig.cpp : implementation file
//

#include "stdafx.h"
#include "WvProjectBill.h"
#include "WvDBConfig.h"
#include "./ado/SqlApi.h"

// CWvDBConfig dialog

IMPLEMENT_DYNAMIC(CWvDBConfig, CDialog)

CWvDBConfig::CWvDBConfig( CIniSettingBase &Configs,CIniLocalLan &LocalLang,CWnd* pParent /*=NULL*/)
	: CDialog(CWvDBConfig::IDD, pParent), m_Configs(Configs),m_LocalLang(LocalLang)
{

}

CWvDBConfig::~CWvDBConfig()
{
}

void CWvDBConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_INFO, m_ListTable);
}


BEGIN_MESSAGE_MAP(CWvDBConfig, CDialog)
    ON_NOTIFY(NM_RCLICK, IDC_LIST_INFO, &CWvDBConfig::OnNMRClickListInfo)
    ON_COMMAND(ID_POPMENU_DELETE, &CWvDBConfig::OnPopmenuDelete)
    ON_COMMAND(ID_POPMENU_ADD, &CWvDBConfig::OnPopmenuAdd)
    ON_COMMAND(ID_POPMENU_INSERT, &CWvDBConfig::OnPopmenuInsert)
    ON_BN_CLICKED(IDC_BUTTON_OK, &CWvDBConfig::OnBnClickedButtonOk)
    ON_BN_CLICKED(IDC_BUTTON_LOAD, &CWvDBConfig::OnBnClickedButtonLoad)
END_MESSAGE_MAP()


// CWvDBConfig message handlers
BOOL CWvDBConfig::OnInitDialog()
{
    CDialog::OnInitDialog();
    DWORD dwAddr = m_Configs.GetIpv4();
    SendDlgItemMessage(IDC_IPADDRESS_DBSERVER,IPM_SETADDRESS,(WPARAM)0,(LPARAM)dwAddr);
    SetDlgItemText(IDC_EDIT_USER        ,m_Configs.strUserName.c_str());
    SetDlgItemText(IDC_EDIT_PWD         ,m_Configs.strPassword.c_str());
    SetDlgItemText(IDC_EDIT_DATABASENAME,m_Configs.strDataBaseName.c_str());
    SetDlgItemText(IDC_EDIT_TABLE       ,m_Configs.strDataBaseTable.c_str());
    CComboBox *hComSupportDatabase = (CComboBox *)GetDlgItem(IDC_COMBO_DBTYPE);
    for(int i = 0; i < m_Configs.nSupportDatabse; i++) {
        hComSupportDatabase->AddString(m_Configs.strSupportDatabse[i].c_str());
    }
    hComSupportDatabase->SetCurSel(0);
    for(int i = FLAG_DEVSN;i < FLAG_WVCNT ; i++ ) {
        SetDlgItemText(IDC_EDIT_TABLE_SN + i,m_Configs.strItemName[i].c_str());
    }
    OnUpdateTable();
    return TRUE;
}
void CWvDBConfig::OnNMRClickListInfo(NMHDR *pNMHDR, LRESULT *pResult)
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

void CWvDBConfig::OnPopmenuDelete()
{
    // TODO: Add your command handler code here
    POSITION pos = m_ListTable.GetFirstSelectedItemPosition();
    if(pos) {
        int nItem = m_ListTable.GetNextSelectedItem(pos);

        m_ListTable.DeleteItem(nItem);
    }
}

void CWvDBConfig::OnPopmenuAdd()
{
    // TODO: Add your command handler code here
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

    } else {
        int nItem = 0;
        nItem = m_ListTable.InsertItem(nItem,TEXT(""));
        m_ListTable.SetItemText(nItem,1,TEXT(""));
        m_ListTable.SetItemText(nItem,2,TEXT(""));

    }
}

void CWvDBConfig::OnPopmenuInsert()
{
    // TODO: Add your command handler code here
    POSITION pos = m_ListTable.GetFirstSelectedItemPosition();
    if(pos) {
        int nItem = m_ListTable.GetNextSelectedItem(pos);
        if(-1 == nItem) nItem = 0;
        if(-1 != nItem) {
            nItem = m_ListTable.InsertItem(nItem,TEXT(""));
            if(-1 != nItem) {
                m_ListTable.SetItemText(nItem,1,TEXT(""));
                m_ListTable.SetItemText(nItem,2,TEXT(""));
            }
        }
    }
}
BOOL CWvDBConfig::OnUpdateTable(BOOL Update2Table,BOOL bInitial)
{
    if(Update2Table) {
        if(bInitial) {
            m_ListTable.SetCellMargin(1.2);
            CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;
            m_ListTable.SetDefaultRowTrait(pRowTrait);
            // Create Columns
            //m_ListTable.InsertHiddenLabelColumn();  // Requires one never uses column 0
            CGridColumnTrait *pTraitColumn0 = new CGridColumnTraitEdit;
            if(pTraitColumn0) {
                m_ListTable.InsertColumnTrait(1, TEXT("Column Name"), LVCFMT_LEFT, 150,0, pTraitColumn0);
            }
            CGridColumnTraitCombo *pTraitColumn1 = new CGridColumnTraitCombo;
            if(pTraitColumn1) {
                pTraitColumn1->AddItem(0,TEXT("bigint"));
                pTraitColumn1->AddItem(1,TEXT("binary(50)"));
                pTraitColumn1->AddItem(2,TEXT("tinyint"));
                pTraitColumn1->AddItem(3,TEXT("char"));
                pTraitColumn1->AddItem(4,TEXT("datetime"));
                pTraitColumn1->AddItem(5,TEXT("nvarchar(50)"));
                pTraitColumn1->AddItem(6,TEXT("float"));
                m_ListTable.InsertColumnTrait(2, TEXT("DataType"), LVCFMT_LEFT, 150,1, pTraitColumn1);
            }
            CGridColumnTraitCombo *pTraitColumn2 = new CGridColumnTraitCombo;
            if(pTraitColumn2) {
                pTraitColumn2->AddItem(0,TEXT("Yes"));
                pTraitColumn2->AddItem(1,TEXT("No"));
                m_ListTable.InsertColumnTrait(3, TEXT("Can be null"), LVCFMT_LEFT, 80,2, pTraitColumn2);
            }
        }
        TCHAR szText[256] = {0};
        for(int nItem =0;nItem < m_Configs.nTableColCount; nItem ++) {
            /*swprintf(szText,nof(szText),TEXT("%d"),nItem); **/
            int nIndex = m_ListTable.InsertItem(nItem, m_Configs.strTableColumnName[nItem].c_str());
            /*m_ListTable.SetItemText(nIndex,0, m_Configs.strTableColumnName[nItem].c_str());**/
            m_ListTable.SetItemText(nIndex,1, m_Configs.strTableType[nItem].c_str());
            m_ListTable.SetItemText(nIndex,2, m_Configs.bCanBeNull[nItem]?TEXT("Yes"):TEXT("No"));
            
        }
        return TRUE;
    } else {
        //Save cable loss
        m_Configs.nTableColCount =0;
        int nTableColCount = 0;
        for (int i = 0; i < min(m_ListTable.GetItemCount(),MAX_COLUMN); i++) {
            CString strTableColumnName  = m_ListTable.GetItemText(i, 0);
            CString strTableType        = m_ListTable.GetItemText(i, 1);
            CString strCanBeNull        = m_ListTable.GetItemText(i, 2);
            strTableColumnName.Trim();
            strTableType.Trim();
            strCanBeNull.Trim();
            if (strTableColumnName.IsEmpty() || strTableType.IsEmpty()) continue;
            if(!strCanBeNull.CompareNoCase(TEXT("Yes"))) {
                m_Configs.bCanBeNull[nTableColCount] = true;
            } else {
                m_Configs.bCanBeNull[nTableColCount] = false;
            }
            m_Configs.strTableType[nTableColCount] = (LPCTSTR)strTableType;
            m_Configs.strTableColumnName[nTableColCount] = (LPCTSTR)strTableColumnName;
            nTableColCount ++;
        }
        m_Configs.nTableColCount =nTableColCount;
        return TRUE;
    }
    return FALSE;
}
BOOL CWvDBConfig::SaveConfig()
{
    DWORD   dwAddr;
    CString strText;
    TCHAR   szUserName[64]      = {0};
    TCHAR   szPassword[64]      = {0};
    TCHAR   szDBTable[64]       = {0};
    TCHAR   szDBName[64]        = {0};
    BOOL IsBlank = SendDlgItemMessage(IDC_IPADDRESS_DBSERVER,IPM_ISBLANK,(WPARAM)0,(LPARAM)0);
    if(IsBlank) {
        return FALSE;
    }
    GetDlgItemText(IDC_EDIT_USER         ,szUserName,64);
    GetDlgItemText(IDC_EDIT_PWD          ,szPassword,64);
    GetDlgItemText(IDC_EDIT_DATABASENAME ,szDBName  ,64);
    GetDlgItemText(IDC_EDIT_TABLE        ,szDBTable ,64);
    /*
    check name and password valid 
    **/
    CString strItemText;
    for(int i = FLAG_DEVSN;i < FLAG_WVCNT ; i++ ) {
        GetDlgItemText(IDC_EDIT_TABLE_SN + i ,strItemText);
        if(strItemText.IsEmpty()) {
            return FALSE;
        }
        m_Configs.strItemName[i] = (LPCTSTR)strItemText;
    }
    SendDlgItemMessage(IDC_IPADDRESS_DBSERVER,IPM_GETADDRESS,(WPARAM)0,(LPARAM)&dwAddr);
    m_Configs.SetIpv4(dwAddr);
    m_Configs.strUserName       = szUserName;
    m_Configs.strPassword       = szPassword;
    m_Configs.strDataBaseName   = szDBName;
    m_Configs.strDataBaseTable  = szDBTable;
    return OnUpdateTable(FALSE);
}
void CWvDBConfig::OnBnClickedButtonOk()
{
    // TODO: Add your control notification handler code here
    if(SaveConfig()){
        CDialog::OnOK();
    }
}

void CWvDBConfig::OnBnClickedButtonLoad()
{
    // TODO: Add your control notification handler code here
    CWvToolDB   m_WvToolDB;
    vector<CADOFieldInfo>       vField;
    if(!SaveConfig()) {
        AfxMessageBox(TEXT("Your Database Config Error!!!"));
        return;
    }
    m_WvToolDB.SetConfig(&m_Configs,&m_LocalLang,NULL);
	if(!m_WvToolDB.Open(TRUE)){
        /*SWNMessageBox(hWnd,GetLocalString(TEXT("IDS_DB_CONNECT_FAIL")).c_str(),
		GetLocalString(_T("ERROR")).c_str(),
		SW_OK); **/
		AfxMessageBox(TEXT("Open DataBase Failed!!!"));
        return;
    }	
	vField.clear();
    if(!m_WvToolDB.GetFieldInfo(vField)){
        /*SWNMessageBox(hWnd,GetLocalString(TEXT("IDS_DB_CONNECT_FAIL")).c_str(),
            GetLocalString(_T("ERROR")).c_str(),
            SW_OK); **/
		AfxMessageBox(TEXT("GetFieldInfo Failed!!!"));
        return;
    }

    vector<CADOFieldInfo> *pvInfo = & vField;

    if(pvInfo) {
		int nItem = 0;
		int iFieldCount = pvInfo->size();
		m_ListTable.DeleteAllItems();
		for(int i = 0; i < iFieldCount; i ++ ) {
	        int nIndex = m_ListTable.InsertItem(nItem, pvInfo[0][i].m_strName);
	        /*m_ListTable.SetItemText(nIndex,0, m_Configs.strTableColumnName[nItem].c_str());**/
			m_ListTable.SetItemText(nIndex,1, adoTypeTostr(pvInfo[0][i].m_nType).c_str());
            m_ListTable.SetItemText(nIndex,2, TEXT(""));
			nItem ++;
		}
	}
}
