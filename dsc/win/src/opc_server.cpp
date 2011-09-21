#define _WIN32_DCOM

#include "opc_server.h"

#include <windows.h>
#include <olectl.h>
#include <oleauto.h>
#include <process.h>
#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include "opcda.h"
#include "opcerror.h"
#include "lightopc.h"

#include "cevent.h"

// отладочные функции
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_MSG_INFO1(X) \
	OPCServer::sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Info1)

#define DEBUG_MSG_GOOD(X) \
	OPCServer::sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Success)

#define DEBUG_MSG_WARN(X) \
	OPCServer::sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Warning)

#define DEBUG_MSG_ERR(X) \
	OPCServer::sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Error)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// статические
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ULog *								OPCServer::m_pLog					= 0;
int									OPCServer::m_iDebugLevel			= 8;
QString								OPCServer::m_sWDir					= QString::null;
QMap<ulong, CParameter*> * 			OPCServer::m_ptrmapParameters		= 0;
QList<CParameter*> * 				OPCServer::m_ptrlistToSend			= 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const loVendorInfo vendor = { 3, 2, 1, 0, "Diagnostic Server v1" };

loService *my_service;
static int driver_init(int lflags);
static void driver_destroy(void);
static void simulate(unsigned pause);

static const GUID CLSID_DSCOPCServer = { 0x4ea2713d, 0xca07, 0x11d4, {0xbe, 0xf5, 0x0, 0x0, 0x21, 0x20, 0xdb, 0x6c} };

myClassFactory my_CF;

void myClassFactory::serverAdd(void)
{
  EnterCriticalSection(&lk_count);
  if (is_out_of_proc) CoAddRefServerProcess();  
  ++server_count;
  LeaveCriticalSection(&lk_count);
}

void myClassFactory::serverRemove(void)
{
  EnterCriticalSection(&lk_count);
  if (is_out_of_proc)
    {
     if (0 == CoReleaseServerProcess())
       server_inuse = 0;
    }
  if (0 == --server_count && server_inuse) server_inuse = 0;
  LeaveCriticalSection(&lk_count);
}

static void a_server_finished(void *arg, loService *b, loClient *c)
{
  /* ARG is the same as we've passed to loClientCreate() */
//  UL_DEBUG((LOGID, "a_server_finished(%lu)...", my_CF.server_count));
  my_CF.serverRemove();

/* OPTIONAL: */
//  UL_INFO((LOGID, "a_server_finished(%lu) USERID=<%ls>",
//    my_CF.server_count, arg? arg: L"<null>"));
}

