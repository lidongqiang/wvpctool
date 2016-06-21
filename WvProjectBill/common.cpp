#include "stdafx.h"
#include "common.h"
unsigned int Str2Int(const char *ptr)
{
    unsigned int uiRet = -1;
    uiRet = 0;
    if(ptr) {
        for(;*ptr;ptr ++) {
    		char            val = *ptr;
    		unsigned int    ival = 0;
    		if(val>= 'a' && val <= 'f') 
                ival = val - 'a' + 10;
    		else if (val>= 'A' && val <= 'F') 
                ival = val - 'A' + 10;
    		else if (val>= '0' && val <= '9') 
                ival = val - '0';
            else 
                break;
    		uiRet = uiRet * 16  + ival;
        }
    }
	return uiRet;
}
unsigned int Str2IntW(const wchar_t *ptr)
{
    unsigned int uiRet = 0;
    if(ptr) {
        for(;*ptr;ptr ++) {
    		wchar_t         val = *ptr;
    		unsigned int    ival = 0;
    		if(val>= TEXT('a') && val <= TEXT('f')) 
                ival = val - TEXT('a') + 10;
    		else if (val>= TEXT('A') && val <= TEXT('F')) 
                ival = val - TEXT('A') + 10;
    		else if (val>= TEXT('0') && val <= TEXT('9')) 
                ival = val - TEXT('0');
            else 
                break;
    		uiRet = uiRet * 16  + ival;
        }
    }
	return uiRet;
}
BOOL IsxDigitalStrW(const wchar_t *str)
{
    if(!str) return FALSE;
    for(;*str;str++) {
        wchar_t val = *str;
		if(val>= TEXT('a') && val <= TEXT('f')) {
		}else if (val>= TEXT('A') && val <= TEXT('F')) {
        }else if (val>= TEXT('0') && val <= TEXT('9')) { 
        }else {
            return FALSE;
        }
    }
    return TRUE;

}
//bi.pidRoots was set,only dir and subdir can appear in the dialog.//ParsePidlFromPath convet path to pidroot
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg==BFFM_INITIALIZED) {
        CWnd *pDlg = CWnd::FromHandle(hwnd);
        pDlg->CenterWindow(CWnd::FromHandle(GetDesktopWindow()));
        pDlg->SetForegroundWindow();
        CString strInitialDir;
        strInitialDir = (LPTSTR)lpData;
        if (!strInitialDir.IsEmpty()) {
            ::SendMessage(hwnd,BFFM_SETSELECTION,TRUE,lpData);
        }
    }
    return 0;
}
BOOL BrowseDialog(CString &strSelectFile,CString strRoot,CString strInitialDir,LPCTSTR strTitle,BOOL bIncludeFile)
{
	TCHAR  szDisplayName[MAX_PATH] = _T("");
	TCHAR  szPath[MAX_PATH] = _T("");
	BROWSEINFO bi;
	bi.hwndOwner        = NULL;
	bi.pidlRoot         = NULL;
	bi.lpszTitle        = strTitle;
	bi.pszDisplayName   = szDisplayName;
	if (bIncludeFile) {
		bi.ulFlags      = BIF_BROWSEINCLUDEFILES;
    } else {
		bi.ulFlags      = BIF_RETURNONLYFSDIRS;
    }
    bi.lpfn             = BrowseCallbackProc;
    bi.lParam           = (LPARAM)(LPTSTR)(LPCTSTR)strInitialDir;
#if 1
    if (!strRoot.IsEmpty()) {
        LPSHELLFOLDER pShellFolder=NULL;
        HRESULT hr;
	    hr = SHGetDesktopFolder(&pShellFolder);
	    if (hr==NOERROR) {
            hr = pShellFolder->ParseDisplayName(NULL,NULL,strRoot.GetBuffer(MAX_PATH),NULL,(LPITEMIDLIST *)&bi.pidlRoot,NULL);
            strRoot.ReleaseBuffer();
        }
        pShellFolder->Release();	
    }
#endif
    LPITEMIDLIST pItemIDList = SHBrowseForFolder( &bi );
    if( pItemIDList ) {
        SHGetPathFromIDList(pItemIDList,szPath) ;
        IMalloc     *pMalloc;
        if( SHGetMalloc( &pMalloc ) != NOERROR ) {
            return FALSE;
        }
        pMalloc->Free( pItemIDList );
        if (bi.pidlRoot) {
            pMalloc->Free((LPITEMIDLIST)bi.pidlRoot);
        }
        if( pMalloc ) {
            pMalloc->Release();
        }
        strSelectFile = szPath;
    } else {
        return FALSE;
    }
    return TRUE;
}
BOOL FileEnum(LPCTSTR lpPath, BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData,LPTSTR partten)
{
    int                 cbpathlen;
    TCHAR               szPath[MAX_PATH];
    WIN32_FIND_DATA     fd;
    HANDLE              hFindFile;
    TCHAR               tempPath[MAX_PATH]; 
    BOOL                bIsDirectory;
    BOOL                bFinish     = FALSE;
    BOOL                busereturn  = TRUE;
    try {
        cbpathlen = _tcslen(lpPath);
        if(NULL == lpPath || 0 >= cbpathlen)  {
            return FALSE;
        }
        _tcscpy(szPath, lpPath);
        if(TEXT('\\') != lpPath[cbpathlen - 1] ) {
            _tcscat(szPath,TEXT("\\"));
        }
        if(partten) {
            _tcscat(szPath,partten);
        } else {
            _tcscat(szPath,TEXT("*"));
        }
        hFindFile   = FindFirstFile(szPath, &fd);
        if( INVALID_HANDLE_VALUE == hFindFile ) {
                return FALSE;
        }
        while(!bFinish) {
            _tcscpy(tempPath,lpPath);
            if(TEXT('\\') != lpPath[cbpathlen - 1] )  {
                _tcscat(tempPath, TEXT("\\"));
            }
            _tcscat(tempPath,fd.cFileName);
            bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
            if( bIsDirectory&&(_tcscmp(fd.cFileName,TEXT("."))== 0|| _tcscmp(fd.cFileName, TEXT("..")) == 0))  {
                bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
                continue;
            }
            if(pFunc && bEnumFiles != bIsDirectory) {
                busereturn = pFunc(tempPath,bIsDirectory, pUserData);
                if( FALSE == busereturn ) {
                    FindClose( hFindFile );
                    return FALSE;
                }
            }
            if(bIsDirectory && bRecursion) {
                if (FALSE == FileEnum(tempPath, bRecursion, bEnumFiles, pFunc, pUserData)) {
                    FindClose( hFindFile );
                    return FALSE;
                }
            }
            bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
        }
        FindClose(hFindFile);
    }
    catch(...)
    { 
        return FALSE; 
    }
    return TRUE;
}