#include "stdafx.h"
#include "SqlApi.h"
#if 0

BOOL CWvToolDB::CommitWVkey(BOOL bPass,const wstring dev_sn)
{
    SYSTEMTIME time;
    if(!m_pRcdSet) return FALSE;
    if(!m_pRcdSet->IsOpen()) return FALSE;
    m_pRcdSet->Edit();
    if(bPass) {
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_WVFLG].c_str(),m_pConfigs->strFlagUsed)) {
            return FALSE;
        }
        /* for write date **/
        GetLocalTime(&time);
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_WOID].c_str(),time)) {
            return FALSE;
        }
#if 0
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_DEVSN].c_str(),dev_sn)) {
            return FALSE;
        }
#endif
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
#else 
BOOL CWvToolDB::CommitWVkey(BOOL bPass,const wstring dev_sn)
{
    BOOL            bRet    = FALSE;
    wstring         strSqlCmd;
    if(!Open(FALSE)) {
        return FALSE;
    }
    strSqlCmd = TEXT("update ") +  m_strTable +  TEXT("  set  ") + UpdataItemOnFinish(bPass) +
            TEXT(" where ") + GenerateCodition(m_pConfigs->strItemName[FLAG_DEVSN],dev_sn) + 
            TEXT(" and ")   + GenerateCodition(TEXT("custname"),m_pConfigs->strCustName);
    if(!m_pAdoDb->Execute(strSqlCmd.c_str())){
        return FALSE;
    }
    return TRUE;
}

#endif
#if 0
/*
get key witch flag = value from server then set value to busy
**/
BOOL CWvToolDB::GetWVkeyAndUpdateFlag(wstring &wvkey,wstring strSn)
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
                TEXT(" where ") + GenerateCodition(m_pConfigs->strItemName[FLAG_DEVSN],strSn) + 
                TEXT(" and ")   + GenerateCodition(TEXT("custname"),TEXT("BBY"));
    m_pAdoDb->BeginTransaction();
    if(m_pRcdSet->Open(strSqlCmd.c_str(), CADORecordset::openQuery)) {
        if(0 == m_pRcdSet->GetRecordCount()) {
            goto commit_tran;
        }   
#if 0 /*for bby **/
        m_pRcdSet->Edit();
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_WVFLG].c_str(),m_pConfigs->strFlagBusy)) {
            goto commit_tran;
        }
        if(!m_pRcdSet->Update()) {
            goto commit_tran;
        }
#endif
        if(!m_pRcdSet->GetFieldValue(m_pConfigs->strItemName[FLAG_WVKEY].c_str(),wvkey)) {
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
#else 
/*
get key witch flag = value from server then set value to busy
**/
BOOL CWvToolDB::GetWVkeyAndUpdateFlag(wstring &wvkey,wstring strSn)
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
                TEXT(" where ") + GenerateCodition(m_pConfigs->strItemName[FLAG_DEVSN],strSn) + 
                TEXT(" and ")   + GenerateCodition(TEXT("custname"),m_pConfigs->strCustName);
    m_pAdoDb->BeginTransaction();
    if(m_pRcdSet->Open(strSqlCmd.c_str(), CADORecordset::openQuery)) {
        if(0 == m_pRcdSet->GetRecordCount()) {
            goto commit_tran;
        }   
#if 0 /*for bby **/
        m_pRcdSet->Edit();
        if(!m_pRcdSet->SetFieldValue(m_pConfigs->strItemName[FLAG_WVFLG].c_str(),m_pConfigs->strFlagBusy)) {
            goto commit_tran;
        }
        if(!m_pRcdSet->Update()) {
            goto commit_tran;
        }
#endif
        if(!m_pRcdSet->GetFieldValue(m_pConfigs->strItemName[FLAG_WVKEY].c_str(),wvkey)) {
            goto commit_tran;
        }

    } else {
        goto commit_tran;
    }
    bRet = TRUE;
commit_tran:
    m_pAdoDb->CommitTransaction();
    m_pRcdSet->Close();
    return bRet;
}
#endif
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