STDMETHODIMP myClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObject)
{
  IUnknown *server = 0;
  HRESULT hr = S_OK;

/* OPTIONAL: security stuff */
  OLECHAR *userid = 0;
  wchar_t *cuserid;
  LONG userno;
  static LONG usercount;

  userno = InterlockedIncrement(&usercount);

//  CoQueryClientBlanket(0, 0, 0, 0, 0, (RPC_AUTHZ_HANDLE*)&userid, 0);
  if (!userid) userid = L"{unknown}";

//  UL_WARNING((LOGID, "USER:#%ld <%ls>", userno, userid));

  if (cuserid = (wchar_t*)malloc((wcslen(userid) + 16) * sizeof(wchar_t)))
    swprintf(cuserid, L"#%ld %ls", userno, userid);
/* -- end of security stuff */

  if (pUnkOuter)
    {
#if 1 /* Do we support aggregation? */
     if (riid != IID_IUnknown) 
#endif
          return CLASS_E_NOAGGREGATION;
    }

  serverAdd();  /* the lock for a_server_finished() */

/***********************************************
/ * check other conditions (i.e. security) here *
/ ***********************************************/

  if (0 == server_inuse) /* we're stopped */
    {                    /* stopping initiated when there are no active instances */
      if (is_out_of_proc) 
        {                /* a stopped EXE should exit soon and can't work anymore */
 //         UL_MESSAGE((LOGID, "myClassFactory:: Server already finished"));
//		  printf( "myClassFactory:: Server already finished" );
          serverRemove();
          return E_FAIL;
        }
 //     else /* in-proc specefic here: */
 //       {  /* restart the server if not already running */
 //         dll_simulator_wait();   /* wait for stopping complete */
 //         dll_simulator_start();  /* then restart */
 //         if (0 == server_inuse)  
 //           {
 ////             UL_MESSAGE((LOGID, "myClassFactory:: Can't start server"));
 //             serverRemove();
 //             return E_FAIL;
 //           }
 //       }
    }
{
 IUnknown *inner = 0;
 if (loClientCreate_agg(my_service, (loClient**)&server, 
                       pUnkOuter, &inner,
                       0, &vendor, a_server_finished, cuserid/*this*/))
    {
      serverRemove();
      hr = E_OUTOFMEMORY;
//      UL_MESSAGE((LOGID, "myClassFactory::loClientCreate_agg() failed"));
//	  printf( "myClassFactory::loClientCreate_agg() failed" );
    }
  else if (pUnkOuter) *ppvObject = (void*)inner; /*aggregation requested*/
  else /* no aggregation */
    {
/* loClientCreate(my_service, (loClient**)&server, 
                  0, &vendor, a_server_finished, cuserid) - with no aggregation */
      /* Initally the created SERVER has RefCount=1 */
      hr = server->QueryInterface(riid, ppvObject); /* Then 2 (if success) */
      server->Release(); /* Then 1 (on behalf of client) or 0 (if QI failed) */
//      if (FAILED(hr)) 
//        UL_MESSAGE((LOGID, "myClassFactory::loClient QueryInterface() failed"));
//	  printf("myClassFactory::loClient QueryInterface() failed");
      /* So we shouldn't carry about SERVER destruction at this point */
    }
}
  if (SUCCEEDED(hr))
    {
      loSetState(my_service, (loClient*)server,
             loOP_OPERATE, (int)OPC_STATUS_RUNNING, /* other states are possible */
             "Finished by client");
 //     UL_DEBUG((LOGID, "myClassFactory::server_count = %ld", server_count));
//	  printf("myClassFactory::server_count = %ld", server_count);
    }

  return hr;
}

static CRITICAL_SECTION lk_values;     /* protects ti[] from simultaneous access by simulator() and WriteTags() */

// ***********************************************************************************************
// *** lgosha ************************************************************************************
#include <QMap>
// хеши идентификаторов и типов параметров
static QMap<uint, int> mIDs;
static QMap<uint, int> mTypes;
static loTagId *tilgosha;
static loTagValue *tvlgosha;
// ***********************************************************************************************

/**********************************************************************
 sample server initiation & simulation
 **********************************************************************/

/* OPTIONAL: show client's interests */

void activation_monitor(const loCaller *ca, int count, loTagPair *til)
{
}

/* OPTIONAL: write back to the device */

