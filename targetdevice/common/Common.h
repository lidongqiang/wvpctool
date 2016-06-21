#pragma once

#include <string>

#define CHECK_TRUE(a,b) {CString ss = a;ss.MakeLower();b = (ss == _T("true"));}
#define GET_TRUE_FALSE(a) (a ? _T("True") : _T("False"))

//Const string definitions
#define TOOL_CONFIG_FILE		_T("\\ToolConfig.xml")

class CCommon
{
public:
	static CString GetToolDir();
	static CString GetToolConfigFile();
	static CString* ExtractStr(LPCTSTR source, LPCTSTR seprator, int *count);
	static void FreeStrings(CString *lpstr, int count);
	static void Copy2WideChar(CString &strDest, char* pSource,int nLen = -1);
	static CString ToWideChar(const char* pSource, int nLen = -1);
	static void Copy2MultiByte(CString strSource, char* pOut,int nBufferLen);
	static CString GetExceptionErrorMessage(CException *pException);
	static bool BrowseDir(CString &strDir, CString strTitle, int nImgID, CString strDefaultDir = _T("C:\\"));
	static CString GetSystemErrorMessage();
	static CString GetSystemErrorMessage(DWORD deErrCode);
	static bool GetGPIBVersion(CString &strGPIBVersion);
	static bool GetVISAVersion(CString &strVISAVersion);
	static void GetSupportedTester(CArray<CString> &ts);
	static bool IsIntegerOrFloat(CString str);
	static CString GetSystemTimeString();
	static CString FormatString(LPCTSTR pstrFormat, ...);
	static std::string ToUTF8(CString strDir);
	static std::string CStringToSTLstring(CString str);

private:
	static LPITEMIDLIST GetDirItemIDLIST(CString strDir);
	static void FreeITEMIDLIST(LPITEMIDLIST pidl);
};
