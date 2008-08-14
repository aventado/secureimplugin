#ifndef __MMI_H__
#define __MMI_H__

#include "commonheaders.h"

char *m_wwstrcat(LPCSTR,LPCSTR);
char *m_awstrcat(LPCSTR,LPCSTR);
char *m_aastrcat(LPCSTR,LPCSTR);
char *m_ustrcat(LPCSTR,LPCSTR);
LPSTR to_hex(PBYTE,int);

void *operator new(size_t sz);
void operator delete(void *p);
void *operator new[](size_t size);
void operator delete[](void * p);

#define SAFE_INIT(t,p)			t p=NULL;
//#define SAFE_FREE(p)			if(p) {mir_free((PVOID)p); p=NULL;}
//#define SAFE_DELETE(p)			if(p) {delete p; p=NULL;}
#define SAFE_FREE(p)			safe_free((void **)&(p));
#define SAFE_DELETE(p)			safe_delete((void **)&(p));

void __fastcall safe_free(void** p);
void __fastcall safe_delete(void** p);

#endif
