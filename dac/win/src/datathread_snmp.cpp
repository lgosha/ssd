//<codepage = utf8
/*! файлы проекта */
#include "datathread_snmp.h"
#include "parameter_abstract.h"
#include "cevent.h"
#include "ulog.h"
#include "exception.h"
#include "xmlconfig.h"

/*! файлы библиотеки netsnmp */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/*! файлы библиотеки Qt */
#include <QList>
#include <QRegExp>
#include <QStringList>
#include <QCoreApplication>

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

// инициализация статических переменных класса CSNMPThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int					CSNMPThread::m_uiActiveHosts	= 0;
QMap<ulong, CParameter*> * 		CSNMPThread::m_ptrmapParameters	= 0;
QObject *						CSNMPThread::m_pParent			= 0;
unsigned int					CSNMPThread::m_uiId				= 0;
int								CSNMPThread::m_iDebugLevel		= 8; // максимальная отладка
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Определение методов класса CSNMPThread::snmphost
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSNMPThread::snmpoid* CSNMPThread::snmphost::get(bool bFirst) {
	// последовательно перебираем все OIDы SNMP-агента
	QMap<ulong, snmpoid>::iterator e = mOIDs.end();
	// перебираем либо с начала, если это запрос первого параметра у данного SNMP-агента
	// либо начиная с текущего
	for(bFirst ? p = mOIDs.begin() : p++;p!=e;++p)
		// ищем параметр, соответствующий данному OIDу, требующий обновления
		if((*m_ptrmapParameters)[p.key()]->isNeedUpdate())
			// как только находим -> прекращаем поиск
			break;
	// нашли параметр, требующий обновления
	if(p != mOIDs.end())
		return &p.value();

	// нет параметров, требующих обновления
	return 0;
}

char* CSNMPThread::snmphost::getCharPointer(const QString &str) {

	char *pBuf = new char[str.length() + 1];
	memset(pBuf, '\0', str.length() + 1);
	memcpy(pBuf, str.toAscii().constData(), str.length());

	return pBuf;
}

ulong CSNMPThread::snmphost::getCurrentOIDId() {
	
	// !!! внимание !!! это исключение нигде не перехватывается
	if(p == mOIDs.end())
		throw ex_base(ex_base::error, QString("unexpected error: no current OID: %1").arg(p.value().toPrint()));
	
	// возвращаем идентификатор параметра в главном "массиве данных", соответствующий текущему запрашиваемому OIDу
	return p.key();
}

