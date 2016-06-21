//
//  MODULE: Ado2.cpp
//
//	AUTHOR: Carlos Antollini 
//
//  mailto: cantollini@hotmail.com
//
//	Date: 07/02/2003
//
//	Version 2.10
//
#include "stdafx.h"
#include "ado2.h"
#define ChunkSize 100

// Convert wstring to string
static std::string wstr2str(const std::wstring& arg)
{
	int requiredSize;
	requiredSize = WideCharToMultiByte(CP_ACP,0,arg.c_str(),arg.length(),NULL,0,NULL,NULL);
	std::string res;
	if (requiredSize<=0) {
		res = "";
		return res;
	}
	res.assign(requiredSize,'\0');
	WideCharToMultiByte(CP_ACP,0,arg.c_str(),arg.length(),const_cast<char*>(res.data()),requiredSize,NULL,NULL);
	return res;
}
// Convert string to wstring
static std::wstring str2wstr(const std::string& arg)
{
	int requiredSize;
	requiredSize = MultiByteToWideChar(CP_ACP,0,arg.c_str(),arg.length(),NULL,0);
	std::wstring res;
	if (requiredSize<=0) {
		res = L"";
		return res;
	}
	res.assign(requiredSize,L'\0');
	MultiByteToWideChar(CP_ACP,0,arg.c_str(),arg.length(),const_cast<wchar_t *>(res.data()),requiredSize);
	return res;
}

static void RTrim(std::wstring &str, const std::wstring& chars = L" \t")
{
    str.erase(str.find_last_not_of(chars)+1);
}

static void LTrim(std::wstring &str, const std::wstring& chars = L" \t" )
{
    str.erase(0, str.find_first_not_of(chars));
}

static void Trim( std::wstring& str , const std::wstring& chars = L" \t" )
{
    str.erase(str.find_last_not_of(chars)+1);
    str.erase(0, str.find_first_not_of(chars));
}
DWORD CADODatabase::GetRecordCount(_RecordsetPtr m_pRs)
{
	DWORD numRows = 0;
	
	numRows = m_pRs->GetRecordCount();

	if(numRows == -1)
	{
		numRows = 0;
		if(m_pRs->EndOfFile != VARIANT_TRUE)
			m_pRs->MoveFirst();

		while(m_pRs->EndOfFile != VARIANT_TRUE)
		{
			numRows++;
			m_pRs->MoveNext();
		}
		if(numRows > 0)
			m_pRs->MoveFirst();
	}
	return numRows;
}

