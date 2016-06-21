#include "stdafx.h"
#include "Common.h"
#include <VersionHelpers.h>

CString CCommon::GetToolDir()
{
	static CString strToolDir;

	if(strToolDir.IsEmpty())
	{
		TCHAR sBuffer[MAX_PATH];
		GetModuleFileName(NULL,sBuffer,MAX_PATH);
		PathRemoveFileSpec(sBuffer);

		strToolDir = sBuffer;
	}

	return strToolDir;
}

CString CCommon::GetToolConfigFile()
{
	return GetToolDir() + TOOL_CONFIG_FILE;
}

CString* CCommon::ExtractStr(LPCTSTR source, LPCTSTR seprator, int *count)
{  
	int     iSubStringCount,nIndex=0;  
	CString strSource=source,strSeperator=seprator,*pstrSubStrings;  

	if (strSource.IsEmpty())
	{
		*count = 0;
		return NULL;
	}

	if(strSeperator.GetLength()==0)  
	{
		*count=0;  
		return NULL;  
	}

	//seprator中的所有分隔符全部替换成第一个,并统计source中seprator的个数  
	iSubStringCount =1;  

	for(int i=0;i<strSource.GetLength();i++)  
	{  
		if(strSource[i]==strSeperator[0])  
			iSubStringCount++;  
	}  

	for(int i=1;i<strSeperator.GetLength();i++)  
	{  
		iSubStringCount+=strSource.Replace(strSeperator[i],strSeperator[0]);  
	}  

	pstrSubStrings=new CString[iSubStringCount];  

	int nNewIndex,nCount=0;  
	while(nIndex<strSource.GetLength())  
	{  
		nNewIndex=strSource.Find(strSeperator[0],nIndex);  
		if(nNewIndex>=0)  
			pstrSubStrings[nCount++]=strSource.Mid(nIndex,nNewIndex-nIndex);  
		else  
			pstrSubStrings[nCount++]=strSource.Mid(nIndex);  

		nIndex+=pstrSubStrings[nCount-1].GetLength()+1;  
	}  

	*count=iSubStringCount;  
	return pstrSubStrings;  
} 

void CCommon::FreeStrings(CString *lpstr, int count)
{  
	if(lpstr==NULL || count<=0)  
	{  
		return;  
	}

	delete[] lpstr;  
}

void CCommon::Copy2WideChar(CString &strDest, char* pSource,int nLen)  
{
	if(nLen < 0)
	{
		nLen = strlen(pSource);
	}

#ifdef UNICODE
	TCHAR *pBuffer = new TCHAR[nLen+1];
	memset(pBuffer,0,sizeof(TCHAR)*(nLen+1));

	MultiByteToWideChar(CP_ACP,0,pSource,nLen,pBuffer,nLen+1);
	strDest = pBuffer;
#else
	strncpy_s(strDest.GetBuffer(nLen+1),nLen,pSource);
	strDest.ReleaseBuffer();
#endif
}

CString CCommon::ToWideChar(const char* pSource, int nLen)
{
	CString strDest;

#ifdef UNICODE
	if (nLen < 0) nLen = strlen(pSource);

	TCHAR *pBuffer = new TCHAR[nLen + 1];
	memset(pBuffer, 0, sizeof(TCHAR)*(nLen + 1));

	MultiByteToWideChar(CP_ACP, 0, pSource, nLen, pBuffer, nLen + 1);
	strDest = pBuffer;
#else
	strDest = pSource;
#endif

	return strDest;
}

void CCommon::Copy2MultiByte(CString strSource, char* pOut,int nBufferLen)  
{
	memset(pOut,0,nBufferLen);
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP,0,(LPCTSTR)strSource,-1,pOut,nBufferLen,NULL,NULL);  
#else
	strcpy_s(pOut,nBufferLen,(LPCSTR)strSource);
#endif
}

CString CCommon::GetExceptionErrorMessage(CException *pException)
{
	TCHAR sReason[1024];
	memset(sReason, 0, 1024);
	if(pException->GetErrorMessage(sReason, 1023))
		return CString(sReason);
	else
		return _T("");
}