ulong CSNMPThread::snmphost::getOIDId(const snmpoid &netsnmp_oid) {

	// последовательно перебираем все OIDы SNMP-агента
	QMap<ulong, snmpoid>::iterator p, e = mOIDs.end();
	for(p=mOIDs.begin();p!=e;++p) {
		// обязательное обновление внутреннего обновления, так как OIDы внутренние
		p.value().update_ptr();	
		//нашли среди OIDов SNMP-агента равный переданному
		if(p.value() == netsnmp_oid) break;
		// для сравнения применяется оператор из snmpoid
	}

	// !!! внимание !!! это исключение нигде не пере хватывается
	if(p == mOIDs.end()) 
		throw ex_base(ex_base::error, QString("unexpected error: no requested OID: %1").arg(netsnmp_oid.toPrint()));

	// возвращаем идентификатор параметра в главном "массиве данных", соответствующий переданному OIDу
	return p.key();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Определение методов класса CSNMPThread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSNMPThread::SOCK_START() {
	
	SOCK_STARTUP;
}

void CSNMPThread::SOCK_STOP() { 

	SOCK_CLEANUP;
}

CSNMPThread::CSNMPThread(XMLConfig *pXMLConfig, ePP_QueryType eType, unsigned int uiUpdate, uint uiMUpdate) :

	m_bStop(false),
	m_eType(eType),
	m_uiUpdate(uiUpdate),
	m_uiMaxUpdate(uiMUpdate)
{
	DEBUG_MSG_INFO("create...", CMsg::eMT_Info1);

	// инициализация netsnmp api
	init_snmp("dac");
	DEBUG_MSG_INFO("create OIDs...", CMsg::eMT_Info2);
	// *** создание SNMP OID для всех SNMP-агентов
	snmpoid objsnmpoid;
	// последовательно перебираем параметры CParameter*
	QMap<ulong, CParameter*>::const_iterator p, e = m_ptrmapParameters->end();
	for(p=m_ptrmapParameters->begin();p!=e;++p) {
		// нас интересуют только SNMP-параметры
		if(p.value()->getQueryType() == m_eType) {
			// если это первый параметр для данного SNMP-агента, создаем объект SNMP-агента
			QString sSNMPHostId = QString().sprintf("%03d%03d", p.value()->getSHWSId(), p.value()->getDevId());
			if( !qmapSNMPHosts.contains(sSNMPHostId) )
				qmapSNMPHosts[sSNMPHostId] = new snmphost;
			// создаем SNMP OID
			QStringList lstr = p.value()->getAddr().right(p.value()->getAddr().length() - 1).split(".");
			QStringList::iterator it; int iCounter = 0;
			for(it = lstr.begin(); it != lstr.end(); ++it) {
				objsnmpoid.intOID[iCounter] = (*it).toLong();
				iCounter++;
			}
			objsnmpoid.OIDlen = lstr.size();
			// добавляем к OIDам SNMP-агента
			qmapSNMPHosts[sSNMPHostId]->add(p.key(), objsnmpoid);
			DEBUG_MSG_INFO(QString("%1 | %2").arg(p.value()->getAddr(), 40).arg(p.key()), CMsg::eMT_Info3);
		}	
	}
	// *** создание SNMP сессий для всех SNMP-агентов
	DEBUG_MSG_INFO("open SNMP sessions...", CMsg::eMT_Info2);
	// перебираем все SNMP-агенты конфигурационного файла
	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "snmphosts", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			// заполняем поля структуры SNMP-сессии !!! netsnmp api !!!
			struct snmp_session s, *ps;
			snmp_sess_init(&s);
			s.peername = CSNMPThread::snmphost::getCharPointer(pXMLConfig->attr(pNode, "addr"));
			s.community = (u_char*)CSNMPThread::snmphost::getCharPointer(pXMLConfig->attr(pNode, "pass"));
			s.community_len = pXMLConfig->attr(pNode, "pass").length();
			s.version = SNMP_VERSION_1;
			s.retries = pXMLConfig->attr(pNode, "retr").toInt();
			s.timeout = pXMLConfig->attr(pNode, "timeout").toInt()*1000;
			s.callback = snmp_asynch_response;
			s.callback_magic = &qmapSNMPHosts;
			// выделяем идентификатор SNMP-агента
			// проверяем на наличие объекта описания SNMP-агента в хеше агентов
			if( !qmapSNMPHosts.contains( pXMLConfig->attr(pNode, "id") ) )
				// нет
				// это означает, что в базе нет параметров для данного агента, пропускаем
				DEBUG_MSG_WARN(
					QString("%1 | %2 | no OIDs, skipping...")
						.arg(pXMLConfig->attr(pNode, "id"), 3)
						.arg(pXMLConfig->attr(pNode, "addr"), 15));
			else {
				// есть
				// открываем сессию с агентом !!! netsnmp api !!!
				if(!(ps = snmp_open(&s))) {
					// не открыли -> выход из программы
					DEBUG_MSG_ERR(QString("can't open session for %1").arg(pXMLConfig->attr(pNode, "addr")));
					throw ex_base(ex_base::error, QString("can't open session for %1").arg(pXMLConfig->attr(pNode, "addr")));
				}
				else {
					// открыли успешно
					// сохраняем сессию в объекте описания SNMP-агента
					qmapSNMPHosts[pXMLConfig->attr(pNode, "id")]->setSession(ps);
					DEBUG_MSG_INFO(QString("%1 | %2 | %3 | %4 | %5")
						.arg(pXMLConfig->attr(pNode, "id"), 3)
						.arg(pXMLConfig->attr(pNode, "addr"),   15)
						.arg(pXMLConfig->attr(pNode, "pass"),    8)
						.arg(pXMLConfig->attr(pNode, "timeout"), 5)
						.arg(pXMLConfig->attr(pNode, "retr"),    2), CMsg::eMT_Info3);
				}
			}
		}
		// переходим к следующему SNMP-агенту
		pNode = pXMLConfig->next();
	}
	// проверяем сессии на наличие информации о SNMP-агенте
	DEBUG_MSG_INFO("check parameters...", CMsg::eMT_Info1);
	foreach( snmphost *pHost, qmapSNMPHosts.values() ) {
		// нет информации (сессия не создана)
		if( !pHost->session() ) {
			// получаем список идентификаторов параметров данного snmphost
			QStringList lstr = pHost->getParametersIds();
			// удаляем эти параметры из глобального хэша параметров и состовляем отладочное сообщение
			QStringList::iterator lp, le = lstr.end();
			QStringList lstrToPrint; lstrToPrint << "no SNMP agent information, remove parameters";
			for(lp=lstr.begin();lp!=le;++lp) {
				lstrToPrint << *lp;
				m_ptrmapParameters->remove(CParameterProperties::toULong(*lp));
			}
			// удаляем такой snmphost
			qmapSNMPHosts.remove( qmapSNMPHosts.key(pHost) );
			// выводим предупреждение
			DEBUG_MSG_WARN(lstrToPrint);
		}
	}
}

