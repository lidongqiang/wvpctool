#ifndef _STRCONVERT_H_
#define _STRCONVERT_H_
BOOL            IsNumberString          (const TCHAR *);
BOOL            IsHexStringW            (const TCHAR *);
BOOL            IsHexStringA            (const char  *);
BOOL            IntStrIncrease          (TCHAR *    ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);
BOOL            HexStrIncrease          (TCHAR *    ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);
BOOL            HexStrIncreaseSkipAlpha (TCHAR * ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);
BOOL            IntStrIncreaseSkipAlpha (TCHAR * ,int spn = 1,TCHAR *strNEW = NULL,int len = 0);

BOOL            IsNumberStringA         (const char *);
DWORD           intToHexLen             (DWORD );
DWORD           intToDecLen             (DWORD );
int             hexchar2value           (const char ch);
int             whexchar2value          (const wchar_t ch);
#endif