int WriteTags(const loCaller *ca,
              unsigned count, loTagPair taglist[],
              VARIANT values[], HRESULT error[], HRESULT *master, LCID lcid)
{
	EnterCriticalSection(&lk_values);

	for(int ii = 0; ii < count; ii++) {

		// only XXXXXX801 parameters is writable
		if( mIDs.key( (int)taglist[ii].tpTi ) % 1000 == 801 ) {

			VariantChangeType( &tvlgosha[(int)taglist[ii].tpTi].tvValue, &values[ii], 0, V_VT(&tvlgosha[(int)taglist[ii].tpTi].tvValue) );

			DEBUG_MSG_INFO1( QString("write to %1 - %2").arg( mIDs.key( (int)taglist[ii].tpTi ) ).arg(tvlgosha[(int)taglist[ii].tpTi].tvValue.iVal ) );
			
			if( tvlgosha[(int)taglist[ii].tpTi].tvValue.iVal == 2 ) {
				OPCServer::m_ptrmapParameters->value( mIDs.key( (int)taglist[ii].tpTi ) )->setValue( "2" );
				OPCServer::m_ptrmapParameters->value( mIDs.key( (int)taglist[ii].tpTi ) )->lock();
				DEBUG_MSG_INFO1( QString("%1 is now locked").arg( OPCServer::m_ptrmapParameters->value( mIDs.key( (int)taglist[ii].tpTi ) )->getStrId() ) );
				QStringList lstr;
				OPCServer::m_ptrmapParameters->value( mIDs.key( (int)taglist[ii].tpTi ) )->print( lstr );
				DEBUG_MSG_INFO1( lstr );
			}

			if( tvlgosha[(int)taglist[ii].tpTi].tvValue.iVal == 3 ) {
				OPCServer::m_ptrmapParameters->value( mIDs.key( (int)taglist[ii].tpTi ) )->setValue( "3" );
				DEBUG_MSG_INFO1( QString("%1 is now unlocked").arg( OPCServer::m_ptrmapParameters->value( mIDs.key( (int)taglist[ii].tpTi ) )->getStrId() ) );
			}

			taglist[ii].tpTi = 0;
		}
	}

	LeaveCriticalSection(&lk_values);

	return loDW_TOCACHE; /* put to the cache all tags unhandled here */
}

/* OPTIONAL: example of non-trivial datatype conversion */

static void local_text(WCHAR buf[32], unsigned nn, LCID lcid);

void ConvertTags(const loCaller *ca,
                 unsigned count, const loTagPair taglist[],
                 VARIANT *values, WORD *qualities, HRESULT *errs,
                 HRESULT *master_err, HRESULT *master_qual,
                 const VARIANT src[], const VARTYPE vtypes[], LCID lcid)
{
}

/* OPTIONAL: example of DS_DEVICE read(). It enforces significant delay, 
  sufficient for view request's queueing and test the cancellation. */

loTrid ReadTags(const loCaller *ca,
                unsigned count, loTagPair taglist[],
                VARIANT *values, WORD *qualities,
                FILETIME *stamps, HRESULT *errs,
                HRESULT *master_err, HRESULT *master_qual,
                const VARTYPE vtypes[], LCID lcid)
{
  return loDR_CACHED; /* perform actual reading from cache */
}

/* OPTIONAL: example of dynamic tag creation */

HRESULT AskItemID(const loCaller *ca, loTagId *ti, 
                  void **acpa, const loWchar *itemid, 
                  const loWchar *accpath, int vartype, int goal)  /* Dynamic tags */
{
  HRESULT hr = S_OK;
  VARIANT var;
  VariantInit(&var);  
  V_R4(&var) = -1;
  V_VT(&var) = VT_R4;

//  UL_NOTICE((LOGID, "AskItemID %ls type = %u(0x%x)", itemid, vartype, vartype));

  if (VT_EMPTY != vartype) /* check conversion */
    hr = VariantChangeType(&var, &var, 0, vartype);

  if (S_OK == hr) /* we got a value of requested type */
    {
      int rv;
      rv =
        loAddRealTag_aW(ca->ca_se, ti, 0, itemid, 0,
                        OPC_READABLE | OPC_WRITEABLE, &var, 0, 100);
      if (rv)
        {
          if (rv == EEXIST) *ti = 0; /* Already there? - Use existing one! */
          else hr = E_OUTOFMEMORY;
        }
    }
  VariantClear(&var);
  return hr;
}

/******* ******* ******* ******* ******* ******* ******* *******/

// ***********************************************************************************************
// *** lgosha ************************************************************************************
static int TI_MAXlgosha;
// ***********************************************************************************************