CSNMPThread::~CSNMPThread() {
	// удаление объектов описания  SNMP-агентов
	QMap<QString, snmphost*>::iterator p, e = qmapSNMPHosts.end();
	for(p=qmapSNMPHosts.begin();p!=e;++p) {
		if(p.value()->session()) snmp_close(p.value()->session());
		delete p.value();
	}
}

void CSNMPThread::sendMessage(const QStringList &sList, const QDateTime &oDateTime, int iType) {
	// если тип сообщения попадает в диапазон уровня отладки
	if( iType <= m_iDebugLevel && m_pParent )
		// посылаем сообщение
		QCoreApplication::postEvent(m_pParent, new CMsg(m_uiId, oDateTime, sList, iType));
}

void CSNMPThread::run() {

	DEBUG_MSG_INFO("started...",  CMsg::eMT_Info1);

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
			// параметр принадлежит данной нити и требует обновления на текущем такте
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

			DEBUG_MSG_INFO("update", CMsg::eMT_Info2);

			DEBUG_MSG_INFO("query parameters", CMsg::eMT_Info2);
			// обнуляем количество активных SNMP-агентов
			m_uiActiveHosts = 0;
			// последовательно перебираем все SNMP-агенты
			QMap<QString, snmphost*>::iterator mp, me = qmapSNMPHosts.end();
			for(mp=qmapSNMPHosts.begin();mp!=me;++mp) {
				// посылаем запрос первого параметра SNMP-агента
				if(!snmp_sendto(mp.value(), true)) continue;
				// если запрос послан успешно, увеличиваем количество активных SNMP-агентов
				m_uiActiveHosts++;
			}
			// после посылки запросов всем SNMP-агентам, при получении ответа, вызывается async_responce, которая в зависимости от ответа SNMP-агента
			// либо посылает запрос следующего параметра, если нечего запрашивать, то прекращает работу с этим SNMP-агентом и уменьшает количество
			// активных SNMP-агентов, либо уменьшает количество активных SNMP-агентов по таймауту

			// ожидание окончания работы со всеми SNMP-агентами (пока m_uiActiveHosts не будет равно 0)
			// эта функция также отвечает за определение таймаута
			if(!snmp_select()) {
				// при работе функции может возникнуть глобальная ошибка работы с сокетами (невозможность выполнить select)
				// останавливаем нить для разбора полетов (на моей памяти этого не было никогда)
				DEBUG_MSG_ERR("GENERAL ERROR: can't select, stopping thread...");
				terminate();
			}
			// опрос SNMP-агентов закончен успешно

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
		else DEBUG_MSG_INFO(QString("sleep time: %1 - %2").arg(iSleep).arg(uiTimeCounter),  CMsg::eMT_Info3);

		// ждем время до окончания такта
		msleep(iSleep);

		// увеличиваем счетчик тактов на время такта
		uiTimeCounter += m_uiUpdate;
	}
}

