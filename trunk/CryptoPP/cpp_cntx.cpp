#include "commonheaders.h"


list<pCNTX> CL; // CL.size() CL.empty()

HANDLE thread_timeout = 0;

unsigned __stdcall sttTimeoutThread(LPVOID);


// get context data on context id
pCNTX get_context_on_id(HANDLE context) {

    if(	!thread_timeout ) {
	unsigned int tID;
	thread_timeout = (HANDLE) _beginthreadex(NULL, 0, sttTimeoutThread, NULL, 0, &tID);
    }

    if( context ) {
	pCNTX cntx = (pCNTX) context;
	if( cntx->header == HEADER && cntx->footer == FOOTER )
		return cntx;
#if defined(_DEBUG) || defined(NETLIB_LOG)
	else
		Sent_NetLog("get_context_on_id: corrupted context %08X", cntx);
#endif
    }
    return NULL;
}

/*
pCNTX get_context_on_id(HANDLE context) {
	return get_context_on_id((int)context);
}
*/

// create context, return context id
HANDLE __cdecl cpp_create_context(int mode) {

	list<pCNTX>::iterator i;
	pCNTX cntx = NULL;

	EnterCriticalSection(&localContextMutex);

	if( !CL.empty() ) {
		for(i=CL.begin(); i!=CL.end(); ++i) { // ���� ������
			if( (*i)->header==EMPTYH && (*i)->footer==EMPTYH ) {
	    	    cntx = (pCNTX) *i;
	    	    break;
			}
		}
	}

	if( !cntx ) { // �� ����� - ������� �����
	    cntx = (pCNTX) malloc(sizeof(CNTX));
	    CL.push_back(cntx); // �������� � ����� ������
	}

	memset(cntx,0,sizeof(CNTX)); // ������� ���������� ����
	cntx->header = HEADER;
	cntx->footer = FOOTER;
	cntx->mode = mode;

	LeaveCriticalSection(&localContextMutex);

	return (HANDLE)cntx;
}


// delete context
void __cdecl cpp_delete_context(HANDLE context) {

	pCNTX tmp = get_context_on_id(context);
	if(tmp) { // �������� �� ��������
		tmp->deleted = gettime()+10; // ����� ������ ����� 10 ������
	}
}


// reset context
void __cdecl cpp_reset_context(HANDLE context) {

	pCNTX tmp = get_context_on_id(context);
	if(tmp)	cpp_free_keys(tmp);
}


// allocate pdata
PBYTE cpp_alloc_pdata(pCNTX ptr) {
	if( !ptr->pdata ) {
	    if( ptr->mode & MODE_PGP ) {
			ptr->pdata = (PBYTE) malloc(sizeof(PGPDATA));
			memset(ptr->pdata,0,sizeof(PGPDATA));
	    }
	    else
	    if( ptr->mode & MODE_GPG ) {
			ptr->pdata = (PBYTE) malloc(sizeof(GPGDATA));
			memset(ptr->pdata,0,sizeof(GPGDATA));
	    }
	    else
	    if( ptr->mode & MODE_RSA ) {
			pRSADATA p = new RSADATA;
			p->state = 0;
			p->time = 0;
			p->thread = p->event = NULL;
			p->thread_exit = 0;
			p->queue = new STRINGQUEUE;
			ptr->pdata = (PBYTE) p;
	    }
	    else {
			ptr->pdata = (PBYTE) malloc(sizeof(SIMDATA));
			memset(ptr->pdata,0,sizeof(SIMDATA));
	    }
	}
	return ptr->pdata;
}


// search not established RSA/AES contexts && clear deleted contexts
unsigned __stdcall sttTimeoutThread( LPVOID ) {

	list<pCNTX>::iterator i;
	while(1) {
		Sleep( 1000 ); // ��� � �������
		if( CL.empty() ) continue;
		u_int time = gettime();
		// ��������� ��� ���������
		EnterCriticalSection(&localContextMutex);
	    for(i=CL.begin(); i!=CL.end(); ++i) {
	    	pCNTX tmp = *i;
			if( tmp->header!=HEADER || tmp->footer!=FOOTER ) continue;
			// ���������� ��������� �����
	    	if( tmp->mode&MODE_PRIV_KEY ) continue;
	    	else
			if( tmp->deleted && tmp->deleted < time ) {
				// ������� ���������� ��� �������� ��������
				cpp_free_keys(tmp);
				tmp->deleted = 0;
				tmp->header = tmp->footer = EMPTYH;
	    	}
	    	else
			if( tmp->mode&MODE_RSA && tmp->pdata ) {
				// ��������� �� �������� �� ����������
				pRSADATA p = (pRSADATA) tmp->pdata;
				if( p->time && p->time < time ) {
					rsa_timeout((HANDLE)tmp,p);
				}
	    	}
	    } // for
		LeaveCriticalSection(&localContextMutex);
	} //while
}


// EOF