int driver_init(int lflags)
{
  loDriver ld;
  VARIANT var;
  loTagId tti;
  int ecode;

// ***********************************************************************************************
// *** lgosha ************************************************************************************
// *** заполнение хеша идентификаторов параметров из базы
	int nCount = 0;
	foreach( uint key, OPCServer::m_ptrmapParameters->keys() ) {
			mIDs[key] = ++nCount;
			if( OPCServer::m_ptrmapParameters->value(key)->getValueType() == eVT_Int )		mTypes[key] = 0;
			if( OPCServer::m_ptrmapParameters->value(key)->getValueType() == eVT_Short )	mTypes[key] = 3;
			if( OPCServer::m_ptrmapParameters->value(key)->getValueType() == eVT_Double )	mTypes[key] = 1;
			if( OPCServer::m_ptrmapParameters->value(key)->getValueType() == eVT_String )	mTypes[key] = 2;
	}
	// ***
	// максимальное количество типов параметров
	TI_MAXlgosha = mTypes.size() ;
	// массивы OPC-сервера
	tilgosha = new loTagId[TI_MAXlgosha + 1];
	tvlgosha = new loTagValue[TI_MAXlgosha + 1];
	// ***
// ***********************************************************************************************

  setlocale(LC_CTYPE, "");

  if (my_service) 
    {
 //     UL_ERROR((LOGID, "Driver already initialized!"));
      return 0;
    }

  memset(&ld, 0, sizeof(ld));   /* basic server parameters: */
//  ld.ldRefreshRate = 3;//10;
  ld.ldSubscribe = activation_monitor;
  ld.ldWriteTags = WriteTags;
//  ld.ldReadTags = ReadTags;
  ld.ldConvertTags = ConvertTags;
#if 0
  ld.ldAskItemID = AskItemID;
#endif
  ld.ldFlags = lflags | loDF_IGNCASE |  /*loDf_FREEMARSH | loDf_BOTHMODEL | */
    /*loDF_NOCOMP| */ loDf_NOFORCE & 0 /*| loDF_SUBSCRIBE_RAW*/;
    /*Fix the Bug in ProTool *//*|loDF_IGNCASE */ ;
  ld.ldBranchSep = '/'; /* Hierarchial branch separator */

  ecode = loServiceCreate(&my_service, &ld, 5000 /* number of tags in the cache */);
                                               /* 500000 is ok too */ 
//  UL_TRACE((LOGID, "%!e loCreate()=", ecode));
  if (ecode) return -1;

  InitializeCriticalSection(&lk_values);
  memset(tvlgosha, 0, sizeof(tvlgosha));    /* instead of VariantInit() for each of them */

  VariantInit(&var);

// ***********************************************************************************************
// *** lgosha ************************************************************************************
// *** создание параметров в OPC-сервере
	
	foreach(uint key, mIDs.keys() ) {
	
		switch(mTypes[key]) {
		case 0:
			V_VT(&var) = VT_UI4;
			ecode = loAddRealTag(my_service, &tilgosha[mIDs.value(key)], (loRealTag)mIDs.value(key),
							OPCServer::m_ptrmapParameters->value(key)->getStrId().toAscii().constData(),
							0, OPC_READABLE | OPC_WRITEABLE, &var, 0, 0);
		break;
		case 1:
			V_VT(&var) = VT_R8;
			ecode = loAddRealTag_a(my_service, &tilgosha[mIDs.value(key)],(loRealTag)mIDs.value(key), 
								OPCServer::m_ptrmapParameters->value(key)->getStrId().toAscii().constData(),
								0, OPC_READABLE | OPC_WRITEABLE, &var, 0, 0);
		break;
		case 2:
			V_VT(&tvlgosha[mIDs.value(key)].tvValue) = VT_BSTR;
			ecode = loAddRealTag(my_service, &tilgosha[mIDs.value(key)],(loRealTag)mIDs.value(key),
							OPCServer::m_ptrmapParameters->value(key)->getStrId().toAscii().constData(),     
							0, OPC_READABLE | OPC_WRITEABLE, &tvlgosha[mIDs.value(key)].tvValue, 0, 0); 
		break;
		case 3:
			V_VT(&var) = VT_UI2;
			ecode = loAddRealTag(my_service, &tilgosha[mIDs.value(key)], (loRealTag)mIDs.value(key),
							OPCServer::m_ptrmapParameters->value(key)->getStrId().toAscii().constData(),
							0, OPC_READABLE | OPC_WRITEABLE, &var, 0, 0);
		break;
		}
	}
// ***
// ***********************************************************************************************

  return 0;
}

