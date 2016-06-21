/**
  Copyright (C) Infineon Wireless Design A/S. All rights reserved.

  This document contains proprietary information belonging to Infineon. Passing on and copying of this document, use and communication
  of its contents is not permitted without prior written authorisation.

  Description:
    This is header file for I2C Driver routines.

  Revision Information:
    File name:
    Version:
    Date:
    Responsible: futtrup
    Comment:
      ** please put NO documentation related comments here! **
 */

#include "stdafx.h"
#include "strtok.h"

#ifndef __MINGW32__
#pragma hdrstop
#endif

StrTok::StrTok(const char *pIn,const char *p) :  curTokenLength(0)
{
  // initialise bit table
  // each possible character value can get a bit set
  curpos = (unsigned char*)pIn;
  startpos = (unsigned char*)pIn;

  memset(map,0,sizeof(map));
  for(unsigned char *ctrl = (unsigned char*) p;*ctrl;*ctrl++)
    map[*ctrl >> 3] |= (1 << (*ctrl & 7));

  // find the first token.
  ++(*this);
}

StrTok &StrTok::operator++()
{
  // move to start of string if we are out of it.

  if (curpos == NULL)
      return *this;
    
  if (curpos < startpos)
	curpos = startpos;

  // skip till end of token
  while(*curpos && IsSkipChar(curpos))
   curpos++;

  // this is the start of the token
  curToken = (char*)curpos;

  // skip till end of token
  for(;*curpos;curpos++)
    if (IsSkipChar(curpos))
	  break;

  // end of search condition
  if (curpos == (unsigned char*)curToken)
  {
	curToken = 0;
	curTokenLength = 0;
  }
  else
    curTokenLength = curpos - (unsigned char*)curToken;
  return *this;
}

StrTok &StrTok::operator--()
{
  unsigned char *pEndOfToken;

  // move to end of string if we are out of it.
  if (*curpos==0)
	-- curpos;

  // skip till end of previous token
  while(curpos>startpos && IsSkipChar(curpos))
   curpos--;
  pEndOfToken = curpos;

  // skip till start of previous token
  for(;curpos>startpos;curpos--)
	if (IsSkipChar(curpos))
	  break;

  if (pEndOfToken == curpos)
  {
	curToken = 0;
	curTokenLength = 0;
  }
  else
  {
	curToken = (char*)curpos+1;
    curTokenLength = pEndOfToken - curpos;
  }
  return *this;
}

int  StrTok::strcmp(const char *arg) const
{
  int ret = strncmp(curToken,arg,curTokenLength);
  if (ret)
	return ret;
  if (arg[curTokenLength]!='\0')
	return -1;
  return 0;
}

int  StrTok::stricmp(const char *arg) const
{
#ifdef _MSC_VER
  int ret = _strnicmp(curToken,arg,curTokenLength);
#elif _WINDOWS
  int ret = strnicmp(curToken,arg,curTokenLength);
#else
  int ret = strncasecmp(curToken,arg,curTokenLength);
#endif
  if (ret)
	return ret;
  if (arg[curTokenLength]!='\0')
	return -1;
  return 0;
}