void CCommon::FreeITEMIDLIST(LPITEMIDLIST pidl)
{
	IMalloc *pIMalloc;
	SHGetMalloc(&pIMalloc);

	if(pIMalloc != NULL)
	{
		pIMalloc->Free(pidl);
		pIMalloc->Release();  
	}
}

LPITEMIDLIST CCommon::GetDirItemIDLIST(CString strDir)
{
	LPITEMIDLIST  pidl;
	LPSHELLFOLDER pDesktopFolder;
	OLECHAR       olePath[MAX_PATH];
	ULONG         chEaten;

	// Get a pointer to the Desktop's IShellFolder interface.
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
		// IShellFolder::ParseDisplayName requires the file name be in Unicode.
#ifndef _UNICODE
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)strDir, -1, olePath, MAX_PATH);
#else
		_tcscpy_s(olePath, MAX_PATH,(LPCTSTR)strDir);
#endif
		// Convert the path to an ITEMIDLIST.
		if(FAILED(pDesktopFolder->ParseDisplayName(NULL, NULL, olePath, &chEaten, &pidl, NULL)))
		{
			//release the desktop folder object
			pDesktopFolder->Release();

			return NULL;
		}

		// pidl now contains a pointer to an ITEMIDLIST for .\readme.txt.
		// This ITEMIDLIST needs to be freed using the IMalloc allocator
		// returned from SHGetMalloc().

		//IMalloc *pIMalloc;
		//SHGetMalloc(&pIMalloc);
		//pIMalloc->Free(pidl);

		//release the desktop folder object
		pDesktopFolder->Release();
		return pidl;
	}

	return NULL;
}

bool CCommon::BrowseDir(CString &strDir, CString strTitle, int nImgID, CString strDefaultDir)
{
	CString sFolderPath;
	BROWSEINFO bi;
	TCHAR Buffer[MAX_PATH];
	bool bNeedFree = true;

	LPITEMIDLIST pidl = GetDirItemIDLIST(strDefaultDir);
	if(pidl == NULL)
	{
		SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);
		bNeedFree = false;
	}

	bi.hwndOwner = NULL;
	bi.pidlRoot = pidl;
	bi.pszDisplayName = Buffer;
	bi.lpszTitle = strTitle;
	bi.ulFlags = BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;

	bi.lpfn = NULL;
	bi.iImage = nImgID;

	LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);
	if(pIDList)
	{
		SHGetPathFromIDList(pIDList, Buffer);
		strDir = Buffer;
	}
	else
	{
		if(bNeedFree) FreeITEMIDLIST(pidl);
		return FALSE;
	}

	if(bNeedFree)
	{
		FreeITEMIDLIST(pidl);
		pidl = NULL;
	}

	FreeITEMIDLIST(pIDList);
	pIDList = NULL;

	return true;
}

CString CCommon::GetSystemErrorMessage()
{
	return GetSystemErrorMessage(GetLastError());
}

CString CCommon::GetSystemErrorMessage(DWORD deErrCode)
{
	CString strRet;
	LPVOID lpMsgBuf;

	if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,deErrCode,GetUserDefaultUILanguage(),(LPTSTR) &lpMsgBuf,0, NULL ))
	{
		//DWORD dd = GetLastError();
		//if(dd == ERROR_RESOURCE_LANG_NOT_FOUND || dd == ERROR_MUI_FILE_NOT_FOUND)
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,deErrCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );
		}
	}

	strRet = (LPTSTR)lpMsgBuf;
	LocalFree(lpMsgBuf);

	return strRet;
}

bool CCommon::GetGPIBVersion(CString &strGPIBVersion)
{
	const int buff_len = 256;
	TCHAR ni_version[32] = { 0 };
	TCHAR ni_message[buff_len] = { 0 };
	int retval = 0;

	DWORD dwSize = 32;
	HKEY hKey;
	DWORD dwType = REG_SZ;

	strGPIBVersion.Empty();

	// NI 488.2 driver info
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\National Instruments\\NI-488.2\\CurrentVersion"), 0L, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	if(RegQueryValueEx(hKey, _T("VersionString"), NULL, &dwType, (LPBYTE)ni_version, &dwSize) == ERROR_SUCCESS)
	{
		strGPIBVersion.Format(_T("National Instruments 488.2 driver version:%s"), ni_version);
	}

	RegCloseKey(hKey);
	return !strGPIBVersion.IsEmpty();
}