void driver_destroy(void)
{
  if (my_service)
    {
      int ecode = loServiceDestroy(my_service);
 //     UL_INFO((LOGID, "%!e loDelete(%p) = ", ecode));

      for(ecode = 0; ecode < sizeof(tvlgosha) / sizeof(tvlgosha[0]); ecode++)
        VariantClear(&tvlgosha[ecode].tvValue);

      DeleteCriticalSection(&lk_values);

      my_service = 0;
    }
}

/********* Data simulator stuff ************************************************/

// ***********************************************************************************************
// *** lgosha ************************************************************************************
// *** функция преобразования метки времени из QDateTime в FILETIME

#include "windef.h" // для FILETIME
#include "QtCore/QDateTime" // для QDateTime

FILETIME toFILETIME( const QDateTime &datetime ) {
	
	FILETIME ft;

	SYSTEMTIME st;
	st.wYear			= datetime.date().year();
	st.wMonth			= datetime.date().month();
	st.wDay				= datetime.date().day();
	st.wHour			= datetime.time().hour();
	st.wMinute			= datetime.time().minute();
	st.wSecond			= datetime.time().second();
	st.wMilliseconds	= datetime.time().msec();

	SystemTimeToFileTime(&st, &ft);

	return ft;
}
// ***
// ***********************************************************************************************

#include <QFile>

