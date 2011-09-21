//<codepage = utf8
/*! файлы проекта */
#include "datathread_wmi.h"
#include "cevent.h"
#include "xmlconfig.h"
#include "parameter_abstract.h"

/*! файлы Qt */
#include <QCoreApplication>

const unsigned int c_uiSleepTime = 500;

// удобные отладочные функции
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_MSG_INFO(X, Y) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), Y)

#define DEBUG_MSG_GOOD(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Success)
		
#define DEBUG_MSG_WARN(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Warning)

#define DEBUG_MSG_ERR(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Error)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// инициализация статических переменных класса CWMIThread и CWMIQuery
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QMap<ulong, CParameter*>  * CWMIThread::m_ptrmapParameters  = 0;
QObject *					CWMIThread::m_pParent			= 0;
unsigned int				CWMIThread::m_uiId				= 0;
int							CWMIThread::m_iDebugLevel		= 8; // максимальная отладка
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Определение методов класса CWMIThread::CWMIQuery
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWMIThread::CWMIQuery::CWMIQuery( CParameter *pPar, QMap<ulong, QStringList> *ptrmapErrors, QMap<ulong, bool> *ptrmapQueriesStatus ) : 
	m_pParameter			( pPar ),
	m_ptrmapErrors			( ptrmapErrors ),
	m_ptrmapQueriesStatus	( ptrmapQueriesStatus ),
	m_uiCounter				( -1 )
{ 
	m_lRef = 0; bDone = false; 
	InitializeCriticalSection(&threadLock);
}

CWMIThread::CWMIQuery::~CWMIQuery() { 

	bDone = true;
	DeleteCriticalSection(&threadLock);
			
	(*m_ptrmapQueriesStatus)[m_pParameter->getId()] = true;
}

ULONG CWMIThread::CWMIQuery::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG CWMIThread::CWMIQuery::Release()
{
	LONG lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT CWMIThread::CWMIQuery::QueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppv = (IWbemObjectSink *) this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else return E_NOINTERFACE;
}

void CWMIThread::CWMIQuery::toErrorMap(const QString &sError) {

	if( !(*m_ptrmapErrors)[m_pParameter->getId()].count() )
		(*m_ptrmapErrors)[m_pParameter->getId()] << QString("--- %1 ---").arg(m_pParameter->getId());

	(*m_ptrmapErrors)[m_pParameter->getId()] << sError;
}

int CWMIThread::CWMIQuery::valueFromVariant(VARIANT *pVARIANTVar, QString &sStringVar) {

	switch(pVARIANTVar->vt) {
		case VT_BOOL:
			sStringVar.sprintf("%d", V_BOOL(pVARIANTVar));
		break;
		case VT_BSTR:
			sStringVar.sprintf("%ls", V_BSTR(pVARIANTVar));
		break;
		case VT_ARRAY + VT_BSTR:
			sStringVar.sprintf("%ls", *((BSTR*)((SAFEARRAY*)pVARIANTVar->parray)->pvData));
		break;
		case VT_I2:
			sStringVar.sprintf("%d", V_I2(pVARIANTVar));
		break;
		case VT_I4:
			sStringVar.sprintf("%d", V_I4(pVARIANTVar));
		break;
		case VT_UI2:
			sStringVar.sprintf("%u", V_UI2(pVARIANTVar));
		break;
		case VT_UI4:
			sStringVar.sprintf("%u", V_UI4(pVARIANTVar));
		break;
		case VT_EMPTY:
			toErrorMap( QString("empty value type in query: %1").arg(m_pParameter->getAddr()) );
		break;
		case VT_NULL:
			toErrorMap( QString("null value in query: %1").arg(m_pParameter->getAddr()) );
			return 1;
		break;
		default:
			toErrorMap( 
						QString("could not determine value type: %1 in query: %2")
							.arg(pVARIANTVar->vt)
							.arg(m_pParameter->getAddr())
					   );
			return 2;
	}

	return 0;
}

