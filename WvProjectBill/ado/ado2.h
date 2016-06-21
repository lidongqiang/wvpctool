#ifndef CM_ADO_H
#define CM_ADO_H
#pragma warning (disable: 4146)
#import "C:\Program Files\Common Files\System\ado\msado15.dll" rename("EOF", "EndOfFile")         
#include "icrsint.h"
#include "string"
#include "math.h"
#if 0
#define USEAFX
#ifdef 
#include ""
#endif
#endif
using namespace ADODB;
using namespace std;

wstring IntToStr(int nVal);
wstring LongToStr(long lVal);
wstring ULongToStr(unsigned long ulVal);
wstring DblToStr(double dblVal, int ndigits = 20);
wstring DblToStr(float fltVal);

class CADOCommand;

struct CADOFieldInfo
{
    TCHAR   m_strName[128]; 
    short   m_nType;
    long    m_lSize; 
    long    m_lDefinedSize;
    long    m_lAttributes;
    short   m_nOrdinalPosition;
    BOOL    m_bRequired;   
    BOOL    m_bAllowZeroLength; 
    long    m_lCollatingOrder;  
};

class CADODatabase
{
public:
    enum cadoConnectModeEnum
    {	
        connectModeUnknown = adModeUnknown,
        connectModeRead = adModeRead,
        connectModeWrite = adModeWrite,
        connectModeReadWrite = adModeReadWrite,
        connectModeShareDenyRead = adModeShareDenyRead,
        connectModeShareDenyWrite = adModeShareDenyWrite,
        connectModeShareExclusive = adModeShareExclusive,
        connectModeShareDenyNone = adModeShareDenyNone
    };

    CADODatabase()
    {
        ::CoInitialize(NULL);
        m_pConnection   = NULL;
        m_strConnection = _T("");
        m_strLastError  = _T("");
        m_dwLastError   = 0;
        m_pConnection.CreateInstance(__uuidof(Connection));
        m_nRecordsAffected = 0;
        m_nConnectionTimeout = 0;
    }

    virtual ~CADODatabase()
    {
        Close();
        m_pConnection.Release();
        m_pConnection = NULL;
        m_strConnection = _T("");
        m_strLastError = _T("");
        m_dwLastError = 0;
        ::CoUninitialize();
    }
	
	BOOL Open(LPCTSTR lpstrConnection = _T(""), LPCTSTR lpstrUserID = _T(""), LPCTSTR lpstrPassword = _T(""));
	_ConnectionPtr GetActiveConnection() 
    {
        return m_pConnection;
    };
	BOOL Execute(LPCTSTR lpstrExec);
	int GetRecordsAffected()
    {
        return m_nRecordsAffected;
    };
	DWORD GetRecordCount(_RecordsetPtr m_pRs);
	long BeginTransaction() 
    {
        return m_pConnection->BeginTrans();
    };
	long CommitTransaction() 
    {
        return m_pConnection->CommitTrans();
    };
	long RollbackTransaction() 
    {
        return m_pConnection->RollbackTrans();
    };
	BOOL IsOpen();
	void Close();
	void SetConnectionMode(cadoConnectModeEnum nMode)
    {
        m_pConnection->PutMode((enum ConnectModeEnum)nMode);
    };
	void SetConnectionString(LPCTSTR lpstrConnection)
    {
        m_strConnection = lpstrConnection;
    };
	wstring GetConnectionString()
    {
        return m_strConnection;
    };
	wstring GetLastErrorString() 
    {
        return m_strLastError;
    };
	DWORD GetLastError()
    {
        return m_dwLastError;
    };
	wstring GetErrorDescription() 
    {
        return m_strErrorDescription;
    };
	void SetConnectionTimeout(long nConnectionTimeout = 30)
    {
        m_nConnectionTimeout = nConnectionTimeout;
    };

protected:
    void dump_com_error(_com_error &e);

public:
    _ConnectionPtr m_pConnection;
	
protected:
    wstring m_strConnection;
    wstring m_strLastError;
    wstring m_strErrorDescription;
    DWORD   m_dwLastError;
    int     m_nRecordsAffected;
    long    m_nConnectionTimeout;
};

class CADORecordset
{
public:
	BOOL Clone(CADORecordset& pRs);
	
	enum cadoOpenEnum
	{
		openUnknown = 0,
		openQuery = 1,
		openTable = 2,
		openStoredProc = 3
	};

	enum cadoEditEnum
	{
		dbEditNone = 0,
		dbEditNew = 1,
		dbEdit = 2
	};
	
	enum cadoPositionEnum
	{
	
		positionUnknown = -1,
		positionBOF = -2,
		positionEOF = -3
	};
	
