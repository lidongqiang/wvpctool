/********************************************************************************
*filename     :utils.h
*synopsis     :utilities
*created      :lanshh 2012-03-12
*last modified:lanshh 2015-04-21
********************************************************************************/
#include "stdafx.h"
#include <string>
#include "utils.h"
// Convert wstring to string
std::string wstr2str(const std::wstring& arg)
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
std::wstring str2wstr(const std::string& arg)
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
void trim(std::string& input ,std::string whitespace )
{
	int start_pos   = 0;
	int end_pos     = 0;
	for (unsigned int i = 0; i < input.size(); ++i)
	{
		if (whitespace.find(input[i]) != std::string::npos)
		{
			start_pos++;
		}
		else
		{
			break;
		}
	}
	for (int i = input.size() - 1; i >= 0; --i)
	{
		if (whitespace.find(input[i]) != std::string::npos)
		{
			end_pos++;
		}
		else
		{
			break;
		}
	}
    input = input.substr(start_pos, input.size() - start_pos - end_pos);
}
void ParseIntegers(std::wstring s, int *pInt, int nLen)
{
    std::wstring            subs;
    std::string::size_type  posl,posr;
    if (s.empty()) return;
    posl    = 0;
    for(int j = 0;j  < nLen ;j++) {
        posr = s.find_first_of(TEXT(","),posl);
        if(std::string::npos != posr&&posr > posl) {
            subs = s.substr(posl,posr - posl );
            posl = posr + 1;
            pInt[j] = _ttoi(subs.c_str());
        } else if (std::string::npos == posr&&posr > posl){
            subs = s.substr(posl);
            pInt[j] = _ttoi(subs.c_str());
            break;
        }
    }
parseintegers_exit:
    return;
}
void ParseFloats(std::wstring s, double *pInt, int *pLen)
{
    int                     j,iLen    = *pLen;
    std::wstring            subs;
    std::string::size_type  posl,posr;
    *pLen = 0;
    if (s.empty()) return;
    posl    = 0;
    for( j = 0;j  < iLen ;) {
        posr = s.find_first_of(TEXT(","),posl);
        if(std::string::npos != posr&&posr > posl) {
            subs = s.substr(posl,posr - posl );
            posl = posr + 1;
            pInt[j++] = _tstof(subs.c_str());
        } else if (std::string::npos == posr&&std::string::npos != posl){
            subs = s.substr(posl);
            pInt[j++] = _tstof(subs.c_str());
            break;
        }
    }
    *pLen = j;
parseintegers_exit:
    return;
}

