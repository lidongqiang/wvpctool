/*   ----------------------------------------------------------------------
     Copyright (C) 2012 Intel Mobile Communications GmbH
     Copyright (C) 2008 - 2010 Infineon Technologies Denmark A/S (IFWD)
     ----------------------------------------------------------------------
     Revision Information:
       $$File name:  /msw_tools/calibration_rf/src/Utilities/Helpers/inc/strtok.h $
       $$CC-Version: .../oint_tools_calibration_src_01/1 $
       $$Date:       2012-05-16    16:17:26 UTC $
     ----------------------------------------------------------------------
     by CCCT (v0.4)
     ---------------------------------------------------------------------- */



#ifndef __my_STRTOK_HXX_1_
#define __my_STRTOK_HXX_1_

// put this header stuff into your precompiled header!!!
#ifndef __MINGW32__
#pragma warning(disable:4786)
#endif

#define _MIN(a,b) a<b?a:b

/*lint -w0 */
#include <stdlib.h>
#include <string.h>
/*lint -w2 */

class StrTok
{
  public:
    StrTok(const char *search,const char *pattern);

    StrTok &operator++();
    StrTok &operator--();
//StrTok &operator++(int); // don't do this for perfomance reasons.
//StrTok &operator--(int); // don't do this for perfomance reasons.

    // get info
    bool operator==(const char *) const;
    friend  bool operator==(const char *,const StrTok &);
    int  strcmp(const char *) const;
    int  stricmp(const char *) const;
    int  length() const;

    // extract string
    operator char*() const;
    void copyto(char *, int) const;
    void ncopyto(char *, int, int size) const;

  protected:
    int  IsSkipChar(unsigned char *);

    // parsing stuff
    char           map[32];
    unsigned char *startpos;
    unsigned char *curpos;

    // current token
    char          *curToken;
    int            curTokenLength;
};

inline int StrTok::IsSkipChar(unsigned char *curposi )
{
  return map[*curposi >> 3] & (1 <<(*curposi & 7));
}

inline StrTok::operator char*() const
{
//  if (curToken)
  return (curToken);
//  return AnsiString("");
}

inline int  StrTok::length() const
{
  return curTokenLength;
}

inline void StrTok::ncopyto(char *p, int size, int nBufSize) const
{
	strncpy_s(p, nBufSize, curToken, _MIN(size, curTokenLength));
}

inline void StrTok::copyto(char *p, int nBufSize) const
{
	strncpy_s(p, nBufSize, curToken, curTokenLength);
  p+=curTokenLength;
  *p='\0';
}

inline bool StrTok::operator==(const char *arg) const
{
  return this->strcmp(arg)==0;
}

inline bool operator==(const char *arg,const StrTok &tok)
{
  return tok.strcmp(arg)==0;
}
#endif