	enum cadoSearchEnum
	{	
		searchForward = 1,
		searchBackward = -1
	};

	enum cadoDataType
	{
		typeEmpty = ADODB::adEmpty,
		typeTinyInt = ADODB::adTinyInt,
		typeSmallInt = ADODB::adSmallInt,
		typeInteger = ADODB::adInteger,
		typeBigInt = ADODB::adBigInt,
		typeUnsignedTinyInt = ADODB::adUnsignedTinyInt,
		typeUnsignedSmallInt = ADODB::adUnsignedSmallInt,
		typeUnsignedInt = ADODB::adUnsignedInt,
		typeUnsignedBigInt = ADODB::adUnsignedBigInt,
		typeSingle = ADODB::adSingle,
		typeDouble = ADODB::adDouble,
		typeCurrency = ADODB::adCurrency,
		typeDecimal = ADODB::adDecimal,
		typeNumeric = ADODB::adNumeric,
		typeBoolean = ADODB::adBoolean,
		typeError = ADODB::adError,
		typeUserDefined = ADODB::adUserDefined,
		typeVariant = ADODB::adVariant,
		typeIDispatch = ADODB::adIDispatch,
		typeIUnknown = ADODB::adIUnknown,
		typeGUID = ADODB::adGUID,
		typeDate = ADODB::adDate,
		typeDBDate = ADODB::adDBDate,
		typeDBTime = ADODB::adDBTime,
		typeDBTimeStamp = ADODB::adDBTimeStamp,
		typeBSTR = ADODB::adBSTR,
		typeChar = ADODB::adChar,
		typeVarChar = ADODB::adVarChar,
		typeLongVarChar = ADODB::adLongVarChar,
		typeWChar = ADODB::adWChar,
		typeVarWChar = ADODB::adVarWChar,
		typeLongVarWChar = ADODB::adLongVarWChar,
		typeBinary = ADODB::adBinary,
		typeVarBinary = ADODB::adVarBinary,
		typeLongVarBinary = ADODB::adLongVarBinary,
		typeChapter = ADODB::adChapter,
		typeFileTime = ADODB::adFileTime,
		typePropVariant = ADODB::adPropVariant,
		typeVarNumeric = ADODB::adVarNumeric,
		typeArray = ADODB::adVariant
	};
	
	enum cadoSchemaType 
	{
		schemaSpecific = adSchemaProviderSpecific,	
		schemaAsserts = adSchemaAsserts,
		schemaCatalog = adSchemaCatalogs,
		schemaCharacterSet = adSchemaCharacterSets,
		schemaCollections = adSchemaCollations,
		schemaColumns = adSchemaColumns,
		schemaConstraints = adSchemaCheckConstraints,
		schemaConstraintColumnUsage = adSchemaConstraintColumnUsage,
		schemaConstraintTableUsage  = adSchemaConstraintTableUsage,
		shemaKeyColumnUsage = adSchemaKeyColumnUsage,
		schemaTableConstraints = adSchemaTableConstraints,
		schemaColumnsDomainUsage = adSchemaColumnsDomainUsage,
		schemaIndexes = adSchemaIndexes,
		schemaColumnPrivileges = adSchemaColumnPrivileges,
		schemaTablePrivileges = adSchemaTablePrivileges,
		schemaUsagePrivileges = adSchemaUsagePrivileges,
		schemaProcedures = adSchemaProcedures,
		schemaTables = adSchemaTables,
		schemaProviderTypes = adSchemaProviderTypes,
		schemaViews = adSchemaViews,
		schemaViewTableUsage = adSchemaViewTableUsage,
		schemaProcedureParameters = adSchemaProcedureParameters,
		schemaForeignKeys = adSchemaForeignKeys,
		schemaPrimaryKeys = adSchemaPrimaryKeys,
		schemaProcedureColumns = adSchemaProcedureColumns,
		schemaDBInfoKeywords = adSchemaDBInfoKeywords,
		schemaDBInfoLiterals = adSchemaDBInfoLiterals,
		schemaCubes = adSchemaCubes,
		schemaDimensions = adSchemaDimensions,
		schemaHierarchies  = adSchemaHierarchies, 
		schemaLevels = adSchemaLevels,
		schemaMeasures = adSchemaMeasures,
		schemaProperties = adSchemaProperties,
		schemaMembers = adSchemaMembers,
	}; 