/**************************************************************
* CHAR:0--------9  A-------F    a---------f
* HEX :30      39  41     46   61--------66
*
***************************************************************/
BOOL IsNumberString(const TCHAR *str)
{
    int     nIndex;
    int     nLen;
    if(NULL == str){
        return FALSE;
    }
    nLen    = _tcslen( str );
    if ( 0 == nLen ) {
        return FALSE;
    }
    nIndex = _tcsspn( str, _T("0123456789") );
    return ( nLen == nIndex ) ? TRUE : FALSE;
}
BOOL IsNumberStringA(const char *str)
{
    int     nIndex;
    int     nLen;
    if(NULL == str){
        return FALSE;
    }
    nLen    = strlen( str );
    if ( 0 == nLen ) {
        return FALSE;
    }
    nIndex = strspn( str, "0123456789" );
    return ( nLen == nIndex ) ? TRUE : FALSE;
}
BOOL IsHexStringW(const TCHAR *str)
{
    int         nIndex;
    int         nLen;
    if(NULL == str){
        return FALSE;
    }    
    nLen    = _tcslen( str );
    if ( 0 == nLen ) {
        return FALSE;
    }
    nIndex = _tcsspn( str, _T("0123456789ABCDEFabcdef"));
    return ( nLen == nIndex ) ? TRUE : FALSE;
}
BOOL IsHexStringA(const char *str)
{
    int         nIndex;
    int         nLen;
    if(NULL == str){
        return FALSE;
    }    
    nLen    = strlen( str );
    if ( 0 == nLen ) {
        return FALSE;
    }
    nIndex = strspn( str, "0123456789ABCDEFabcdef");
    return ( nLen == nIndex ) ? TRUE : FALSE;
}
DWORD intToHexLen(DWORD dwValue)
{
    DWORD       dwLen;
    dwLen       = 0;
    dwValue     /=16;
    while(dwValue){
        dwLen ++;
        dwValue /=16;
    }
    return dwLen;
}
DWORD intToDecLen(DWORD dwValue)
{
    DWORD       dwLen;
    dwLen       = 0;
    dwValue     /=10;
    while(dwValue){
        dwLen ++;
        dwValue /=10;
    }
    return dwLen;
}
BOOL IntStrIncrease(TCHAR *strValue,int spn,TCHAR *strNEW,int len)
{
    TCHAR       szChar;
    TCHAR       newChar;
    TCHAR       *STR;
    BOOL        isINC;
    int         i;
    int         nLen;
    if (!IsNumberString(strValue)||spn > 10 ||spn < -10) {
        return FALSE;
    }
    if(0 > spn){
        spn   = -spn;
        isINC   = FALSE;
    } else if(0 < spn){
        isINC   = TRUE;
    } else {
        return FALSE;
    }
    nLen    = _tcslen(strValue);
    if( NULL != strNEW ){
        if(nLen + 1 > len ){
            return FALSE;
        }
        wsprintf(strNEW,TEXT("%s"),strValue);
        STR = strNEW;
    } else {
        STR = strValue;
    }
    if(isINC) {
        for (i = nLen-1; i >= 0; i-- ) {
            szChar  = STR[i];
            newChar = szChar + spn;
            if (newChar <=_T('9') ) {
                szChar  = newChar;
                spn     =  0;
            } else if (newChar > _T('9')) {
                szChar  = newChar - 10;
                STR[i]  = szChar;
                spn     =  1;
                continue;
            }
            STR[i] = szChar;
            break;
        }
    } else {
        for (i = nLen-1; i >= 0; i-- ) {
            szChar  = STR[i];
            newChar = szChar - spn;
            if (newChar >=_T('0')) {
                szChar  = newChar;
                spn     =  0;
            } else if ( newChar < _T('0') ){
                szChar  = newChar + 10 ;
                STR[i]  = szChar;
                spn     =  1;
                continue;
            }
            STR[i] = szChar;
            break;
        }
    }
    return TRUE;
}
/**************************************************************
* CHAR:0--------9  A-------F    a---------f
* HEX :30      39  41     46   61--------66
*
***************************************************************/
/*
*for plus result calculating,the input must in range 0 ~ 31
*
*
*/
static BOOL HEXTOCHARP(int hex,int *out)
{
    if(hex < 10){                   /*0 ---- 9 */
        *out = hex + TEXT('0');
        return FALSE;
    }else if (hex < 16 ){           /*10----15*/
        *out = hex - 10 + TEXT('A');
        return FALSE;
    }else if (hex < 26){
        *out = hex - 16 + TEXT('0');/*16----25 carry out */
        return TRUE;
    }else if (hex < 32){
        *out = hex - 26 + TEXT('A');/*26----31 carry out */
        return TRUE;
    }
    return FALSE;
}
/*
*for minus result calculating,the input must in range -15 ~ +15
*
*
*/
static BOOL HEXTOCHARM(int hex,int *out)
{
    if(hex >= 0 ){
        if(hex < 10){                   /*0 ---- 9*/
            *out = hex + TEXT('0');
        }else if (hex < 16 ){           /*10----15*/
            *out = hex - 10 + TEXT('A');
        }
        return FALSE;   //hex > 0 ,just the result
    } else {
        if(hex < -6 ){  // -7 ---- -15 16 + hex = 9 8 7 6 5 4 3 2 1 0 
            *out = 16 + hex + TEXT('0');
        } else {        // -1 ---- -6  16 + hex = f e d c b a 
            *out = 1 + hex + TEXT('F');
        }
        return TRUE;    //hex < 0 ,need borrow 
    }
}
BOOL HexStrIncrease(TCHAR *strValue,int spn,TCHAR *strNEW,int len)
{
    int         szChar;
    TCHAR       *STR;
    int         newChar;
    BOOL        isINC;
    int         i;
    int         nLen;
    if (!IsHexStringW(strValue)||spn > 16 ||spn < -16 ) {  //here must ensure input is hex string and the span is less than 16,
                                                           //otherwise  will get wrong result 
        return FALSE;
    }
    if(0 > spn){
        spn   = -spn;
        isINC   = FALSE;
    } else if(0 < spn){
        isINC   = TRUE;
    } else {
        return FALSE;
    }
    nLen = _tcslen(strValue);
    if( NULL != strNEW ){
        if(nLen + 1 > len ){
            return FALSE;
        }
        wsprintf(strNEW,TEXT("%s"),strValue);
        STR = strNEW;
    } else {
        STR = strValue;
    }
    if(isINC) {
        for (i = nLen - 1;i >= 0;i-- ) {
            szChar  = STR[i];
            if(szChar >= TEXT('0')&&szChar <= TEXT('9')){
                newChar = szChar -  TEXT('0') + spn;
            }else if (szChar >= TEXT('A')&&szChar <= TEXT('F')){
                newChar = szChar -  TEXT('A') + spn + 10;
            }else if (szChar >= TEXT('a')&&szChar <= TEXT('f')){
                newChar = szChar -  TEXT('a') + spn + 10;
            }
            if(HEXTOCHARP(newChar,&szChar)){    //the input of HEXTOCHARP range is from 0 + 0 to 15 + 16
                                                
                STR[i]  =(TCHAR)szChar;
                spn     = 1;
                continue;
            } 
            STR[i] = (TCHAR)szChar;
            break;
        }
    } else {
        for (i = nLen-1;i >= 0;i--) {
            szChar  = STR[i];
            if(szChar >= TEXT('0')&&szChar <= TEXT('9')){
                newChar = szChar -  TEXT('0') - spn;
            }else if (szChar >= TEXT('A')&&szChar <= TEXT('F')){
                newChar = szChar -  TEXT('A') - spn + 10;
            }else if (szChar >= TEXT('a')&&szChar <= TEXT('f')){
                newChar = szChar -  TEXT('a') - spn + 10;
            }
            if(HEXTOCHARM(newChar,&szChar)){ //the input of HEXTOCHARP range is from -16 to 15
                STR[i]  =(TCHAR)szChar;
                spn     = 1;
                continue;
            } 
            STR[i] = (TCHAR)szChar;
            break;
        }
    }
    return TRUE;
}