BOOL CADODatabase::Open(LPCTSTR lpstrConnection, LPCTSTR lpstrUserID, LPCTSTR lpstrPassword)
{
	HRESULT hr = S_OK;

	if(IsOpen())
		Close();

	if(_tcscmp(lpstrConnection, _T("")) != 0)
		m_strConnection = lpstrConnection;

    ASSERT(!m_strConnection.empty());

	try
	{
		if(m_nConnectionTimeout != 0) {
            m_pConnection->PutConnectionTimeout(m_nConnectionTimeout);
        }
		hr = m_pConnection->Open(_bstr_t(lpstrConnection), _bstr_t(lpstrUserID), _bstr_t(lpstrPassword), NULL);
		return hr == S_OK;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
	
}

void CADODatabase::dump_com_error(_com_error &e)
{
	TCHAR ErrorStr[1024];
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
	swprintf(ErrorStr,sizeof(ErrorStr)/sizeof(ErrorStr[0]),
        _T("CADODatabase Error\r\nCode = %08lx\r\nCode meaning = %s\r\nSource = %s\r\nDescription = %s\r\n"),
		e.Error(), 
        e.ErrorMessage(), 
        (LPCTSTR)bstrSource,
        (LPCTSTR)bstrDescription);
    m_strErrorDescription   = (LPCTSTR)bstrDescription ;
    m_strLastError          = wstring(_T("Connection String = "))/* + GetConnectionString() **/+ _T('\r\n') + ErrorStr;
    m_dwLastError           = e.Error(); 
}

BOOL CADODatabase::IsOpen()
{
	if(m_pConnection )
		return m_pConnection->GetState() != adStateClosed;
	return FALSE;
}

void CADODatabase::Close()
{
	if(IsOpen())
		m_pConnection->Close();
}


///////////////////////////////////////////////////////
//
// CADORecordset Class
//

CADORecordset::CADORecordset()
{
    m_pRecordset        = NULL;
    m_pCmd              = NULL;
    m_strQuery          = _T("");
    m_strLastError      = _T("");
    m_dwLastError       = 0;
    m_pRecBinding       = NULL;
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    m_pCmd.CreateInstance(__uuidof(Command));
    m_nEditStatus       = CADORecordset::dbEditNone;
    m_nSearchDirection  = CADORecordset::searchForward;
    m_pConnection       = NULL;
}

CADORecordset::CADORecordset(CADODatabase* pAdoDatabase)
{
    m_pRecordset        = NULL;
    m_pCmd              = NULL;
    m_strQuery          = _T("");
    m_strLastError      = _T("");
    m_dwLastError       = 0;
    m_pRecBinding       = NULL;
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    m_pCmd.CreateInstance(__uuidof(Command));
    m_nEditStatus       = CADORecordset::dbEditNone;
    m_nSearchDirection  = CADORecordset::searchForward;

    m_pConnection = pAdoDatabase->GetActiveConnection();
}

BOOL CADORecordset::Open(_ConnectionPtr mpdb, LPCTSTR lpstrExec, int nOption)
{	
    Close();
 
    if(_tcscmp(lpstrExec, _T("")) != 0)
        m_strQuery = lpstrExec;

    ASSERT(!m_strQuery.empty());

    if(m_pConnection == NULL)
        m_pConnection = mpdb;

    LTrim(m_strQuery);

    BOOL bIsSelect = m_strQuery.substr(0,_tcslen(_T("select "))).compare(_T("select ")) == 0 && nOption == openUnknown;
    try {
        m_pRecordset->CursorType        = adOpenStatic;
        m_pRecordset->CursorLocation    = adUseClient;
        if(bIsSelect || nOption == openQuery || nOption == openUnknown) {
            m_pRecordset->Open(m_strQuery.c_str(),
                _variant_t((IDispatch*)mpdb,
                TRUE),
                adOpenStatic,
                adLockOptimistic,
                adCmdUnknown);

        } else if(nOption == openTable) {
            m_pRecordset->Open(m_strQuery.c_str(),
                _variant_t((IDispatch*)mpdb,
                TRUE),
                adOpenKeyset,
                adLockOptimistic,
                adCmdTable);

        } else if(nOption == openStoredProc) {
            m_pCmd->ActiveConnection        = mpdb;
            m_pCmd->CommandText             = _bstr_t(m_strQuery.c_str());
            m_pCmd->CommandType             = adCmdStoredProc;
            m_pConnection->CursorLocation   = adUseClient;
            m_pRecordset = m_pCmd->Execute(NULL, NULL, adCmdText);
        } else {
            TRACE( _T("Unknown parameter. %d"), nOption);
            return FALSE;
        }
    } catch(_com_error &e) {
        dump_com_error(e);
        return FALSE;
    }
    return m_pRecordset != NULL && m_pRecordset->GetState()!= adStateClosed;
}

BOOL CADORecordset::Open(LPCTSTR lpstrExec, int nOption)
{
	ASSERT(m_pConnection != NULL);
	ASSERT(m_pConnection->GetState() != adStateClosed);
	return Open(m_pConnection, lpstrExec, nOption);
}

BOOL CADORecordset::OpenSchema(int nSchema, LPCTSTR SchemaID )
{
	try
	{
		_variant_t vtSchemaID = vtMissing;

		if(_tcslen(SchemaID) != 0)
		{
			vtSchemaID = SchemaID;
			nSchema = adSchemaProviderSpecific;
		}
			
		m_pRecordset = m_pConnection->OpenSchema((enum SchemaEnum)nSchema, vtMissing, vtSchemaID);
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::Requery()
{
	if(IsOpen())
	{
		try
		{
			m_pRecordset->Requery(adExecuteRecord);
		}
		catch(_com_error &e)
		{
			dump_com_error(e);
			return FALSE;
		}
	}
	return TRUE;
}


BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, double& dbValue)
{	
	double val = (double)NULL;
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch(vtFld.vt)
		{
		case VT_R4:
			val = vtFld.fltVal;
			break;
		case VT_R8:
			val = vtFld.dblVal;
			break;
		case VT_DECIMAL:
			val = vtFld.decVal.Lo32;
			val *= (vtFld.decVal.sign == 128)? -1 : 1;
			val /= pow((float)10, vtFld.decVal.scale); 
			break;
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_I2:
		case VT_I4:
			val = vtFld.lVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_CY:   
			vtFld.ChangeType(VT_R8);
			val = vtFld.dblVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = vtFld.dblVal;
		}
		dbValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADORecordset::GetFieldValue(int nIndex, double& dbValue)
{	
	double val = (double)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch(vtFld.vt)
		{
		case VT_R4:
			val = vtFld.fltVal;
			break;
		case VT_R8:
			val = vtFld.dblVal;
			break;
		case VT_DECIMAL:
			val = vtFld.decVal.Lo32;
			val *= (vtFld.decVal.sign == 128)? -1 : 1;
			val /= pow((float)10, vtFld.decVal.scale); 
			break;
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_I2:
		case VT_I4:
			val = vtFld.lVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_CY:  
			vtFld.ChangeType(VT_R8);
			val = vtFld.dblVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = 0;
		}
		dbValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, long& lValue)
{
	long val = (long)NULL;
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		if(vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.lVal;
		lValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, long& lValue)
{
	long val = (long)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		if(vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.lVal;
		lValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, unsigned long& ulValue)
{
	long val = (long)NULL;
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		if(vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.ulVal;
		ulValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, unsigned long& ulValue)
{
	long val = (long)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		if(vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.ulVal;
		ulValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, unsigned char& ucValue)
{
    int val   = 0;
    _variant_t      vtFld;
    try {
    vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
    switch(vtFld.vt) {
    case VT_BOOL:
        val = vtFld.boolVal;
        break;
    case VT_I2:
    case VT_UI1:
        val = vtFld.iVal;
        break;
    case VT_INT:
        val = vtFld.intVal;
        break;
    case VT_NULL:
    case VT_EMPTY:
        val = 0;
        break;
    default:
        val = vtFld.iVal;
    }	
    ucValue = val;
    return TRUE;
    } catch(_com_error &e) {
        dump_com_error(e);
        return FALSE;
    }

}
BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, int& nValue)
{
	int val = NULL;
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch(vtFld.vt)
		{
		case VT_BOOL:
			val = vtFld.boolVal;
			break;
		case VT_I2:
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = vtFld.iVal;
		}	
		nValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, int& nValue)
{
	int val = (int)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch(vtFld.vt)
		{
		case VT_BOOL:
			val = vtFld.boolVal;
			break;
		case VT_I2:
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = vtFld.iVal;
		}	
		nValue = val;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, wstring& strValue, wstring strDateFormat)
{
	wstring str;
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch(vtFld.vt) 
		{
		case VT_R4:
			str = DblToStr(vtFld.fltVal);
			break;
		case VT_R8:
			str = DblToStr(vtFld.dblVal);
			break;
		case VT_BSTR:
			str = vtFld.bstrVal;
			break;
		case VT_I2:
		case VT_UI1:
			str = IntToStr(vtFld.iVal);
			break;
		case VT_INT:
			str = IntToStr(vtFld.intVal);
			break;
		case VT_I4:
			str = LongToStr(vtFld.lVal);
			break;
		case VT_UI4:
			str = ULongToStr(vtFld.ulVal);
			break;
		case VT_DECIMAL:
			{
			double val = vtFld.decVal.Lo32;
			val *= (vtFld.decVal.sign == 128)? -1 : 1;
			val /= pow((float)10, vtFld.decVal.scale); 
			str = DblToStr(val);
			}
			break;
		case VT_DATE:
			{
#if 0 /*lanshh **/
				COleDateTime dt(vtFld);

				if(strDateFormat.IsEmpty())
					strDateFormat = _T("%Y-%m-%d %H:%M:%S");
				str = dt.Format(strDateFormat);
#endif
			}
			break;
#if 0
		case VT_CY:		
			{
				vtFld.ChangeType(VT_R8);
 
				wstring str;
				str.Format(_T("%f"), vtFld.dblVal);
 
				_TCHAR pszFormattedNumber[64];
 
				//	LOCALE_USER_DEFAULT
				if(GetCurrencyFormat(
						LOCALE_USER_DEFAULT,	// locale for which string is to be formatted 
						0,						// bit flag that controls the function's operation
						str,					// pointer to input number string
						NULL,					// pointer to a formatting information structure
												//	NULL = machine default locale settings
						pszFormattedNumber,		// pointer to output buffer
						63))					// size of output buffer
				{
					str = pszFormattedNumber;
				}
			}
			break;
#endif
		case VT_EMPTY:
		case VT_NULL:
            str.clear();
			break;
		case VT_BOOL:
			str = vtFld.boolVal == VARIANT_TRUE? _T('T'):_T('F');
			break;
		default:
            str.clear();
			return FALSE;
		}
		strValue = str;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, wstring& strValue, wstring strDateFormat)
{
	wstring str;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch(vtFld.vt) 
		{
		case VT_R4:
			str = DblToStr(vtFld.fltVal);
			break;
		case VT_R8:
			str = DblToStr(vtFld.dblVal);
			break;
		case VT_BSTR:
			str = vtFld.bstrVal;
			break;
		case VT_I2:
		case VT_UI1:
			str = IntToStr(vtFld.iVal);
			break;
		case VT_INT:
			str = IntToStr(vtFld.intVal);
			break;
		case VT_I4:
			str = LongToStr(vtFld.lVal);
			break;
		case VT_UI4:
			str = ULongToStr(vtFld.ulVal);
			break;
		case VT_DECIMAL:
			{
			double val = vtFld.decVal.Lo32;
			val *= (vtFld.decVal.sign == 128)? -1 : 1;
			val /= pow((float)10, vtFld.decVal.scale); 
			str = DblToStr(val);
			}
			break;
		case VT_DATE:
			{
#if  0 /*lanshh **/
				COleDateTime dt(vtFld);
				
				if(strDateFormat.IsEmpty())
					strDateFormat = _T("%Y-%m-%d %H:%M:%S");
				str = dt.Format(strDateFormat);
#endif
			}
			break;
#if 0
		case VT_CY:	
			{
				vtFld.ChangeType(VT_R8);
 
				wstring str;
				str.Format(_T("%f"), vtFld.dblVal);
 
				_TCHAR pszFormattedNumber[64];
 
				//	LOCALE_USER_DEFAULT
				if(GetCurrencyFormat(
						LOCALE_USER_DEFAULT,	// locale for which string is to be formatted 
						0,						// bit flag that controls the function's operation
						str,					// pointer to input number string
						NULL,					// pointer to a formatting information structure
												//	NULL = machine default locale settings
						pszFormattedNumber,		// pointer to output buffer
						63))					// size of output buffer
				{
					str = pszFormattedNumber;
				}
			}
			break;
#endif
		case VT_BOOL:
			str = vtFld.boolVal == VARIANT_TRUE? _T('T'):_T('F');
			break;
		case VT_EMPTY:
		case VT_NULL:
            str.clear();
			break;
		default:
            str.clear();
			return FALSE;
		}
		strValue = str;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, SYSTEMTIME& time)
{
	_variant_t vtFld;
	DOUBLE vt;
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch(vtFld.vt) 
		{
		case VT_DATE:
			{
                vt = vtFld.date;
                VariantTimeToSystemTime(vt,&time);
			}
			break;
		case VT_EMPTY:
		case VT_NULL:
            memset(&time,0,sizeof(SYSTEMTIME));
			/*time.SetStatus(COleDateTime::null); **/
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, SYSTEMTIME& time)
{
	_variant_t  vtFld;
	_variant_t  vtIndex;
    DOUBLE      vt;
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch(vtFld.vt) 
		{
		case VT_DATE:
			{
                vt = vtFld.date;
                VariantTimeToSystemTime(vt,&time);
			}
			break;
		case VT_EMPTY:
		case VT_NULL:
            memset(&time,0,sizeof(SYSTEMTIME));
			/*time.SetStatus(COleDateTime::null);**/
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, bool& bValue)
{
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch(vtFld.vt) 
		{
		case VT_BOOL:
			bValue = vtFld.boolVal == VARIANT_TRUE? true: false;
			break;
		case VT_EMPTY:
		case VT_NULL:
			bValue = false;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, bool& bValue)
{
	_variant_t vtFld;
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch(vtFld.vt) 
		{
		case VT_BOOL:
			bValue = vtFld.boolVal == VARIANT_TRUE? true: false;
			break;
		case VT_EMPTY:
		case VT_NULL:
			bValue = false;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
#if USEAFX

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, COleCurrency& cyValue)
{
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch(vtFld.vt) 
		{
		case VT_CY:
			cyValue = (CURRENCY)vtFld.cyVal;
			break;
		case VT_EMPTY:
		case VT_NULL:
			{
			cyValue = COleCurrency();
			cyValue.m_status = COleCurrency::null;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
BOOL CADORecordset::GetFieldValue(int nIndex, COleCurrency& cyValue)
{
	_variant_t vtFld;
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch(vtFld.vt) 
		{
		case VT_CY:
			cyValue = (CURRENCY)vtFld.cyVal;
			break;
		case VT_EMPTY:
		case VT_NULL:
			{
			cyValue = COleCurrency();
			cyValue.m_status = COleCurrency::null;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
#endif
BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, _variant_t& vtValue)
{
	try
	{
		vtValue = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, _variant_t& vtValue)
{
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtValue = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldNull(LPCTSTR lpFieldName)
{
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		return vtFld.vt == VT_NULL;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldNull(int nIndex)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		return vtFld.vt == VT_NULL;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldEmpty(LPCTSTR lpFieldName)
{
	_variant_t vtFld;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldEmpty(int nIndex)
{
	_variant_t vtFld;
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SetFieldEmpty(LPCTSTR lpFieldName)
{
	_variant_t vtFld;
	vtFld.vt = VT_EMPTY;
	
	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldEmpty(int nIndex)
{
	_variant_t vtFld;
	vtFld.vt = VT_EMPTY;

	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	return PutFieldValue(vtIndex, vtFld);
}


DWORD CADORecordset::GetRecordCount()
{
	DWORD nRows = 0;
	
	nRows = m_pRecordset->GetRecordCount();

	if(nRows == -1)
	{
		nRows = 0;
		if(m_pRecordset->EndOfFile != VARIANT_TRUE)
			m_pRecordset->MoveFirst();
		
		while(m_pRecordset->EndOfFile != VARIANT_TRUE)
		{
			nRows++;
			m_pRecordset->MoveNext();
		}
		if(nRows > 0)
			m_pRecordset->MoveFirst();
	}
	
	return nRows;
}

BOOL CADORecordset::IsOpen()
{
	if(m_pRecordset != NULL && IsConnectionOpen())
		return m_pRecordset->GetState() != adStateClosed;
	return FALSE;
}

void CADORecordset::Close()
{
	if(IsOpen())
	{
		if (m_nEditStatus != dbEditNone)
		      CancelUpdate();

		m_pRecordset->PutSort(_T(""));
		m_pRecordset->Close();	
	}
}


BOOL CADODatabase::Execute(LPCTSTR lpstrExec)
{
	ASSERT(m_pConnection != NULL);
	ASSERT(_tcscmp(lpstrExec, _T("")) != 0);
	_variant_t vRecords;
	
	m_nRecordsAffected = 0;

	try
	{
		m_pConnection->CursorLocation = adUseClient;
		m_pConnection->Execute(_bstr_t(lpstrExec), &vRecords, adExecuteNoRecords);
		m_nRecordsAffected = vRecords.iVal;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;	
	}
}

BOOL CADORecordset::RecordBinding(CADORecordBinding &pAdoRecordBinding)
{
	HRESULT hr;
	m_pRecBinding = NULL;

	//Open the binding interface.
	if(FAILED(hr = m_pRecordset->QueryInterface(__uuidof(IADORecordBinding), (LPVOID*)&m_pRecBinding )))
	{
		_com_issue_error(hr);
		return FALSE;
	}
	
	//Bind the recordset to class
	if(FAILED(hr = m_pRecBinding->BindToRecordset(&pAdoRecordBinding)))
	{
		_com_issue_error(hr);
		return FALSE;
	}
	return TRUE;
}

BOOL CADORecordset::GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo* fldInfo)
{
	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);
	
	return GetFieldInfo(pField, fldInfo);
}

BOOL CADORecordset::GetFieldInfo(int nIndex, CADOFieldInfo* fldInfo)
{
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return GetFieldInfo(pField, fldInfo);
}


BOOL CADORecordset::GetFieldInfo(FieldPtr pField, CADOFieldInfo* fldInfo)
{
	memset(fldInfo, 0, sizeof(CADOFieldInfo));

    _tcscpy_s(fldInfo->m_strName,sizeof(fldInfo->m_strName)/sizeof(fldInfo->m_strName[0]),(LPCTSTR)pField->GetName());
	fldInfo->m_lDefinedSize = pField->GetDefinedSize();
	fldInfo->m_nType        = pField->GetType();
	fldInfo->m_lAttributes  = pField->GetAttributes();
	if(!IsEof())
		fldInfo->m_lSize = pField->GetActualSize();
	return TRUE;
}

BOOL CADORecordset::GetChunk(LPCTSTR lpFieldName, wstring& strValue)
{
	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);
	
	return GetChunk(pField, strValue);
}

BOOL CADORecordset::GetChunk(int nIndex, wstring& strValue)
{
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);
	
	return GetChunk(pField, strValue);
}


BOOL CADORecordset::GetChunk(FieldPtr pField, wstring& strValue)
{
	wstring str;
	long lngSize, lngOffSet = 0;
	_variant_t varChunk;

	lngSize = pField->ActualSize;
	
    str.clear();
	while(lngOffSet < lngSize)
	{ 
		try
		{
			varChunk = pField->GetChunk(ChunkSize);
			
			str += varChunk.bstrVal;
			lngOffSet += ChunkSize;
		}
		catch(_com_error &e)
		{
			dump_com_error(e);
			return FALSE;
		}
	}

	lngOffSet = 0;
	strValue = str;
	return TRUE;
}

BOOL CADORecordset::GetChunk(LPCTSTR lpFieldName, LPVOID lpData)
{
	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

	return GetChunk(pField, lpData);
}

BOOL CADORecordset::GetChunk(int nIndex, LPVOID lpData)
{
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return GetChunk(pField, lpData);
}

BOOL CADORecordset::GetChunk(FieldPtr pField, LPVOID lpData)
{
	long lngSize, lngOffSet = 0;
	_variant_t varChunk;    
	UCHAR chData;
	HRESULT hr;
	long lBytesCopied = 0;

	lngSize = pField->ActualSize;
	
	while(lngOffSet < lngSize)
	{ 
		try
		{
			varChunk = pField->GetChunk(ChunkSize);

			//Copy the data only upto the Actual Size of Field.  
            for(long lIndex = 0; lIndex <= (ChunkSize - 1); lIndex++)
            {
                hr= SafeArrayGetElement(varChunk.parray, &lIndex, &chData);
                if(SUCCEEDED(hr))
                {
                    //Take BYTE by BYTE and advance Memory Location
                    //hr = SafeArrayPutElement((SAFEARRAY FAR*)lpData, &lBytesCopied ,&chData); 
					((UCHAR*)lpData)[lBytesCopied] = chData;
                    lBytesCopied++;
                }
                else
                    break;
            }
			lngOffSet += ChunkSize;
		}
		catch(_com_error &e)
		{
			dump_com_error(e);
			return FALSE;
		}
	}

	lngOffSet = 0;
	return TRUE;
}

BOOL CADORecordset::AppendChunk(LPCTSTR lpFieldName, LPVOID lpData, UINT nBytes)
{

	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

	return AppendChunk(pField, lpData, nBytes);
}


BOOL CADORecordset::AppendChunk(int nIndex, LPVOID lpData, UINT nBytes)
{
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return AppendChunk(pField, lpData, nBytes);
}

BOOL CADORecordset::AppendChunk(FieldPtr pField, LPVOID lpData, UINT nBytes)
{
	HRESULT hr;
	_variant_t varChunk;
	long lngOffset = 0;
	UCHAR chData;
	SAFEARRAY FAR *psa = NULL;
	SAFEARRAYBOUND rgsabound[1];

	try
	{
		//Create a safe array to store the array of BYTES 
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = nBytes;
		psa = SafeArrayCreate(VT_UI1,1,rgsabound);

		while(lngOffset < (long)nBytes)
		{
			chData	= ((UCHAR*)lpData)[lngOffset];
			hr = SafeArrayPutElement(psa, &lngOffset, &chData);

			if(FAILED(hr))
				return FALSE;
			
			lngOffset++;
		}
		lngOffset = 0;

		//Assign the Safe array  to a variant. 
		varChunk.vt = VT_ARRAY|VT_UI1;
		varChunk.parray = psa;

		hr = pField->AppendChunk(varChunk);

		if(SUCCEEDED(hr)) return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}

	return FALSE;
}

wstring CADORecordset::GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows)
{
	_bstr_t varOutput;
	_bstr_t varNull(_T(""));
	_bstr_t varCols(_T("\t"));
	_bstr_t varRows(_T("\r"));

	if(_tcslen(lpCols) != 0)
		varCols = _bstr_t(lpCols);

	if(_tcslen(lpRows) != 0)
		varRows = _bstr_t(lpRows);
	
	if(numRows == 0)
		numRows =(long)GetRecordCount();			
			
	varOutput = m_pRecordset->GetString(adClipString, numRows, varCols, varRows, varNull);

	return (LPCTSTR)varOutput;
}

wstring IntToStr(int nVal)
{
	wstring strRet;
	TCHAR buff[10];
	_itot_s(nVal,buff,sizeof(buff)/sizeof(buff[0]),10);
	strRet = buff;
	return strRet;
}

wstring LongToStr(long lVal)
{
	wstring strRet;
	TCHAR buff[20];
	_ltot_s(lVal,buff,sizeof(buff)/sizeof(buff[0]),10);
	strRet = buff;
	return strRet;
}

wstring ULongToStr(unsigned long ulVal)
{
	wstring strRet;
	TCHAR buff[20];
	_ultot_s(ulVal,buff,sizeof(buff)/sizeof(buff[0]),10);
	strRet = buff;
	return strRet;

}


wstring DblToStr(double dblVal, int ndigits)
{
    char buff[50];
    _gcvt_s(buff,sizeof(buff),dblVal, ndigits);
    return str2wstr(string(buff));
}

wstring DblToStr(float fltVal)
{
    char buff[50];
    _gcvt_s(buff,sizeof(buff),fltVal, 10);
    return str2wstr(string(buff));
}

void CADORecordset::Edit()
{
    m_nEditStatus = dbEdit;
}

BOOL CADORecordset::AddNew()
{
	m_nEditStatus = dbEditNone;
	if(m_pRecordset->AddNew() != S_OK)
		return FALSE;

	m_nEditStatus = dbEditNew;
	return TRUE;
}

BOOL CADORecordset::AddNew(CADORecordBinding &pAdoRecordBinding)
{
	try
	{
		if(m_pRecBinding->AddNew(&pAdoRecordBinding) != S_OK)
		{
			return FALSE;
		}
		else
		{
			m_pRecBinding->Update(&pAdoRecordBinding);
			return TRUE;
		}
			
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}	
}

BOOL CADORecordset::Update()
{
	BOOL bret = TRUE;

	if(m_nEditStatus != dbEditNone)
	{

		try
		{
			if(m_pRecordset->Update() != S_OK)
				bret = FALSE;
		}
		catch(_com_error &e)
		{
			dump_com_error(e);
			bret = FALSE;
		}

		if(!bret)
			m_pRecordset->CancelUpdate();
		m_nEditStatus = dbEditNone;
	}
	return bret;
}

void CADORecordset::CancelUpdate()
{
	m_pRecordset->CancelUpdate();
	m_nEditStatus = dbEditNone;
}

BOOL CADORecordset::SetFieldValue(int nIndex, wstring strValue)
{
	_variant_t vtFld;
	_variant_t vtIndex;	
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

    if(!strValue.empty())
		vtFld.vt = VT_BSTR;
	else
		vtFld.vt = VT_NULL;

    vtFld.bstrVal = _bstr_t( strValue.c_str());

	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, wstring strValue)
{
	_variant_t vtFld;

    if(!strValue.empty())
		vtFld.vt = VT_BSTR;
	else
		vtFld.vt = VT_NULL;

	vtFld.bstrVal = _bstr_t( strValue.c_str());

	BOOL bret =  PutFieldValue(lpFieldName, vtFld);
	SysFreeString(vtFld.bstrVal);
	return bret;
}

BOOL CADORecordset::SetFieldValue(int nIndex, int nValue)
{
	_variant_t vtFld;
	
	vtFld.vt = VT_I2;
	vtFld.iVal = nValue;
	
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, int nValue)
{
	_variant_t vtFld;
	
	vtFld.vt = VT_I2;
	vtFld.iVal = nValue;
	
	
	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, long lValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_I4;
	vtFld.lVal = lValue;
	
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	return PutFieldValue(vtIndex, vtFld);
	
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, long lValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_I4;
	vtFld.lVal = lValue;
	
	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, unsigned long ulValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_UI4;
	vtFld.ulVal = ulValue;
	
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	return PutFieldValue(vtIndex, vtFld);
	
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, unsigned long ulValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_UI4;
	vtFld.ulVal = ulValue;
	
	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, double dblValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_R8;
	vtFld.dblVal = dblValue;

	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, double dblValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_R8;
	vtFld.dblVal = dblValue;
		
	return PutFieldValue(lpFieldName, vtFld);
}
BOOL CADORecordset::SetFieldValue(int nIndex, SYSTEMTIME time)
{
    double          vtime;
	_variant_t      vtFld;
    SystemTimeToVariantTime(&time,&vtime);
	vtFld.vt = VT_DATE;
	vtFld.date = vtime;
	_variant_t vtIndex;
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, SYSTEMTIME time)
{
    double          vtime;
	_variant_t      vtFld;
  
    SystemTimeToVariantTime(&time,&vtime);
    vtFld.vt = VT_DATE;
	vtFld.date = vtime;
	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, bool bValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_BOOL;
	vtFld.boolVal = bValue;
	
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, bool bValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_BOOL;
	vtFld.boolVal = bValue;
	
	return PutFieldValue(lpFieldName, vtFld);
}

#if USEAFX
BOOL CADORecordset::SetFieldValue(int nIndex, COleCurrency cyValue)
{
	if(cyValue.m_status == COleCurrency::invalid)
		return FALSE;

	_variant_t vtFld;
		
	vtFld.vt = VT_CY;
	vtFld.cyVal = cyValue.m_cur;
	
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, COleCurrency cyValue)
{
	if(cyValue.m_status == COleCurrency::invalid)
		return FALSE;

	_variant_t vtFld;

	vtFld.vt = VT_CY;
	vtFld.cyVal = cyValue.m_cur;	
		
	return PutFieldValue(lpFieldName, vtFld);
}
#endif
BOOL CADORecordset::SetFieldValue(int nIndex, _variant_t vtValue)
{
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	return PutFieldValue(vtIndex, vtValue);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, _variant_t vtValue)
{	
	return PutFieldValue(lpFieldName, vtValue);
}


BOOL CADORecordset::SetBookmark()
{
	if(m_varBookmark.vt != VT_EMPTY)
	{
		m_pRecordset->Bookmark = m_varBookmark;
		return TRUE;
	}
	return FALSE;
}

BOOL CADORecordset::Delete()
{
	if(m_pRecordset->Delete(adAffectCurrent) != S_OK)
		return FALSE;

	if(m_pRecordset->Update() != S_OK)
		return FALSE;
	
	m_nEditStatus = dbEditNone;
	return TRUE;
}

BOOL CADORecordset::Find(LPCTSTR lpFind, int nSearchDirection)
{

	m_strFind = lpFind;
	m_nSearchDirection = nSearchDirection;

    ASSERT(!m_strFind.empty());

	if(m_nSearchDirection == searchForward)
	{
        m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 0, adSearchForward, _T(""));
		if(!IsEof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else if(m_nSearchDirection == searchBackward)
	{
        m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 0, adSearchBackward, _T(""));
		if(!IsBof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else
	{
		TRACE(_T("Unknown parameter. %d"), nSearchDirection);
		m_nSearchDirection = searchForward;
	}
	return FALSE;
}

BOOL CADORecordset::FindFirst(LPCTSTR lpFind)
{
	m_pRecordset->MoveFirst();
	return Find(lpFind);
}

BOOL CADORecordset::FindNext()
{
	if(m_nSearchDirection == searchForward)
	{
        m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 1, adSearchForward, m_varBookFind);
		if(!IsEof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else
	{
		m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 1, adSearchBackward, m_varBookFind);
		if(!IsBof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CADORecordset::PutFieldValue(LPCTSTR lpFieldName, _variant_t vtFld)
{
	if(m_nEditStatus == dbEditNone)
		return FALSE;
	
	try
	{
		m_pRecordset->Fields->GetItem(lpFieldName)->Value = vtFld; 
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;	
	}
}


BOOL CADORecordset::PutFieldValue(_variant_t vtIndex, _variant_t vtFld)
{
	if(m_nEditStatus == dbEditNone)
		return FALSE;

	try
	{
		m_pRecordset->Fields->GetItem(vtIndex)->Value = vtFld;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::Clone(CADORecordset &pRs)
{
	try
	{
		pRs.m_pRecordset = m_pRecordset->Clone(adLockUnspecified);
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SetFilter(LPCTSTR strFilter)
{
	ASSERT(IsOpen());
	
	try
	{
		m_pRecordset->PutFilter(strFilter);
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SetSort(LPCTSTR strCriteria)
{
	ASSERT(IsOpen());
	
	try
	{
		m_pRecordset->PutSort(strCriteria);
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SaveAsXML(LPCTSTR lpstrXMLFile)
{
	HRESULT hr;

	ASSERT(IsOpen());
	
	try
	{
		hr = m_pRecordset->Save(lpstrXMLFile, adPersistXML);
		return hr == S_OK;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
	return TRUE;
}

BOOL CADORecordset::OpenXML(LPCTSTR lpstrXMLFile)
{
	HRESULT hr = S_OK;

	if(IsOpen())
		Close();

	try
	{
		hr = m_pRecordset->Open(lpstrXMLFile, _T("Provider=MSPersist;"), adOpenForwardOnly, adLockOptimistic, adCmdFile);
		return hr == S_OK;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::Execute(CADOCommand* pAdoCommand)
{
	if(IsOpen())
		Close();

    ASSERT(!pAdoCommand->GetText().empty());
	try
	{
		m_pConnection->CursorLocation = adUseClient;
		m_pRecordset = pAdoCommand->GetCommand()->Execute(NULL, NULL, pAdoCommand->GetType());
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

void CADORecordset::dump_com_error(_com_error &e)
{
    TCHAR ErrorStr[1024];
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
    swprintf(ErrorStr,sizeof(ErrorStr)/sizeof(ErrorStr[0]),
        _T("CADORecordset Error\r\nCode = %08lx\r\nCode meaning = %s\r\nSource = %s\r\nDescription = %s\r\n"),
        e.Error(),
        e.ErrorMessage(),
        (LPCTSTR)bstrSource,
        (LPCTSTR)bstrDescription );
    m_strLastError  = _T("Query = ") + GetQuery() + _T('\r\n') + ErrorStr;
    m_dwLastError   = e.Error();
}


///////////////////////////////////////////////////////
//
// CADOCommad Class
//

CADOCommand::CADOCommand(CADODatabase* pAdoDatabase, wstring strCommandText, int nCommandType)
{
    m_pCommand              = NULL;
    m_pCommand.CreateInstance(__uuidof(Command));
    m_strCommandText        = strCommandText;
    m_pCommand->CommandText = _bstr_t(m_strCommandText.c_str());
    m_nCommandType          = nCommandType;
    m_pCommand->CommandType = (CommandTypeEnum)m_nCommandType;
    m_pCommand->ActiveConnection    = pAdoDatabase->GetActiveConnection();	
    m_nRecordsAffected      = 0;
}

BOOL CADOCommand::AddParameter(CADOParameter* pAdoParameter)
{
	ASSERT(pAdoParameter->GetParameter() != NULL);

	try
	{
		m_pCommand->Parameters->Append(pAdoParameter->GetParameter());
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOCommand::AddParameter(wstring strName, int nType, int nDirection, long lSize, int nValue)
{

	_variant_t vtValue;

	vtValue.vt = VT_I2;
	vtValue.iVal = nValue;

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

BOOL CADOCommand::AddParameter(wstring strName, int nType, int nDirection, long lSize, long lValue)
{

	_variant_t vtValue;

	vtValue.vt = VT_I4;
	vtValue.lVal = lValue;

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

BOOL CADOCommand::AddParameter(wstring strName, int nType, int nDirection, long lSize, double dblValue, int nPrecision, int nScale)
{

	_variant_t vtValue;

	vtValue.vt = VT_R8;
	vtValue.dblVal = dblValue;

	return AddParameter(strName, nType, nDirection, lSize, vtValue, nPrecision, nScale);
}

BOOL CADOCommand::AddParameter(wstring strName, int nType, int nDirection, long lSize, wstring strValue)
{

	_variant_t vtValue;

	vtValue.vt = VT_BSTR;
	vtValue.bstrVal = _bstr_t(strValue.c_str());

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}
#if USEAFX

BOOL CADOCommand::AddParameter(wstring strName, int nType, int nDirection, long lSize, COleDateTime time)
{

	_variant_t vtValue;

	vtValue.vt = VT_DATE;
	vtValue.date = time;

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}
#endif

BOOL CADOCommand::AddParameter(wstring strName, int nType, int nDirection, long lSize, _variant_t vtValue, int nPrecision, int nScale)
{
	try
	{
		_ParameterPtr pParam = m_pCommand->CreateParameter(_bstr_t(strName.c_str()), (DataTypeEnum)nType, (ParameterDirectionEnum)nDirection, lSize, vtValue);
		pParam->PutPrecision(nPrecision);
		pParam->PutNumericScale(nScale);
		m_pCommand->Parameters->Append(pParam);
		
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


void CADOCommand::SetText(wstring strCommandText)
{
    ASSERT(!strCommandText.empty());

	m_strCommandText = strCommandText;

	SysFreeString(m_pCommand->CommandText);
	
	m_pCommand->CommandText = _bstr_t(m_strCommandText.c_str());
}
void CADOCommand::SetType(int nCommandType)
{
	m_nCommandType = nCommandType;
	m_pCommand->CommandType = (CommandTypeEnum)m_nCommandType;
}

BOOL CADOCommand::Execute(int nCommandType)
{
	_variant_t vRecords;
	m_nRecordsAffected = 0;
	try
	{
		m_nCommandType = nCommandType;
		m_pCommand->Execute(&vRecords, NULL, nCommandType);
		m_nRecordsAffected = vRecords.iVal;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

void CADOCommand::dump_com_error(_com_error &e)
{
    TCHAR   ErrorStr[1024];
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
    swprintf(ErrorStr,sizeof(ErrorStr)/sizeof(ErrorStr[0]),
        _T("cmADOCommand Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
        e.Error(),
        e.ErrorMessage(),
        (LPCTSTR)bstrSource,
        (LPCTSTR)bstrDescription);
    m_strLastError  = ErrorStr;
    m_dwLastError   = e.Error();
    /*throw CADOException(e.Error(), e.Description()); **/
}


///////////////////////////////////////////////////////
//
// CADOParameter Class
//

CADOParameter::CADOParameter(int nType, long lSize, int nDirection, wstring strName)
{
	m_pParameter = NULL;
	m_pParameter.CreateInstance(__uuidof(Parameter));
	m_strName = _T("");
	m_pParameter->Direction = (ParameterDirectionEnum)nDirection;
	m_strName = strName;
    m_pParameter->Name = _bstr_t(m_strName.c_str());
	m_pParameter->Type = (DataTypeEnum)nType;
	m_pParameter->Size = lSize;
	m_nType = nType;
}

BOOL CADOParameter::SetValue(int nValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);
	
	vtVal.vt = VT_I2;
	vtVal.iVal = nValue;

	try
	{
		if(m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(int);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADOParameter::SetValue(long lValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);
	
	vtVal.vt = VT_I4;
	vtVal.lVal = lValue;

	try
	{
		if(m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(long);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::SetValue(double dblValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);
	
	vtVal.vt = VT_R8;
	vtVal.dblVal = dblValue;

	try
	{
		if(m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(double);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::SetValue(wstring strValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);
	
    if(!strValue.empty())
		vtVal.vt = VT_BSTR;
	else
		vtVal.vt = VT_NULL;

    vtVal.bstrVal = _bstr_t(strValue.c_str());

	try
	{
		if(m_pParameter->Size == 0)
            m_pParameter->Size = sizeof(char) * strValue.length();

		m_pParameter->Value = vtVal;
		::SysFreeString(vtVal.bstrVal);
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		::SysFreeString(vtVal.bstrVal);
		return FALSE;
	}
}
#if USEAFX
BOOL CADOParameter::SetValue(COleDateTime time)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);
	
	vtVal.vt = VT_DATE;
	vtVal.date = time;

	try
	{
		if(m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(DATE);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
#endif
BOOL CADOParameter::SetValue(_variant_t vtValue)
{

	ASSERT(m_pParameter != NULL);

	try
	{
		if(m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(VARIANT);
		
		m_pParameter->Value = vtValue;
		return TRUE;
	}
	catch(_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(int& nValue)
{
	_variant_t vtVal;
	int nVal = 0;

	try
	{
		vtVal = m_pParameter->Value;

		switch(vtVal.vt)
		{
		case VT_BOOL:
			nVal = vtVal.boolVal;
			break;
		case VT_I2:
		case VT_UI1:
			nVal = vtVal.iVal;
			break;
		case VT_INT:
			nVal = vtVal.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			nVal = 0;
			break;
		default:
			nVal = vtVal.iVal;
		}	
		nValue = nVal;
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(long& lValue)
{
	_variant_t vtVal;
	long lVal = 0;

	try
	{
		vtVal = m_pParameter->Value;
		if(vtVal.vt != VT_NULL && vtVal.vt != VT_EMPTY)
			lVal = vtVal.lVal;
		lValue = lVal;
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(double& dbValue)
{
	_variant_t vtVal;
	double dblVal;
	try
	{
		vtVal = m_pParameter->Value;
		switch(vtVal.vt)
		{
		case VT_R4:
			dblVal = vtVal.fltVal;
			break;
		case VT_R8:
			dblVal = vtVal.dblVal;
			break;
		case VT_DECIMAL:
			dblVal = vtVal.decVal.Lo32;
			dblVal *= (vtVal.decVal.sign == 128)? -1 : 1;
			dblVal /= pow((float)10, (int)vtVal.decVal.scale); 
			break;
		case VT_UI1:
			dblVal = vtVal.iVal;
			break;
		case VT_I2:
		case VT_I4:
			dblVal = vtVal.lVal;
			break;
		case VT_INT:
			dblVal = vtVal.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			dblVal = 0;
			break;
		default:
			dblVal = 0;
		}
		dbValue = dblVal;
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(wstring& strValue, wstring strDateFormat)
{
	_variant_t vtVal;
	wstring strVal = _T("");

	try
	{
		vtVal = m_pParameter->Value;
		switch(vtVal.vt) 
		{
		case VT_R4:
			strVal = DblToStr(vtVal.fltVal);
			break;
		case VT_R8:
			strVal = DblToStr(vtVal.dblVal);
			break;
		case VT_BSTR:
			strVal = vtVal.bstrVal;
			break;
		case VT_I2:
		case VT_UI1:
			strVal = IntToStr(vtVal.iVal);
			break;
		case VT_INT:
			strVal = IntToStr(vtVal.intVal);
			break;
		case VT_I4:
			strVal = LongToStr(vtVal.lVal);
			break;
		case VT_DECIMAL:
			{
			double val = vtVal.decVal.Lo32;
			val *= (vtVal.decVal.sign == 128)? -1 : 1;
			val /= pow((float)10, vtVal.decVal.scale); 
			strVal = DblToStr(val);
			}
			break;
		case VT_DATE:
			{
#if USEAFX
				COleDateTime dt(vtVal);

                if(strDateFormat.empty())
					strDateFormat = _T("%Y-%m-%d %H:%M:%S");
                strVal = dt.Format(strDateFormat.c_str());
#endif
			}
			break;
		case VT_EMPTY:
		case VT_NULL:
			strVal.empty();
			break;
		default:
            strVal.empty();
			return FALSE;
		}
		strValue = strVal;
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
#if USEAFX
BOOL CADOParameter::GetValue(COleDateTime& time)
{
	_variant_t vtVal;

	try
	{
		vtVal = m_pParameter->Value;
		switch(vtVal.vt) 
		{
		case VT_DATE:
			{
				COleDateTime dt(vtVal);
				time = dt;
			}
			break;
		case VT_EMPTY:
		case VT_NULL:
			time.SetStatus(COleDateTime::null);
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}
#endif
BOOL CADOParameter::GetValue(_variant_t& vtValue)
{
	try
	{
		vtValue = m_pParameter->Value;
		return TRUE;
	}
	catch(_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


void CADOParameter::dump_com_error(_com_error &e)
{
    TCHAR ErrorStr[1024];
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
    swprintf(ErrorStr,sizeof(ErrorStr)/sizeof(ErrorStr[0]),
        _T("cmADOParameter Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
        e.Error(), 
        e.ErrorMessage(), 
        (LPCTSTR)bstrSource,
        (LPCTSTR)bstrDescription );
    m_strLastError = ErrorStr;
    m_dwLastError = e.Error();
    throw CADOException(e.Error(), e.Description());
}






const struct sTypeTostr 
{
    ADODB::DataTypeEnum type;
    TCHAR        *str;
} TypeTostr[] = {
        {ADODB::adEmpty,TEXT("Empty")},
        {ADODB::adTinyInt,TEXT("TinyInt")},
        {ADODB::adSmallInt,TEXT("SmallInt")},
        {ADODB::adInteger,TEXT("Integer")},
        {ADODB::adBigInt,TEXT("BigInt")},
        {ADODB::adUnsignedTinyInt,TEXT("UnsignedTinyInt")},
        {ADODB::adUnsignedSmallInt,TEXT("UnsignedSmallInt")},
        {ADODB::adUnsignedInt,TEXT("UnsignedInt")},
        {ADODB::adUnsignedBigInt,TEXT("UnsignedBigInt")},
        {ADODB::adSingle,TEXT("Single")},
        {ADODB::adDouble,TEXT("Double")},
        {ADODB::adCurrency,TEXT("Currency")},
        {ADODB::adDecimal,TEXT("Decimal")},
        {ADODB::adNumeric,TEXT("Numeric")},
        {ADODB::adBoolean,TEXT("Boolean")},
        {ADODB::adError,TEXT("Error")},
        {ADODB::adUserDefined,TEXT("UserDefined")},
        {ADODB::adVariant,TEXT("Variant")},
        {ADODB::adIDispatch,TEXT("IDispatch")},
        {ADODB::adIUnknown,TEXT("IUnknown")},
        {ADODB::adGUID,TEXT("GUID")},
        {ADODB::adDate,TEXT("Date")},
        {ADODB::adDBDate,TEXT("DBDate")},
        {ADODB::adDBTime,TEXT("DBTime")},
        {ADODB::adDBTimeStamp,TEXT("DBTimeStamp")},
        {ADODB::adBSTR,TEXT("BSTR")},
        {ADODB::adChar,TEXT("Char")},
        {ADODB::adVarChar,TEXT("VarChar")},
        {ADODB::adLongVarChar,TEXT("LongVarChar")},
        {ADODB::adWChar,TEXT("WChar")},
        {ADODB::adVarWChar,TEXT("VarWChar")},
        {ADODB::adLongVarWChar,TEXT("LongVarWChar")},
        {ADODB::adBinary,TEXT("Binary")},
        {ADODB::adVarBinary,TEXT("VarBinary")},
        {ADODB::adLongVarBinary,TEXT("LongVarBinary")},
        {ADODB::adChapter,TEXT("Chapter")},
        {ADODB::adFileTime,TEXT("FileTime")},
        {ADODB::adPropVariant,TEXT("PropVariant")},
        {ADODB::adVarNumeric,TEXT("VarNumeric")},
        {ADODB::adVariant,TEXT("Variant")},
};
       
wstring adoTypeTostr(short type) 
{
    for(int i = 0; i < sizeof(TypeTostr)/sizeof(TypeTostr[0]);i ++){
        if(TypeTostr[i].type == type ) {
            return TypeTostr[i].str;
        }
    }
    return TEXT("Error");
}