	BOOL SetFieldValue(int nIndex, int nValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, int nValue);
	BOOL SetFieldValue(int nIndex, long lValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, long lValue);
	BOOL SetFieldValue(int nIndex, unsigned long lValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, unsigned long lValue);
	BOOL SetFieldValue(int nIndex, double dblValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, double dblValue);
	BOOL SetFieldValue(int nIndex, wstring strValue);
    BOOL SetFieldValue(LPCTSTR lpFieldName, string strValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, wstring strValue);
    BOOL SetFieldValue(int nIndex           , SYSTEMTIME time);
    BOOL SetFieldValue(LPCTSTR lpFieldName  , SYSTEMTIME time);
	BOOL SetFieldValue(int nIndex, bool bValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, bool bValue);
#if USEAFX
	BOOL SetFieldValue(int nIndex, COleCurrency cyValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, COleCurrency cyValue);
#endif
	BOOL SetFieldValue(int nIndex, _variant_t vtValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, _variant_t vtValue);

	BOOL SetFieldEmpty(int nIndex);
	BOOL SetFieldEmpty(LPCTSTR lpFieldName);

	void CancelUpdate();
	BOOL Update();
	void Edit();
	BOOL AddNew();
	BOOL AddNew(CADORecordBinding &pAdoRecordBinding);

	BOOL Find(LPCTSTR lpFind, int nSearchDirection = CADORecordset::searchForward);
	BOOL FindFirst(LPCTSTR lpFind);
	BOOL FindNext();

	CADORecordset();

	CADORecordset(CADODatabase* pAdoDatabase);
    void SetCADODatabase(CADODatabase* pAdoDatabase) {
        m_pConnection = pAdoDatabase->GetActiveConnection();
    }

	virtual ~CADORecordset()
	{
		Close();
		if(m_pRecordset)
			m_pRecordset.Release();
		if(m_pCmd)
			m_pCmd.Release();
		m_pRecordset = NULL;
		m_pCmd = NULL;
		m_pRecBinding = NULL;
		m_strQuery = _T("");
		m_strLastError = _T("");
		m_dwLastError = 0;
		m_nEditStatus = dbEditNone;
		m_pConnection = NULL;
	}

	wstring GetQuery() 
		{return m_strQuery;};
	void SetQuery(LPCTSTR strQuery) 
		{m_strQuery = strQuery;};
	BOOL RecordBinding(CADORecordBinding &pAdoRecordBinding);
	DWORD GetRecordCount();
	BOOL IsOpen();
	void Close();
	BOOL Open(_ConnectionPtr mpdb, LPCTSTR lpstrExec = _T(""), int nOption = CADORecordset::openUnknown);
	BOOL Open(LPCTSTR lpstrExec = _T(""), int nOption = CADORecordset::openUnknown);
	BOOL OpenSchema(int nSchema, LPCTSTR SchemaID = _T(""));
	long GetFieldCount()
		{return m_pRecordset->Fields->GetCount();};
	BOOL GetFieldValue(LPCTSTR lpFieldName, int& nValue);
    BOOL GetFieldValue(LPCTSTR lpFieldName, unsigned char& ulValue);
	BOOL GetFieldValue(int nIndex, int& nValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, long& lValue);
	BOOL GetFieldValue(int nIndex, long& lValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, unsigned long& ulValue);
	BOOL GetFieldValue(int nIndex, unsigned long& ulValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, double& dbValue);
	BOOL GetFieldValue(int nIndex, double& dbValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, wstring& strValue, wstring strDateFormat = _T(""));
	BOOL GetFieldValue(int nIndex, wstring& strValue, wstring strDateFormat = _T(""));

	BOOL GetFieldValue(LPCTSTR lpFieldName  , SYSTEMTIME& time);
	BOOL GetFieldValue(int nIndex           , SYSTEMTIME& time);

	BOOL GetFieldValue(int nIndex, bool& bValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, bool& bValue);
#if USEAFX
	BOOL GetFieldValue(int nIndex, COleCurrency& cyValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, COleCurrency& cyValue);
#endif
	BOOL GetFieldValue(int nIndex, _variant_t& vtValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, _variant_t& vtValue);
	