BOOL HexStrIncreaseSkipAlpha(TCHAR *strValue,int spn,TCHAR *strNEW,int len)
{
    int         szChar;
    TCHAR       *STR;
    int         newChar;
    BOOL        isINC;
    int         i;
    int         nLen;
    if ( spn > 16 ||spn < -16 ) {  //here must ensure input is hex string and the span is less than 16,
                                                           //otherwise  will get wrong result 
        return FALSE;
    }
    if(0 > spn){
        spn   = -spn;
        isINC   = FALSE;
    } else if(0 < spn){
        isINC   = TRUE;
    } else {
        return FALSE;
    }
    nLen = _tcslen(strValue);
    if( NULL != strNEW ){
        if(nLen + 1 > len ){
            return FALSE;
        }
        wsprintf(strNEW,TEXT("%s"),strValue);
        STR = strNEW;
    } else {
        STR = strValue;
    }
    if(isINC) {
        for (i = nLen - 1;i >= 0;i-- ) {
            szChar  = STR[i];
            if(szChar >= TEXT('0')&&szChar <= TEXT('9')){
                newChar = szChar -  TEXT('0') + spn;
            }else if (szChar >= TEXT('A')&&szChar <= TEXT('F')){
                newChar = szChar -  TEXT('A') + spn + 10;
            }else if (szChar >= TEXT('a')&&szChar <= TEXT('f')){
                newChar = szChar -  TEXT('a') + spn + 10;
            } else {
                continue;
            }
            if(HEXTOCHARP(newChar,&szChar)){    //the input of HEXTOCHARP range is from 0 + 0 to 15 + 16
                                                
                STR[i]  =(TCHAR)szChar;
                spn     = 1;
                continue;
            } 
            STR[i] = (TCHAR)szChar;
            break;
        }
    } else {
        for (i = nLen-1;i >= 0;i--) {
            szChar  = STR[i];
            if(szChar >= TEXT('0')&&szChar <= TEXT('9')){
                newChar = szChar -  TEXT('0') - spn;
            }else if (szChar >= TEXT('A')&&szChar <= TEXT('F')){
                newChar = szChar -  TEXT('A') - spn + 10;
            }else if (szChar >= TEXT('a')&&szChar <= TEXT('f')){
                newChar = szChar -  TEXT('a') - spn + 10;
            } else {
                continue;
            }
            if(HEXTOCHARM(newChar,&szChar)){ //the input of HEXTOCHARP range is from -16 to 15
                STR[i]  =(TCHAR)szChar;
                spn     = 1;
                continue;
            } 
            STR[i] = (TCHAR)szChar;
            break;
        }
    }
    return TRUE;
}

