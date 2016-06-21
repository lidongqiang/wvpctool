#ifndef _COM_H_
#define _COM_H_

#define WM_UPDATE_MSG               WM_USER+7
typedef struct  
{
    int flag;
    TCHAR   pszLineText[MAX_PATH];
}STRUCT_LIST_LINE,*PSTRUCT_LIST_LINE;
#define UPDATE_LIST         3
#define LIST_EMPTY          4
#define UPDATE_PROMPT       5
#define PROMPT_TESTING      1
#define PROMPT_PASS         2
#define PROMPT_FAIL         3
#define PROMPT_ABORT        4
#define PROMPT_EMPTY        5
#define LIST_INFO 0
#define LIST_TIME 1
#define LIST_WARN 2
#define LIST_ERR  3

BOOL BrowseDialog(CString &strSelectFile,CString strRoot=_T(""),CString strInitialDir=_T(""),LPCTSTR strTitle=NULL,BOOL bIncludeFile=FALSE);
unsigned int Str2Int(const char *ptr);
unsigned int Str2IntW(const wchar_t *ptr);
BOOL         IsxDigitalStrW(const wchar_t *str);
typedef BOOL (WINAPI *EnumerateFunc) (LPCTSTR lpFileOrPath,BOOL bEnumFiles,void* pUserData);
BOOL FileEnum(LPCTSTR lpPath, BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData,LPTSTR partten = NULL);

#define FINDFILE_START { \
        int             cbpathlen; \
        WIN32_FIND_DATA fd;\
        BOOL    bIsDirectory;\
        TCHAR   FindFileName[MAX_PATH];\
        HANDLE          hFindFile;\
        TCHAR           szPath[MAX_PATH];\
        BOOL            bFinish;\
        cbpathlen = _tcslen(lpPath);\
        if(NULL == lpPath || 0 >= cbpathlen)  {\
            return FALSE;\
        }\
        _tcscpy(szPath, lpPath);\
        if(TEXT('\\') != lpPath[cbpathlen - 1] ) {\
            _tcscat(szPath,TEXT("\\"));\
        }\
        if(partten) {\
            _tcscat(szPath,partten);\
        } else {\
            _tcscat(szPath,TEXT("*"));\
        }\
        bFinish = FALSE;\
        hFindFile   = FindFirstFile(szPath, &fd);\
        if( INVALID_HANDLE_VALUE == hFindFile ) {\
                bFinish = TRUE;\
        }\
        while(!bFinish) {\
            _tcscpy(FindFileName,lpPath);\
            if(TEXT('\\') != lpPath[cbpathlen - 1] )  {\
                _tcscat(FindFileName, TEXT("\\"));\
            }\
            _tcscat(FindFileName,fd.cFileName);\
            bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);\
            if( bIsDirectory&&(_tcscmp(fd.cFileName,TEXT("."))== 0|| _tcscmp(fd.cFileName, TEXT("..")) == 0))  {\
                bFinish = (FindNextFile(hFindFile, &fd) == FALSE);\
                continue;\
            }

#define FINDFILE_END if(!bFinish) {\
                bFinish = (FindNextFile(hFindFile, &fd) == FALSE);\
            }\
        }\
        if(INVALID_HANDLE_VALUE != hFindFile) {\
            FindClose(hFindFile);\
        }\
    }
#define FINDFILE_STOP  bFinish=TRUE;

#endif