void CSNMPThread::apply_parameter( CParameter *pPar, const QDateTime &date ) {

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

int CSNMPThread::snmp_asynch_response(int operation, struct snmp_session *sp, int reqid, struct snmp_pdu *resp, void *magic) {

	// преобразовываем указатель на void* к указателю на хеш SNMP-агентов
	QMap<QString, snmphost*> *ptrlocalmapSNMPHosts = static_cast<QMap<QString, snmphost*>*>(magic);

	// находим в хеше SNMP-агентов агента от которого получен ответ
	QMap<QString, snmphost*>::iterator p, e = ptrlocalmapSNMPHosts->end();
	for(p=ptrlocalmapSNMPHosts->begin();p!=e;++p)
		if(p.value()->session() == sp) break;

	// действительно ответ
	if(operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
		// обрабатываем ответ
		snmp_parse_recv(STAT_SUCCESS, sp, resp, p.value());
		// запрашиваем следующий параметр
		if(snmp_sendto(p.value())) return 1;
	}
	// таймаут
	else snmp_parse_recv(STAT_TIMEOUT, sp, resp, p.value());

	// уменьшаем количество активных SNMP-агентов
	// сюда попадаем только если: SNMP-агент не ответил за время таймаута; нет параметров для запроса (все запросили)
	m_uiActiveHosts--;

	// ok ;-)
	return 1;
}

int CSNMPThread::snmp_sendto(snmphost *pHost, bool bFirst) {
	// запрашиваем у SNMP-агента OID соответствующий параметр которому требует обновления isNeddUpdate() = true
	snmpoid *pOID;
	if(!(pOID = pHost->get(bFirst)))
		// нет такого SNMP OID -> выходим (нечего запрашивать)
		return 0;
	// есть такой OID	
	// создаем структуру, содержащую ответ SNMP-агента
	struct snmp_pdu *req;
	req = snmp_pdu_create(SNMP_MSG_GET); // !!! netsnmp api !!!
	snmp_add_null_var(req, pOID->intOID, pOID->OIDlen); // !!! netsnmp api !!!
	// попосылаем запрос !!! netsnmp api !!!
	if(!snmp_send(pHost->session(), req)) {
		// не удалось послать
		DEBUG_MSG_ERR(QString("sendto   %1 | failed").arg(pHost->session()->peername, 15));
		snmp_free_pdu(req); // очищаем структуру
		// выходим с ошибкой
		return 0;
	}
	// запрос успешно послан
	else {
		DEBUG_MSG_INFO(
			QString("sendto   %1 | %2")
				.arg(pHost->session()->peername, 15)
				.arg((*m_ptrmapParameters)[pHost->getCurrentOIDId()]->getAddr()),

				CMsg::eMT_Info4);
	}

	// выходим с успешным завершением
	return 1;
}

int CSNMPThread::snmp_select() {

	int fds = 0, block = 0; fd_set fdset; struct timeval timeout;
	// пока есть активные SNMP-агенты
	while(m_uiActiveHosts) {
		// подготавливаем входные данные для select
		FD_ZERO( &fdset );
		snmp_select_info(&fds, &fdset, &timeout, &block); // !!! netsnmp api !!!
		// ждем данных (ответ от SNMP-агента) в течение 50 ms
		timeout.tv_sec = 0; timeout.tv_usec = 50000;
		fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
		// !!! ошибка select !!!
		if(fds < 0)	return 0;
		// если есть данные обрабатываем (передаем в async_responce)
		// если нет, значит  SNMP-агент не ответил -> генерируем таймаут 
		fds ? snmp_read(&fdset) : snmp_timeout(); // !!! netsnmp api !!!
	}
	// успешное завершение
	return 1;
}

int CSNMPThread::snmp_parse_recv(int iStatus, struct snmp_session *pSNMPSession, struct snmp_pdu *pSNMPPDU, snmphost *pSNMPHost) {

	// локальные переменные
	// !!! может не хватить буфера !!!
	char buf[1024];
	struct variable_list *vp; // !!! netsnmp api !!!
	// определяем соответствующий SNMP IOD -> параметр CParameter * в глобальном "массиве" данных
	CParameter *ppar = (*m_ptrmapParameters)[pSNMPHost->getOIDId(snmpoid(pSNMPPDU->variables->name, pSNMPPDU->variables->name_length))];
	// формальная проверка
	Q_ASSERT(ppar);
	// в зависимости от состояние ответа
	switch(iStatus) {
		// ответ от SNMP-агента получен
		case STAT_SUCCESS:
			vp = pSNMPPDU->variables; // !!! netsnmp api !!!
			// в ответе SNMP-агента нет ошибок
			if(pSNMPPDU->errstat == SNMP_ERR_NOERROR) { // !!! netsnmp api !!!
				// не очень понимаю, зачем цикл, у нас один параметр, но так в примере и главное так работает
				// вообщем перебираем переменные в ответе SNMP-агента
				while(vp) {
					// вытаскиваем значение параметра в символьный буфер 
					snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp); // !!! netsnmp api !!!
					// вырезаем лишнее
					QString sTmp = buf; sTmp = sTmp.remove(QRegExp(".*=\\s+")); sTmp = sTmp.remove(QRegExp(".*:\\s+"));
					// устанавливаем полученное значение, как новое значение параметра
					ppar->setValue(sTmp);
					// отладка
					DEBUG_MSG_INFO(
								QString("recvfrom %1 | %2 | %3")
									.arg(pSNMPSession->peername, 15)
									.arg(ppar->getAddr(), 20)
									.arg(sTmp),
									CMsg::eMT_Info4);
					// переходим к следующей переменной
					vp = vp->next_variable;
				}
			}
			// в ответе SNMP-агента ошибка
			else {
				// перебираем все ошибки
				for(int i = 1; vp && i != pSNMPPDU->errindex; vp = vp->next_variable, i++)
					DEBUG_MSG_ERR(
						QString("recvfrom %1 | %2 | %3 | %4")
							.arg(pSNMPSession->peername, 15)
							.arg(ppar->getId())
							.arg(ppar->getAddr(), 20)
							.arg(snmp_errstring(pSNMPPDU->errstat)));
				// !!! новое значение параметра не устанавливаем !!!
			}
		return 1;
		// таймаут
		case STAT_TIMEOUT: 
			DEBUG_MSG_ERR(QString("recvfrom %1 | timeout").arg(pSNMPSession->peername, 15));
		return 0;
		// неизвестная ошибка
		case STAT_ERROR:
			DEBUG_MSG_ERR(QString("recvfrom %1 | unknown error").arg(pSNMPSession->peername, 15));
		return 0;
	}
	
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