	BOOL IsFieldNull(LPCTSTR lpFieldName);
	BOOL IsFieldNull(int nIndex);
	BOOL IsFieldEmpty(LPCTSTR lpFieldName);
	BOOL IsFieldEmpty(int nIndex);	
	BOOL IsEof()
		{return m_pRecordset->EndOfFile == VARIANT_TRUE;};
	BOOL IsEOF()
		{return m_pRecordset->EndOfFile == VARIANT_TRUE;};
	BOOL IsBof()
		{return m_pRecordset->BOF == VARIANT_TRUE;};
	BOOL IsBOF()
		{return m_pRecordset->BOF == VARIANT_TRUE;};
	void MoveFirst() 
		{m_pRecordset->MoveFirst();};
	void MoveNext() 
		{m_pRecordset->MoveNext();};
	void MovePrevious() 
		{m_pRecordset->MovePrevious();};
	void MoveLast() 
		{m_pRecordset->MoveLast();};
	long GetAbsolutePage()
		{return m_pRecordset->GetAbsolutePage();};
	void SetAbsolutePage(int nPage)
		{m_pRecordset->PutAbsolutePage((enum PositionEnum)nPage);};
	long GetPageCount()
		{return m_pRecordset->GetPageCount();};
	long GetPageSize()
		{return m_pRecordset->GetPageSize();};
	void SetPageSize(int nSize)
		{m_pRecordset->PutPageSize(nSize);};
	long GetAbsolutePosition()
		{return m_pRecordset->GetAbsolutePosition();};
	void SetAbsolutePosition(int nPosition)
		{m_pRecordset->PutAbsolutePosition((enum PositionEnum)nPosition);};
	BOOL GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo* fldInfo);
	BOOL GetFieldInfo(int nIndex, CADOFieldInfo* fldInfo);
	BOOL AppendChunk(LPCTSTR lpFieldName, LPVOID lpData, UINT nBytes);
	BOOL AppendChunk(int nIndex, LPVOID lpData, UINT nBytes);

	BOOL GetChunk(LPCTSTR lpFieldName, wstring& strValue);
	BOOL GetChunk(int nIndex, wstring& strValue);
	
	BOOL GetChunk(LPCTSTR lpFieldName, LPVOID pData);
	BOOL GetChunk(int nIndex, LPVOID pData);

	wstring GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows = 0);
	wstring GetLastErrorString() 
		{return m_strLastError;};
	DWORD GetLastError()
		{return m_dwLastError;};
	void GetBookmark()
		{m_varBookmark = m_pRecordset->Bookmark;};
	BOOL SetBookmark();
	BOOL Delete();
	BOOL IsConnectionOpen()
		{return m_pConnection != NULL && m_pConnection->GetState() != adStateClosed;};
	_RecordsetPtr GetRecordset()
		{return m_pRecordset;};
	_ConnectionPtr GetActiveConnection() 
		{return m_pConnection;};
	void SetActiveConnection(_ConnectionPtr pConn)
		{m_pConnection = pConn;}
	BOOL SetFilter(LPCTSTR strFilter);
	BOOL SetSort(LPCTSTR lpstrCriteria);
	BOOL SaveAsXML(LPCTSTR lpstrXMLFile);
	BOOL OpenXML(LPCTSTR lpstrXMLFile);
	BOOL Execute(CADOCommand* pCommand);
	BOOL Requery();

public:
    _RecordsetPtr   m_pRecordset;
    _CommandPtr     m_pCmd;
	
protected:
    _ConnectionPtr  m_pConnection;
    int m_nSearchDirection;
	wstring m_strFind;
	_variant_t m_varBookFind;
	_variant_t m_varBookmark;
	int m_nEditStatus;
	wstring m_strLastError;
	DWORD m_dwLastError;
	void dump_com_error(_com_error &e);
	IADORecordBinding *m_pRecBinding;
	wstring m_strQuery;

protected:
	BOOL PutFieldValue(LPCTSTR lpFieldName, _variant_t vtFld);
	BOOL PutFieldValue(_variant_t vtIndex, _variant_t vtFld);
	BOOL GetFieldInfo(FieldPtr pField, CADOFieldInfo* fldInfo);
	BOOL GetChunk(FieldPtr pField, wstring& strValue);
	BOOL GetChunk(FieldPtr pField, LPVOID lpData);
	BOOL AppendChunk(FieldPtr pField, LPVOID lpData, UINT nBytes);
		
};

class CADOParameter
{
public:

	enum cadoParameterDirection
	{
		paramUnknown = adParamUnknown,
		paramInput = adParamInput,
		paramOutput = adParamOutput,
		paramInputOutput = adParamInputOutput,
		paramReturnValue = adParamReturnValue 
	};

	CADOParameter(int nType, long lSize = 0, int nDirection = paramInput, wstring strName = _T(""));
	
	virtual ~CADOParameter()
	{
		m_pParameter.Release();
		m_pParameter = NULL;
		m_strName = _T("");
	}

