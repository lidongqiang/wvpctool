#include "stdafx.h"
#include "SqlApi.h"
BOOL CWvToolDB::Open(const TCHAR *Server,const TCHAR *DataBase,const TCHAR *User,const TCHAR *Password,const TCHAR *Table,BOOL bReOpen) 
{
    LDEGMSGW((CLogger::DEBUG_INFO, TEXT("Open(%s,%s)"),Server,DataBase));
    m_strServer     = Server;
    m_strDataBase   = DataBase;
    m_strUser       = User;
    m_strPassword   = Password;
    m_strTable      = Table;
    if(m_pAdoDb&&!bReOpen) return TRUE;
    if(!m_pAdoDb) {
        m_pAdoDb = new CADODatabase();
    }
    if(!m_pAdoDb) return FALSE;
    wstring strConnection = TEXT("Provider=SQLOLEDB;Server=")+ m_strServer +
                    TEXT(";DATABASE=")  + m_strDataBase+
                    TEXT(";UID=")       + m_strUser +
                    TEXT(";PWD=")       + m_strPassword + TEXT(";");
    if(m_pAdoDb->Open(strConnection.c_str())) {
        return TRUE;
    } 
    m_strLastError = TEXT("Open:")+ m_pAdoDb->GetLastErrorString();  
    delete m_pAdoDb;
    m_pAdoDb = NULL;
    return FALSE;
}
BOOL CWvToolDB::Open(BOOL bReOpen)
{
    if(!m_pConfigs) return FALSE;
    return Open(m_pConfigs->strServer.c_str(),
            m_pConfigs->strDataBaseName.c_str(),
            m_pConfigs->strUserName.c_str(),
            m_pConfigs->strPassword.c_str(),
            m_pConfigs->strDataBaseTable.c_str(),bReOpen);
}
BOOL CWvToolDB::CommitWVkey(BOOL bPass,const wstring dev_sn)
{
    if(!m_pRcdSet) return FALSE;
    if(!m_pRcdSet->IsOpen()) return FALSE;
    m_pRcdSet->Edit();
    if(bPass) {
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_WVFLG].c_str(),m_pConfigs->strFlagUsed)) {
            return FALSE;
        }
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_DEVSN].c_str(),dev_sn)) {
            return FALSE;
        }
    } else {
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_WVFLG].c_str(),m_pConfigs->strFlagUnsd)) {
            return FALSE;
        }
    }
    if(!m_pRcdSet->Update()) {
        return FALSE;
    }
    return TRUE;
}
BOOL CWvToolDB::OpenTableForAddNew(std::wstring strTableName)
{
    TCHAR           szPrompt[128];
    wstring         Codition,Items;
    TCHAR           strSqlQuery[512];
    if(!Open(FALSE)) {
        /*m_strLastError = GetLocalString(TEXT("IDS_ERROR_DB_CONNECT_FAIL")); **/
        return FALSE;
    }
    if(!m_pRcdSet) {
        m_pRcdSet = new CADORecordset(m_pAdoDb);
    } else {
        m_pRcdSet->Close();
    }
    if(!m_pRcdSet->Open(strTableName.c_str(), CADORecordset::openTable)) {
        return FALSE;
    }
    return TRUE;

}

BOOL CWvToolDB::AddSingleItem2Database(RECORD&vsRecord)
{
    if(!m_pRcdSet) {
        goto additem2database_exit;
    }
    if(!m_pRcdSet->IsOpen()) {
        goto additem2database_exit;
    }
    if(!m_pRcdSet->AddNew()) {
        goto additem2database_exit;
    }
    for(int j = FLAG_WVKEY; j  < FLAG_WVCNT; j ++ ) {
        if(vsRecord.m_strItem[j].empty()) continue;
        if(!m_pRcdSet->SetFieldValue(TypeToStr(j).c_str(),vsRecord.m_strItem[j])) {
            goto additem2database_exit;
        }
    }
    if(!m_pRcdSet->Update()) {
        goto additem2database_exit;
    }
    return TRUE;
additem2database_exit:
    return FALSE;
}

BOOL CWvToolDB::AddSingleItem2Database(RECORDA&vsRecord)
{
    if(!m_pRcdSet) {
        goto additem2database_exit;
    }
    if(!m_pRcdSet->IsOpen()) {
        goto additem2database_exit;
    }
    if(!m_pRcdSet->AddNew()) {
        goto additem2database_exit;
    }
    for(int j = FLAG_DEVSN; j  < FLAG_WVCNT; j ++ ) {
        if(vsRecord.m_strItem[j].empty()) continue;
        if(!m_pRcdSet->SetFieldValue(TypeToStr(j).c_str(),vsRecord.m_strItem[j])) {
            goto additem2database_exit;
        }
    }
    if(!m_pRcdSet->Update()) {
        goto additem2database_exit;
    }
    return TRUE;
additem2database_exit:
    return FALSE;
}

