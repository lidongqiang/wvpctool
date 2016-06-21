#ifndef _XMLLIB_H_
#define _XMLLIB_H_
#ifdef __cplusplus
extern "C" {
#endif
#define XML_CLOSE_SAVE          1
#define XML_OPEN_NEW            1
int     openconf        (char * name,unsigned int flags);
bool    closeconf       (int hconf,unsigned int flags);
bool    writeconf       (int hconf,char *option,char *field,char *value );
bool    writeconfattr   (int hconf,char *option,char *field,char *name,char *value );
bool    readconf        (int hconf,char *option,char *field,char *value,int *cnt);
bool    writevalue      (int hconf,const char *field,char *value );
bool    readvalue       (int hconf,const char *field,char *value,int *cnt);
#ifdef __cplusplus
};
#endif
#endif