BOOL IntStrIncreaseSkipAlpha(TCHAR *strValue,int spn,TCHAR *strNEW,int len)
{
    TCHAR       szChar;
    TCHAR       newChar;
    TCHAR       *STR;
    BOOL        isINC;
    int         i;
    int         nLen;
    if ( spn > 10 ||spn < -10) {
        return FALSE;
    }
    if(0 > spn){
        spn   = -spn;
        isINC   = FALSE;
    } else if(0 < spn){
        isINC   = TRUE;
    } else {
        return FALSE;
    }
    nLen    = _tcslen(strValue);
    if( NULL != strNEW ){
        if(nLen + 1 > len ){
            return FALSE;
        }
        wsprintf(strNEW,TEXT("%s"),strValue);
        STR = strNEW;
    } else {
        STR = strValue;
    }
    if(isINC) {
        for (i = nLen-1; i >= 0; i-- ) {
            szChar  = STR[i];
            if(!isdigit(szChar)){
                continue;
            }
            newChar = szChar + spn;
            if (newChar <=_T('9') ) {
                szChar  = newChar;
                spn     =  0;
            } else if (newChar > _T('9')) {
                szChar  = newChar - 10;
                STR[i]  = szChar;
                spn     =  1;
                continue;
            }else {
                continue;
            }
            STR[i] = szChar;
            break;
        }
    } else {
        for (i = nLen-1; i >= 0; i-- ) {
            szChar  = STR[i];
            if(!isdigit(szChar)){
                continue;
            }
            newChar = szChar - spn;
            if (newChar >=_T('0')) {
                szChar  = newChar;
                spn     =  0;
            } else if ( newChar < _T('0') ){
                szChar  = newChar + 10 ;
                STR[i]  = szChar;
                spn     =  1;
                continue;
            }
            STR[i] = szChar;
            break;
        }
    }
    return TRUE;
}

int hexchar2value(const char ch)
{
    int result = 0;
    if(ch >= '0' && ch <= '9') {
        result = (int)(ch - '0');
    } else if(ch >= 'a' && ch <= 'z') {
        result = (int)(ch - 'a') + 10;
    } else if(ch >= 'A' && ch <= 'Z') {
        result = (int)(ch - 'A') + 10;
    } else{
        result = -1;
    }
    return result;
}

int whexchar2value(const wchar_t ch)
{
    int result = 0;
    if(ch >= L'0' && ch <= L'9') {
        result = (int)(ch - L'0');
    } else if(ch >= L'a' && ch <= L'z') {
        result = (int)(ch - L'a') + 10;
    } else if(ch >= L'A' && ch <= L'Z') {
        result = (int)(ch - L'A') + 10;
    } else{
        result = -1;
    }
    return result;
}

