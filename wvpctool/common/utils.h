/********************************************************************************
*filename     :utils.h
*synopsis     :utilities
*created      :lanshh 2012-03-12
*last modified:lanshh 2015-04-21
********************************************************************************/
#ifndef __UTILS_H
#define __UTILS_H
#include "debug.h"
//#define  LOG(a) if(getInfo()->pLog) getInfo()->pLog->PrintMSGA a
/*****************************************************************************************
* generate an entry that converts the #define'd value into string  
******************************************************************************************/
#define LSH_ENTRY(value)                { value, L#value }
/****************** Returns Number Of Elements *********************/
#define dim(x) (sizeof(x) / sizeof(x[0]))
// Convert wstring to string
std::string     wstr2str                (const std::wstring& arg);
std::wstring    str2wstr                (const std::string& arg);
void            ParseIntegers           (std::wstring s, int *pInt, int nLen);
void            ParseFloats             (std::wstring s, double *pInt, int *pLen);
BOOL            IsNumberString          (const TCHAR *);
BOOL            IsHexStringW            (const TCHAR *);
BOOL            IsHexStringA            (const char  *);
BOOL            IntStrIncrease          (TCHAR * ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);
BOOL            HexStrIncrease          (TCHAR * ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);
BOOL            HexStrIncreaseSkipAlpha (TCHAR * ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);
BOOL            IntStrIncreaseSkipAlpha (TCHAR * ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);

BOOL            IsNumberStringA         (const char *);
DWORD           intToHexLen             (DWORD );
DWORD           intToDecLen             (DWORD );
int             hexchar2value           (const char ch);
int             whexchar2value          (const wchar_t ch);
#endif