bool CCommon::GetVISAVersion(CString &strVISAVersion)
{
	const int buff_len = 256;
	TCHAR ni_version[32] = { 0 };
	TCHAR ni_message_visa[buff_len] = { 0 };
	int retval = 0;

	DWORD dwSize = 32;
	HKEY hKey;
	DWORD dwType = REG_SZ;

	strVISAVersion.Empty();

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\National Instruments\\NI-VISA\\CurrentVersion"), 0L, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\National Instruments\\NI-VISA for Windows 95/NT\\CurrentVersion"), 0L, KEY_READ, &hKey) != ERROR_SUCCESS)
		{
			return false;
		}
		else if (RegQueryValueEx(hKey, _T("Version"), NULL, &dwType, (LPBYTE)ni_version, &dwSize) == ERROR_SUCCESS)
		{
			strVISAVersion.Format(_T("VISA driver version %s"), ni_version);
		}
	}
	else if (RegQueryValueEx(hKey, _T("Version"), NULL, &dwType, (LPBYTE)ni_version, &dwSize) == ERROR_SUCCESS)
	{
		strVISAVersion.Format(_T("VISA driver version %s"), ni_version);
	}

	RegCloseKey(hKey);
	return !strVISAVersion.IsEmpty();
}

void CCommon::GetSupportedTester(CArray<CString> &ts)
{
	ts.Add(_T("Agilent8960"));
	ts.Add(_T("CMU200"));
	ts.Add(_T("CMW500"));
}

bool CCommon::IsIntegerOrFloat(CString str)
{
	TCHAR chars[] = _T("0123456789.-Ee");
	for (int i = 0; i < str.GetLength(); i++)
	{
		if (!_tcschr(chars, str[i])) return false;
	}

	return true;
}

CString CCommon::GetSystemTimeString()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	CTime curtime(st);

	CString strms;
	strms.Format(_T(".%d"), st.wMilliseconds);

	return (curtime.Format("%Y-%m-%d %H-%M-%S") + strms);
}

CString CCommon::FormatString(LPCTSTR pstrFormat, ...)
{
	CString strResult;

	va_list args;
	va_start(args, pstrFormat);
	strResult.FormatV(pstrFormat, args);
	va_end(args);

	return strResult;
}

std::string CCommon::ToUTF8(CString strDir)
{
	int nLen = 0;
	WCHAR* strA = NULL;

#ifndef _UNICODE
	nLen = MultiByteToWideChar(CP_ACP, 0, strDir.GetBuffer(), -1, NULL, 0);

	strA = new  WCHAR[nLen + 1];
	MultiByteToWideChar(CP_ACP, 0, strDir.GetBuffer(), -1, strA, nLen);
#else
	nLen = strDir.GetLength();
	strA = new WCHAR[nLen + 1];

	_tcscpy_s(strA, nLen + 1, strDir);
#endif

	nLen = WideCharToMultiByte(CP_UTF8, 0, strA, -1, NULL, 0, NULL, NULL);
	char  *strRes = new  char[nLen + 1];
	WideCharToMultiByte(CP_UTF8, 0, strA, -1, strRes, nLen, NULL, NULL);

	std::string strRet = strRes;

	delete[] strA;
	delete[] strRes;

	return strRet;
}

std::string CCommon::CStringToSTLstring(CString str)
{
	std::string sOut;
#ifdef _UNICODE
	int iTextLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);

	char *pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char)* (iTextLen + 1));

	WideCharToMultiByte(CP_ACP, 0, str, -1, pElementText, iTextLen, NULL, NULL);

	sOut.assign(pElementText);

	delete[] pElementText;
#else
	sOut.assign((LPCSTR)str);
#endif

	return sOut;
}