	BOOL SetValue(int nValue);
	BOOL SetValue(long lValue);
	BOOL SetValue(double dbValue);
	BOOL SetValue(wstring strValue);
#if USEAFX
	BOOL SetValue(COleDateTime time);
#endif
	BOOL SetValue(_variant_t vtValue);
	BOOL GetValue(int& nValue);
	BOOL GetValue(long& lValue);
	BOOL GetValue(double& dbValue);
	BOOL GetValue(wstring& strValue, wstring strDateFormat = _T(""));
#if USEAFX
	BOOL GetValue(COleDateTime& time);
#endif
	BOOL GetValue(_variant_t& vtValue);
	void SetPrecision(int nPrecision)
		{m_pParameter->PutPrecision(nPrecision);};
	void SetScale(int nScale)
		{m_pParameter->PutNumericScale(nScale);};

	void SetName(wstring strName)
		{m_strName = strName;};
	wstring GetName()
		{return m_strName;};
	int GetType()
		{return m_nType;};
	_ParameterPtr GetParameter()
		{return m_pParameter;};

protected:
	void dump_com_error(_com_error &e);
	
protected:
	_ParameterPtr m_pParameter;
	wstring m_strName;
	int m_nType;
	wstring m_strLastError;
	DWORD m_dwLastError;
};

class CADOCommand
{
public:
	enum cadoCommandType
	{
		typeCmdText = adCmdText,
		typeCmdTable = adCmdTable,
		typeCmdTableDirect = adCmdTableDirect,
		typeCmdStoredProc = adCmdStoredProc,
		typeCmdUnknown = adCmdUnknown,
		typeCmdFile = adCmdFile
	};
	
	CADOCommand(CADODatabase* pAdoDatabase, wstring strCommandText = _T(""), int nCommandType = typeCmdStoredProc);
		
	virtual ~CADOCommand()
	{
		SysFreeString(m_pCommand->CommandText);
		m_pCommand.Release();
		m_pCommand = NULL;
		m_strCommandText = _T("");
	}

	void SetTimeout(long nTimeOut)
		{m_pCommand->PutCommandTimeout(nTimeOut);};
	void SetText(wstring strCommandText);
	void SetType(int nCommandType);
	int GetType()
		{return m_nCommandType;};
	BOOL AddParameter(CADOParameter* pAdoParameter);
	BOOL AddParameter(wstring strName, int nType, int nDirection, long lSize, int nValue);
	BOOL AddParameter(wstring strName, int nType, int nDirection, long lSize, long lValue);
	BOOL AddParameter(wstring strName, int nType, int nDirection, long lSize, double dblValue, int nPrecision = 0, int nScale = 0);
	BOOL AddParameter(wstring strName, int nType, int nDirection, long lSize, wstring strValue);
#if USEAFX
	BOOL AddParameter(wstring strName, int nType, int nDirection, long lSize, COleDateTime time);
#endif
	BOOL AddParameter(wstring strName, int nType, int nDirection, long lSize, _variant_t vtValue, int nPrecision = 0, int nScale = 0);
	wstring GetText()
		{return m_strCommandText;};
	BOOL Execute(int nCommandType = typeCmdStoredProc);
	int GetRecordsAffected()
		{return m_nRecordsAffected;};
	_CommandPtr GetCommand()
		{return m_pCommand;};

protected:
	void dump_com_error(_com_error &e);

protected:
	_CommandPtr m_pCommand;
	int m_nCommandType;
	int m_nRecordsAffected;
	wstring m_strCommandText;
	wstring m_strLastError;
	DWORD m_dwLastError;
};

/////////////////////////////////////////////////////////////////////
//
//		CADOException Class
//

class CADOException
{
public:
	CADOException() :
		m_lErrorCode(0),
		m_strError(_T(""))
		{
		}

	CADOException(long lErrorCode) :
		m_lErrorCode(lErrorCode),
		m_strError(_T(""))
		{
		}

	CADOException(long lErrorCode, const wstring& strError) :
		m_lErrorCode(lErrorCode),
		m_strError(strError)
		{
		}

	CADOException(const wstring& strError) :
		m_lErrorCode(0),
		m_strError(strError)
		{
		}

	CADOException(long lErrorCode, const TCHAR* szError) :
		m_lErrorCode(lErrorCode),
		m_strError(szError)
		{
		}

	CADOException(const TCHAR* szError) :
		m_lErrorCode(0),
		m_strError(szError)
		{
		}

	virtual ~CADOException()
		{
		}

	wstring GetErrorMessage() const
		{return m_strError;};
	void SetErrorMessage(LPCTSTR lpstrError = _T(""))
		{m_strError = lpstrError;};
	long GetError()
		{return m_lErrorCode;};
	void SetError(long lErrorCode = 0)
		{m_lErrorCode = lErrorCode;};

protected:
	wstring m_strError;
	long m_lErrorCode;
};
wstring adoTypeTostr(short type);
#endif