BOOL CWvToolDB::CreateTable(const TCHAR *strCmd)
{
    TCHAR           szPrompt[128];
    wstring         Codition,Items;
    TCHAR           strSqlQuery[512];
    if(!Open(FALSE)) {
        return FALSE;
    }
    if(!m_pAdoDb->Execute(strCmd)) {
        m_strLastError = TEXT("Execute:") + m_pAdoDb->GetLastErrorString();
        return FALSE;
    }
    Close();
    return TRUE;
}
/*
get key witch flag = value from server then set value to busy
**/
BOOL CWvToolDB::GetWVkeyAndUpdateFlag(wstring &wvkey)
{
    BOOL            bRet    = FALSE;
    wstring         strSqlCmd;
    if(!Open(FALSE)) {
        return FALSE;
    }
    if(!m_pRcdSet) {
        m_pRcdSet = new CADORecordset(m_pAdoDb);
    } else {
        m_pRcdSet->Close();
    }
    strSqlCmd = TEXT("select ") + GenItemFromConfigs() + TEXT(" from ")  + m_strTable + TEXT(" with (xlock,rowlock) ")
                TEXT(" where ") + GenerateCodition(m_pConfigs->strItemName[FLAG_WVFLG],m_pConfigs->strFlagUnsd);
    m_pAdoDb->BeginTransaction();
    if(m_pRcdSet->Open(strSqlCmd.c_str(), CADORecordset::openQuery)) {
        if(0 == m_pRcdSet->GetRecordCount()) {
            goto commit_tran;
        }       
        m_pRcdSet->Edit();
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_WVFLG].c_str(),m_pConfigs->strFlagBusy)) {
            goto commit_tran;
        }
        if(!m_pRcdSet->GetFieldValue(TEXT("wvkey"),wvkey)) {
            goto commit_tran;
        }
        if(!m_pRcdSet->Update()) {
            goto commit_tran;
        }
    } else {
        goto commit_tran;
    }
    bRet = TRUE;
commit_tran:
    m_pAdoDb->CommitTransaction();
    return bRet;
}
BOOL CWvToolDB::GetWVkey(const TCHAR *strItem,DWORD type,DWORD dwflags,wstring &wvkey)
{
    TCHAR           szPrompt[128];
    wstring         Codition,Items;
    TCHAR           strSqlQuery[512];
    if(!Open(FALSE)) {
        /*m_strLastError = GetLocalString(TEXT("IDS_ERROR_DB_CONNECT_FAIL")); **/
        return FALSE;
    }
    if(!m_pRcdSet) {
        m_pRcdSet = new CADORecordset(m_pAdoDb);
    } else {
        m_pRcdSet->Close();
    }
    swprintf(strSqlQuery,sizeof(strSqlQuery)/sizeof(strSqlQuery[0]),
        TEXT("update wvkey set dev_sn=1 where %s"),
        GenItemFromConfigs().c_str(),
        m_strTable.c_str(),
        GenerateCodition(m_pConfigs->strItemName[FLAG_WVFLG],TEXT("0")).c_str());
    if(m_pRcdSet->Open(strSqlQuery, CADORecordset::openQuery)) {
        if(0 == m_pRcdSet->GetRecordCount()) {
            /*swprintf(szPrompt,dim(szPrompt),GetLocalString(TEXT("IDS_DB_RECORD_NOFIND")).c_str(),
                TypeToStr(type).c_str(),strItem);
            m_strLastError  = szPrompt; **/
            return FALSE;
        }            
        if(!m_pRcdSet->GetFieldValue(TEXT("wvkey"),wvkey)) {
            /*swprintf(szPrompt,dim(szPrompt),
                GetLocalString(TEXT("IDS_ERROR_DB_GET_SS_FAIL")).c_str(),
                GetWriteFlag().c_str()); 
            m_strLastError  = szPrompt;**/
            return FALSE;
        }
    } else {
        /*swprintf(szPrompt,dim(szPrompt),
            GetLocalString(TEXT("IDS_ERROR_DB_GET_SS_FAIL")).c_str(),
            GetItemStr().c_str()); 
        m_strLastError  = szPrompt;**/
        return FALSE;
    }
    return TRUE;
}