void simulate(unsigned pause)
{
  DWORD hitime = 0;
  unsigned starttime = GetTickCount();
//  UL_WARNING((LOGID, "Simulator Started..."));

  EnterCriticalSection(&lk_values);

// ***********************************************************************************************
// *** lgosha ************************************************************************************
// *** первичная инициализация параметров

	foreach(uint key, mIDs.keys() ) {

		tvlgosha[mIDs.value(key)].tvTi = tilgosha[mIDs.value(key)];
		tvlgosha[mIDs.value(key)].tvState.tsError = S_OK;
		tvlgosha[mIDs.value(key)].tvState.tsQuality = OPC_QUALITY_UNCERTAIN;

		switch(mTypes[key]) {
		case 0:
			V_VT(&tvlgosha[mIDs.value(key)].tvValue) = VT_UI4;
			V_UI4(&tvlgosha[mIDs.value(key)].tvValue) = 0;
		break;
		case 1:
			V_VT(&tvlgosha[mIDs.value(key)].tvValue) = VT_R8;
			V_R8(&tvlgosha[mIDs.value(key)].tvValue) = 0.0;
		break;
		case 3:
			V_VT(&tvlgosha[mIDs.value(key)].tvValue) = VT_UI2;
			V_UI2(&tvlgosha[mIDs.value(key)].tvValue) = 0;
		break;
		}
	}	
// ***
// ***********************************************************************************************

	LeaveCriticalSection(&lk_values);

/**** Then do simulate ***********/

  bool m_bStop = false;

  while( 0 != my_CF.server_inuse ) {

	  FILETIME ft;

      Sleep(100);   /* main delay */
      GetSystemTimeAsFileTime(&ft); /* awoke */

      unsigned tnow = GetTickCount();
	  if (tnow - starttime >= 1000 && !QFile::exists( QString("%1/dsc.lock").arg( OPCServer::m_sWDir ) ) )
			my_CF.serverRemove();

/* OPTIONAL: reload log's configuration fromtime to time */
      if (hitime != (ft.dwLowDateTime & 0xf8000000))    /* 13.5 sec */
      {                       /* 0xff000000 is about 1.67 sec */
          hitime = ft.dwLowDateTime & 0xf8000000;
      }

/***** The main job: update the values ******/
      EnterCriticalSection(&lk_values);

// ***********************************************************************************************
// *** lgosha ************************************************************************************
// *** обновление параметров

	QStringList lstr; lstr << QString::number( OPCServer::m_ptrlistToSend->size() );

	WCHAR wstr[256];
	QList<CParameter*>::iterator p;

	for( p=OPCServer::m_ptrlistToSend->begin(); p!=OPCServer::m_ptrlistToSend->end(); ++p ) {

		(*p)->print( lstr );

		if( (*p)->status() == CParameter::ePUS_Unknown ) {
			tvlgosha[mIDs[(*p)->getId()]].tvState.tsError = S_OK;
			tvlgosha[mIDs[(*p)->getId()]].tvState.tsQuality = OPC_QUALITY_UNCERTAIN;
		}

		if( (*p)->status() == CParameter::ePUS_NotUpdated ) {
			tvlgosha[mIDs[(*p)->getId()]].tvState.tsError = S_OK;
			tvlgosha[mIDs[(*p)->getId()]].tvState.tsQuality = OPC_QUALITY_BAD;
		}

		if((*p)->status() == CParameter::ePUS_Updated ) {

			tvlgosha[mIDs[(*p)->getId()]].tvState.tsError = S_OK;
			tvlgosha[mIDs[(*p)->getId()]].tvState.tsQuality = OPC_QUALITY_GOOD;

			switch(mTypes[(*p)->getId()]) {
				case 0:
					V_UI4(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = atoi((*p)->getStringValue().toAscii().constData());
					V_VT(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = VT_UI4;
					tvlgosha[mIDs[(*p)->getId()]].tvState.tsTime = toFILETIME((*p)->getTime());
				break;
				case 1:
					V_R8(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = atof((*p)->getStringValue().toAscii().constData());
					V_VT(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = VT_R8;
					tvlgosha[mIDs[(*p)->getId()]].tvState.tsTime = toFILETIME((*p)->getTime());
				break;
				case 2:
					MultiByteToWideChar(CP_ACP, 0, (*p)->getStringValue().toAscii().constData(), -1, wstr, 256);
					VariantClear(&tvlgosha[mIDs[(*p)->getId()]].tvValue);
					V_BSTR(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = SysAllocString(wstr);
					V_VT(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = VT_BSTR;
					tvlgosha[mIDs[(*p)->getId()]].tvState.tsTime = toFILETIME((*p)->getTime());
				break;
				case 3:
					V_UI2(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = atoi((*p)->getStringValue().toAscii().constData());
					V_VT(&tvlgosha[mIDs[(*p)->getId()]].tvValue) = VT_UI2;
					tvlgosha[mIDs[(*p)->getId()]].tvState.tsTime = toFILETIME((*p)->getTime());
				break;
			}
		}

		(*p)->resetToOPC();
		p = OPCServer::m_ptrlistToSend->erase( p ); --p;
	}

	if( OPCServer::m_pLog ) DEBUG_MSG_INFO1( lstr );

// ***
// ***********************************************************************************************

/** MANDATORY: send all the values into the cache: */
      loCacheUpdate(my_service, TI_MAXlgosha, tvlgosha + 1, 0);

      LeaveCriticalSection(&lk_values);

      if( pause ) 
      {
/* IMPORTANT: force unloading of an out-of-proc 
   if not connected during PAUSE millisec */ 
          unsigned tnow = GetTickCount();
          if (tnow - starttime >= pause)
		  {
              pause = 0;
              my_CF.serverRemove();
           }
      }
  } /* end of loop */

  if (0 == my_CF.is_out_of_proc) /* For in-proc servers only! */
    {
      EnterCriticalSection(&my_CF.lk_count);
      driver_destroy();
      LeaveCriticalSection(&my_CF.lk_count);
    }
  
//  UL_MESSAGE((LOGID, "All clean. exiting..."));
}

const char eClsidName[] = "dsc";
const char eProgID[] = "OPC.dsc";

HMODULE server_module = 0;
static int mymine(HINSTANCE hInstance, int argc, char *argv[]);

int OPCServer::mymine(HINSTANCE hInstance, int argc, char *argv[])
{
  int main_rc = 0;
  DWORD objid;
  int daemon = 0, pause = 20;
  const char *exit_msg = "Exiting...";
  server_module = hInstance;

//  log = unilog_Create("DIAG-exe", "|DIAG-exe", "%!T", -1,       /* Max filesize: -1 unlimited, -2 -don't change */
 //                     ll_MESSAGE);        /* level [ll_FATAL...ll_DEBUG] */
//  unilog_Redirect("Diag-exe", "DiagOPC", 0);
//  unilog_Delete(log);
 // log = unilog_Create("Diagnostic-exe", "|Diagnostic-exe", "", -1,    /* Max filesize: -1 unlimited, -2 -don't change */
 //                     ll_TRACE);        /* level [ll_FATAL...ll_DEBUG] */
//  UL_DEBUG((LOGID, "WinMain(%s) invoked...", argv[0]));

  if (argv[1])
    {
//      printf("%s\nClass %s :  ", argv[0], eProgID);
      if (strstr(argv[1], "/r"))
        {
          char np[FILENAME_MAX + 32];
 //         printf("Registering");
		  DEBUG_MSG_INFO1( QString("register") );
          GetModuleFileName(NULL, (LPCH)(np + 1), sizeof(np) - 8);
          np[0] = '"'; strcat(np, "\"");
          
          if (strstr(argv[1], "/r")[2] == 'c') strcat(np, " /c");

          if (loServerRegister(&CLSID_DSCOPCServer, eProgID, eClsidName, np, 0))
            {
	 		    DEBUG_MSG_ERR( QString("failed") );
 //             UL_ERROR((LOGID, "%!L Reg <%s> <%s> Failed", eProgID, argv[0]));
//              puts(" FAILED");
              main_rc = 1;
            }
          else
            {
	 		    DEBUG_MSG_GOOD( QString("success") );
 //             UL_INFO((LOGID, "Reg <%s> <%s> Ok", eProgID, argv[0]));
//              puts(" Succeeded");
            }
          goto Finish;
        }
      else if (strstr(argv[1], "/u"))
        {
		  DEBUG_MSG_INFO1( QString("unregister") );
//          printf("UnRegistering");
          if (loServerUnregister(&CLSID_DSCOPCServer, eProgID))
            {
	 		    DEBUG_MSG_ERR( QString("failed") );
//              UL_WARNING((LOGID, "%!L UnReg <%s> <%s> Failed", eProgID, argv[0]));
//              puts(" FAILED");
              main_rc = 1;
            }
          else
            {
	 		    DEBUG_MSG_GOOD( QString("success") );
 //             UL_DEBUG((LOGID, "UnReg <%s> <%s> Ok", eProgID, argv[0]));
//              puts(" Success");
            }
          goto Finish;
        }
      else if (strstr(argv[1], "/detach"))
        {
		  DEBUG_MSG_INFO1( "start report file" );
//          UL_MESSAGE((LOGID, "Running as Detached DAEMON"));
//          puts("Running as Detached DAEMON...");
          daemon = 2;
          goto Cont;
        }
      else
        {
 //         UL_WARNING((LOGID, "unknown option <%s>", argv[1]));
          //printf("Unknown option <%s>", argv[1]);
          //puts("\nOne of following flags can be specified:"
          //     "\n\t/r       - register;"
          //     "\n\t/u       - unregister;"
          //     "\n\t/detach  - run in unbreakable daemon mode.");
          goto Cont;
        }
      goto Finish;
    }

Cont: /* Attempt to start the server */
  FreeConsole();

  my_CF.is_out_of_proc = my_CF.server_inuse = 1;

  if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
 //     exit_msg = "CoInitializeEx() failed. Exiting...";
 //     UL_ERROR((LOGID, exit_msg));
      goto Finish;
    }

  pause = 20; /* 20 sec can be too short for remote connections */

  if (driver_init(0))
    {
 //     exit_msg = "driver_init() failed. Exiting...";
 //     UL_ERROR((LOGID, exit_msg));
    }
  else if (FAILED(CoRegisterClassObject(CLSID_DSCOPCServer, &my_CF,
                                        CLSCTX_LOCAL_SERVER |
                                        CLSCTX_REMOTE_SERVER |
                                        CLSCTX_INPROC_SERVER,
                                        REGCLS_MULTIPLEUSE, &objid)))
    {
 //     exit_msg = "CoRegisterClassObject() failed. Exiting...";
 //     UL_ERROR((LOGID, exit_msg));
    }
  else
    {
      if (daemon) pause = 0;          // infinite

      my_CF.serverAdd(); /* Oops. This will prewent our server from unloading
                till PAUSE elapsed and simulate() do my_CF.serverRemove() */
/* STARTing the simulator */
      simulate(pause * 1000); /* sec -> millisec; */ 
/* FINISHED */
      //if (FAILED(CoRevokeClassObject(objid)))
      //  UL_WARNING((LOGID, "CoRevokeClassObject() failed..."));
    }
  driver_destroy();
  CoUninitialize();

  DEBUG_MSG_INFO1( "stop report file" );

Finish:

	return main_rc;
}

static void local_text(WCHAR buf[32], unsigned nn, LCID lcid)
{
  char sbt[40];
  long lcp = CP_ACP;
  unsigned nx;

  nn %= 7;

  if (0 == GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, sbt, sizeof(sbt)-1))
    goto Failure;
  lcp = strtoul(sbt, 0, 10);

  /* How does called "Monday"+nn in LCID country? */
  if (0 == GetLocaleInfoA(lcid, LOCALE_SDAYNAME1 + nn, sbt, sizeof(sbt) - 1))
    goto Failure;

  nx = strlen(sbt);
  if (sizeof(sbt) - nx > 12)
    {
      sbt[nx++] = ' '; /* Append language name [OPTIONAL] */
      if (0 == GetLocaleInfoA(lcid, LOCALE_SENGLANGUAGE, sbt + nx,
                         sizeof(sbt) - nx - 1) && 
          0 == GetLocaleInfoA(lcid, LOCALE_SENGCOUNTRY, sbt + nx,
                         sizeof(sbt) - nx - 1) )
        sbt[--nx] = 0; /* ... or the country name */
    }
  if (0 == MultiByteToWideChar(lcp, 0, sbt, -1, buf, 32))
    {
    Failure:
      swprintf(buf, L"%d [string]", nn);
    }
}

ULog::EMsgType OPCServer::CMsgTypeToULogType( int iMsgType ) {

	switch(iMsgType) {
		case   CMsg::eMT_Error:
		return ULog::eMT_Error;
		case   CMsg::eMT_Warning:
		return ULog::eMT_Warning;
		case   CMsg::eMT_Success:
		return ULog::eMT_Success;
		case   CMsg::eMT_Info1:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info2:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info3:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info4:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info5:
		return ULog::eMT_Info;
	}

	return ULog::eMT_Info;
}

void OPCServer::sendMessage(const QStringList &sList, const QDateTime &oDateTime, int iType) {
	// если тип сообщения попадает в диапазон уровня отладки
	if( iType <= m_iDebugLevel && m_pLog )
		// посылаем сообщение
		m_pLog->addD( sList, oDateTime, CMsgTypeToULogType(iType) );
}