//! функция вызывается в случае успешного ответа WMI на наш запрос
HRESULT CWMIThread::CWMIQuery::Indicate(long lObjectCount, IWbemClassObject **apObjArray)
{
	HRESULT hres = S_OK;

	// если в ответе есть объекты, то обнуляем ошибки по данному параметру
	if( lObjectCount )
		(*m_ptrmapErrors)[m_pParameter->getId()] = QStringList();

	// для всех объектов в ответе
	for (int i = 0; i < lObjectCount; i++)
    {
		// получаем требуемое свойство объекта
        VARIANT varValue;
		hres = apObjArray[i]->Get(_bstr_t(m_pParameter->getProperty().toAscii()), 0, &varValue, 0, 0);

		// свойство не получено
        if (FAILED(hres))
        {
			toErrorMap( QString("failed to get the data from the query: %1").arg(m_pParameter->getAddr()) );
            return WBEM_E_FAILED;
        }

		// свойство получено

		// конвертируем значение свойства в QString
		QString sTmp;
		if( valueFromVariant(&varValue, sTmp) ) return WBEM_E_FAILED;

		// устанавливаем значение параметра
		m_pParameter->setValue(sTmp);
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CWMIThread::CWMIQuery::SetStatus(
            /* [in] */ LONG lFlags,
            /* [in] */ HRESULT hResult,
            /* [in] */ BSTR strParam,
            /* [in] */ IWbemClassObject __RPC_FAR *pObjParam
        )
{
	if(lFlags == WBEM_STATUS_COMPLETE)
	{
		EnterCriticalSection(&threadLock);
		bDone = true;
		LeaveCriticalSection(&threadLock);
	}
    
    return WBEM_S_NO_ERROR;
}

bool CWMIThread::CWMIQuery::IsDone()
{
    bool done = true;

	EnterCriticalSection(&threadLock);
	done = bDone;
	LeaveCriticalSection(&threadLock);

    return done;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Определение методов класса CWMIThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CWMIThread::CWMIThread(XMLConfig *pXMLConfig, ePP_QueryType eType, uint uiUpdate, uint uiMUpdate ) :

	m_eType			( eType ),
	m_uiUpdate		( uiUpdate ),
	m_uiMaxUpdate	( uiMUpdate ),
	m_bStop			( false )
{
	DEBUG_MSG_INFO("create...", CMsg::eMT_Info1);

	QMap<ulong, CParameter*>::iterator p, e = m_ptrmapParameters->end();
	for(p=m_ptrmapParameters->begin();p!=e;++p) {
		// нас интересуют только WMI-параметры
		if( p.value()->getQueryType() == m_eType ) {
			DEBUG_MSG_INFO(QString("%1").arg(p.key()), CMsg::eMT_Info3);
		}	
	}

	DEBUG_MSG_INFO("done", CMsg::eMT_Info1);
}

CWMIThread::~CWMIThread() {
}

void CWMIThread::sendMessage(const QStringList &sList, const QDateTime &oDateTime, int iType) {
	// если тип сообщения попадает в диапазон уровня отладки
	if(iType <= m_iDebugLevel)
		// посылаем сообщение
		QCoreApplication::postEvent(m_pParent, new CMsg(m_uiId, oDateTime, sList, iType));
}

bool CWMIThread::WMI_init() {\

	// --- инициализация ---
    if( CoInitializeEx(0, COINIT_MULTITHREADED) < 0 )  {
		DEBUG_MSG_ERR("failed to initialize COM library");
        return false; 
    }
/*
	// --- установка уровня защиты ---
	if( CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL) < 0 ) {
		DEBUG_MSG_ERR("failed to initialize security");
        CoUninitialize();
        return false;
    }
*/
	// --- получение WMI locator ---
	if( CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &m_pLoc) < 0 ) {
		DEBUG_MSG_ERR("failed to create IWbemLocator object");
        CoUninitialize();
        return false;
    }
	// --- подключение к WMI ---
	if( m_pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &m_pSvc) < 0 ) {  
		DEBUG_MSG_ERR("could not connect to WMI");
        m_pLoc->Release();     
        CoUninitialize();
        return false;
    }
	// --- установка уровня защиты proxy ---
	if( CoSetProxyBlanket(m_pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE) < 0 ) {
		DEBUG_MSG_ERR("could not set proxy blanket");
        m_pSvc->Release();
        m_pLoc->Release();     
        CoUninitialize();
        return false;
    }

	return true;
}

bool CWMIThread::WMI_query(CParameter *pPar, QMap<ulong, QStringList> &mapErrors, QMap<ulong, bool> &mapQueryesStatus) {

	// добавляем в хеш ошибок сообщение об ошибке для данного параметра
	// это сообщение будет удалено, если получен ответ 
	// или заменено, если в процессе обработки ответа произойдет ошибка
	QStringList lstr;
	lstr	<< QString("--- %1 ---").arg(pPar->getId())
			<< QString("no objects for query: %1").arg(pPar->getAddr());
	mapErrors[pPar->getId()] = lstr;
	// устанавливаем в хеше состояния запросов, что запрос данного параметра находится в процессе выполнения
	mapQueryesStatus[pPar->getId()] = false;
	// создаем объект результата запроса
	CWMIQuery* pResponse = new CWMIQuery(pPar, &mapErrors, &mapQueryesStatus);
	// выполняем запрос
	if( m_pSvc->ExecQueryAsync(bstr_t("WQL"), bstr_t(pPar->getAddr().toAscii()), WBEM_FLAG_BIDIRECTIONAL, NULL, pResponse) < 0 ) {
			DEBUG_MSG_ERR( QString("Query of %1 failed").arg(pPar->getAddr()) );
			pResponse->Release();
			return false;
	}

	return true;
}

bool CWMIThread::WMI_wait(const QMap<ulong, bool> &mapQueryesStatus) const {

	bool bTimeout = false; int iTimeCounter = 0;
	// no comments ;-)
	while( true ) {
		// если запросы не выполнены за время такта, прерываем ожидание
		// !!! это очень, очень плохо !!!
		// после этого превышения программа обычно вылетает, так как остаются не уничтоженные объеты CWMIQuery
		// необходимо увеличивать время такта, чтобы этого не допустить
		if(iTimeCounter > m_uiUpdate) { bTimeout = true; break; }
		
		// перебираем все состояния запросов
		int iCounter = 0;
		QMap<ulong, bool>::const_iterator p, e = mapQueryesStatus.end();
		for(p=mapQueryesStatus.begin();p!=e;++p)
			// если выполнился увеличиваем счетчик
			if(p.value()) iCounter++;

		// выполнились все запросы - ожидание окончено
		if(iCounter == mapQueryesStatus.size()) break;

		// спим до следующей проверки
		msleep(c_uiSleepTime);

		// увеличиваем счетчик времени
		iTimeCounter += c_uiSleepTime;
	}

	// ожидание прервано, время такта превышено
	if(bTimeout) return false;

	// все OK
	return true;
}

void CWMIThread::WMI_check(const QMap<ulong, QStringList> &mapErrors) const {

	QStringList lstr; lstr << "--- WARNING ---";
	QMap<ulong, QStringList>::const_iterator mp, me = mapErrors.end();
	bool bGood = true;
	for(mp=mapErrors.begin();mp!=me;++mp) {
		if( mp.value().size() ) { 			
			lstr << mp.value();
			bGood = false;
		}
	}
	if(!bGood) DEBUG_MSG_WARN(lstr);
}

void CWMIThread::run() {

	DEBUG_MSG_INFO("started...",  CMsg::eMT_Info1);
	
	// инициализация и подключение к WMI
	if( !WMI_init() ) return;

	// переменная для выдерживания временного такта (счетчик прошедшего времени)
	QTime time;	time.start();
	// счетчик тактов
	uint uiTimeCounter = 0;
	// пока "мягко" не остановили нить 
	while(!m_bStop) {
		// перезапуск счетчика времени
		time.restart();
		// метка времени текущего такта
		QDateTime date = QDateTime::currentDateTime();
		// сбрасываем счетчик тактов, если достигнуто максимальное значение
		if(uiTimeCounter == m_uiMaxUpdate) uiTimeCounter = 0;
		// *** определяем параметры для обновления
		bool bUpdate = false; // флаг наличия параметров для обновления
		QStringList lstr;
		lstr << "to update";
		// последовательно перебираем параметры
		QMap<ulong, CParameter*>::iterator p, e = m_ptrmapParameters->end();
		QList<CParameter *> lToUpdate;
		for(p= m_ptrmapParameters->begin();p!=e;++p) { 
			// параметр принадлежит данной нити требует обновления на текущем такте
			if( p.value()->getQueryType() == m_eType && p.value()->isNowUpdate( uiTimeCounter ) ) {
				p.value()->setNeedUpdate( true ); lToUpdate.append( p.value() );
				bUpdate = true;
				lstr << CParameterProperties::toStr(p.key());
			}
		}
		// отладка
		if(bUpdate) DEBUG_MSG_INFO(lstr, CMsg::eMT_Info3);

		// если есть параметры для обновления 
		if(bUpdate) {

			// ***** запрос параметров *****

			// хэш ошибок при выполнении запроса
			QMap<ulong, QStringList> mapErrors;
			// хеш cостояний запроса
			QMap<ulong, bool> mapQueryesStatus;

			// *** обновление асинхронных параметров ***

			DEBUG_MSG_INFO("update async", CMsg::eMT_Info2);

			// запрос параметров
			for(p=m_ptrmapParameters->begin();p!=e;++p) {
				// только WMI и асинхронные
				if( p.value()->getQueryType() == eQT_WMI && !p.value()->isSync() ) {
					// если параметр должен был быть обновлен на этом такте
					if( p.value()->isNeedUpdate() )
						WMI_query(p.value(), mapErrors, mapQueryesStatus);
				}
			}
			// ожидание обновления параметров
			if( !WMI_wait(mapQueryesStatus) ) 
				DEBUG_MSG_WARN("wait failed during async");

			DEBUG_MSG_INFO("done", CMsg::eMT_Info3);
			// *** асинхронные параметры обновлены ***


			// *** обновление синхронных параметров ***
			DEBUG_MSG_INFO("update sync", CMsg::eMT_Info2);

			// запрос параметров
			for(p=m_ptrmapParameters->begin();p!=e;++p) {
				// только WMI и синхронные
				if( p.value()->getQueryType() == eQT_WMI && p.value()->isSync() ) {
					// если параметр должен был быть обновлен на этом такте
					if( p.value()->isNeedUpdate() ) {
						// запрос
						WMI_query(p.value(), mapErrors, mapQueryesStatus);
						// ожидание обновления параметра
						if ( !WMI_wait(mapQueryesStatus) ) {
							DEBUG_MSG_WARN("wait failed during sync");
							break;
						}
					}
				}
			}

			DEBUG_MSG_INFO("done", CMsg::eMT_Info3);
			// *** синхронные параметры обновлены ***

			// проверка на наличие ошибок во время запроса
			WMI_check(mapErrors);
			
			// *** применяем значения
			DEBUG_MSG_INFO("apply values", CMsg::eMT_Info2);
			foreach( CParameter *pPar, lToUpdate )
				if( pPar->getQueryType() == m_eType ) apply_parameter( pPar, date );

			// отладка
			{
				QStringList lstr; lstr << "dump parameters";
			for(p=m_ptrmapParameters->begin();p!=e;++p)
					/*if( p.value()->getQueryType() == m_eType )*/ p.value()->print(lstr);
			DEBUG_MSG_INFO(lstr, CMsg::eMT_Info3);
			}
		}

		// определяем затраченное время
		int iSleep = m_uiUpdate - time.elapsed();
		// превышено время такта
		// !!! плохо - не выдержали время такта - необходимо увеличить время такта !!!
		// !!! иначе мы не сможем обеспечить обновление значений параметров через заданный период времени !!! 
		if(iSleep < 0) {
			// ждать уже нечего и так время такта превышено
			iSleep = 0;
			DEBUG_MSG_WARN("!!! update time exceeded !!!");
		}
		// отладка
		else DEBUG_MSG_INFO(QString("sleep time: %1").arg(iSleep),  CMsg::eMT_Info3);

		// ждем время до окончания такта
		msleep(iSleep);

		// увеличиваем счетчик тактов на время такта
		uiTimeCounter += m_uiUpdate;		
	}

	// освобождение памяти
    m_pSvc->Release();
    m_pLoc->Release();
    CoUninitialize();
}

void CWMIThread::apply_parameter( CParameter *pPar, const QDateTime &date ) {

	// применяем новое значение как текущее
	pPar->applyValue();
	// если значение параметра обновилось, обновляем и метку времени
	if(pPar->status() == CParameter::ePUS_Updated) {
		pPar->setTime( date.toUTC() );
		pPar->setForceUpdate( false );
	}
	// снимаем флаги обновления
	pPar->setNeedUpdate( false );
}
