#ifndef __OPC_SERVER_H__
#define __OPC_SERVER_H__

#include <ole2.h>

#include "ulog.h"
#include "parameter_abstract.h"

class myClassFactory: public IClassFactory
{
public:
 int  is_out_of_proc, 
      server_inuse; /* go 0 when unloading initiated */
 LONG server_count;
 CRITICAL_SECTION lk_count;  /* protect server_count */

 myClassFactory(): is_out_of_proc(0), server_inuse(0), server_count(0)
   {
      InitializeCriticalSection(&lk_count);
   }
 ~myClassFactory()
   {
      DeleteCriticalSection(&lk_count);
   }

  void serverAdd(void);
  void serverRemove(void);

/* Do nothing: we're static, he-he */  
  STDMETHODIMP_(ULONG) AddRef(void) { return 1; }
  STDMETHODIMP_(ULONG) Release(void) { return 1; }

  STDMETHODIMP QueryInterface(REFIID iid, LPVOID *ppInterface)
    {
      if (ppInterface == NULL)
        return E_INVALIDARG;
      if (iid == IID_IUnknown || iid == IID_IClassFactory)
        {
//         UL_DEBUG((LOGID, "myClassFactory::QueryInterface() Ok"));
          *ppInterface = this;
          AddRef();
          return S_OK;
        }
//      UL_DEBUG((LOGID, "myClassFactory::QueryInterface() Failed"));
      *ppInterface = NULL;
      return E_NOINTERFACE;
    }

  STDMETHODIMP LockServer(BOOL fLock)
    {
//      UL_DEBUG((LOGID, "myClassFactory::LockServer(%d)", fLock));
      if (fLock) serverAdd();
      else serverRemove();
      return S_OK;
    }

  STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid,
                              LPVOID *ppvObject);

};

class OPCServer {
public:
	static int mymine( HINSTANCE hInstance, int argc, char *argv[] );

	static void setLogger( ULog *p ) { m_pLog = p; }
	static void setWDir( const QString  &dir ) { m_sWDir = dir; }
	static void setDebugLevel( int l ) { m_iDebugLevel = l; }
	//! устанавливает указатель на глобальный "массив" данных
	static void setPtrToParameters(QMap<ulong, CParameter*> &ptrmapParameters) { m_ptrmapParameters = &ptrmapParameters; }
	//! устанавливает указатель на список изменившихся параметров
	static void setPtrToModified(QList<CParameter*> &ptrlistParameters) { m_ptrlistToSend = &ptrlistParameters; }
	//! устанавливает указатель на хэш индикаторов
	/*! \brief Посылка отладочного сообщения
		\param lstr cообщение
		\param date дата и метка времени сообщения
		\param type тип сообщения
		\sa ULog для формирования однострочного или многострочного сообщения
		\sa CMsg для установки типа сообщения
	*/
	static void sendMessage(const QStringList &lstr, const QDateTime &date = QDateTime::currentDateTime(), int type = 1);
private:
	static ULog::EMsgType CMsgTypeToULogType( int );

public:
	/*! \brief указатель на глобальный "массив" данных (объектов CParameter*) */
	static QMap<ulong, CParameter*> *m_ptrmapParameters;
	/*! \brief */
	static QList<CParameter*> *m_ptrlistToSend;
	/*! \brief */
	static QString	m_sWDir;
	/*! \brief */
	static ULog *	m_pLog;
private:
	static int		m_iDebugLevel;
};


#endif
