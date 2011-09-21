//<codepage = utf8
#include <QtNetwork>

#include "udp_server.h"
#include "cevent.h"
#include "parameter_abstract.h"
#include "xmlconfig.h"

extern QMap<ulong, CParameter*> qmapParameters;
extern QMap<ulong, CParameter*> qmapLink;

// отладочные функции
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_MSG_INFO(X, Y) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), Y)

#define DEBUG_MSG_INFO1(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Info1)

#define DEBUG_MSG_GOOD(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Success)

#define DEBUG_MSG_WARN(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Warning)

#define DEBUG_MSG_ERR(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Error)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// инициализация статических переменных
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QMap<ulong, CParameter*> * 		UDPServer::m_ptrmapParameters	= 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UDPServer::UDPServer(XMLConfig *pXMLConfig, const QString &filename) : 
	QThread				( 0 ),
	m_pobjUDPInSocket	( 0 ),
	m_pobjUDPOutSocket	( 0 ),
	m_uiInPort			( pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network_s", ""), "port_r").toInt() ),
	m_uiOutPort			( pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network_s", ""), "port_t").toInt() ),
	m_iDebugLevel		( pXMLConfig->gattr("debug:level").toInt() ),
	m_usSId				( pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network_s", ""), "id").toInt() ),
	m_bStop				( false ),
	m_uiUpdate			( pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network_s", ""), "updt").toUInt() ),
	m_pLog				( !filename.isNull() ? new ULog( filename, "cp1251" ) : 0 )
{
	DEBUG_MSG_INFO("create...", CMsg::eMT_Info1);

	QStringList lstr; lstr << "clients:";
	// сохраняем идентификаторы клиентов
	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "clients", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			ClientInfo cinfo;
			uint uiId = pXMLConfig->attr(pNode, "id").toUInt();
			cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_b")] = m_uiOutPort;
			if(pXMLConfig->isAttr( pNode, "port" ))
				cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_b")] = pXMLConfig->attr( pNode, "port" ).toUInt();
			cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_r")] = m_uiOutPort;
			if(pXMLConfig->isAttr( pNode, "port" ))
				cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_r")] = pXMLConfig->attr( pNode, "port" ).toUInt();
			m_mapClientsInfo[ uiId ] = cinfo;

			lstr << QString("%1 | %2:%3 | %4:%5")
						.arg( uiId )
						.arg(pXMLConfig->attr(pNode, "addr_b"), 15)
						.arg(m_mapClientsInfo[uiId].m_mapAddrs[pXMLConfig->attr(pNode, "addr_b")], 5)
						.arg(pXMLConfig->attr(pNode, "addr_r"), 15)
						.arg(m_mapClientsInfo[uiId].m_mapAddrs[pXMLConfig->attr(pNode, "addr_r")], 5);
		}
		pNode = pXMLConfig->next();
	}
	DEBUG_MSG_INFO(lstr, CMsg::eMT_Info1);

//	connect(m_pobjUDPInSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

	QMap<ulong, CParameter*>::iterator p, e = qmapParameters.end();
	for(p=qmapParameters.begin();p!=e;++p)
		m_mapStatus[p.value()] = (unsigned int)CParameter::ePUS_Unknown;
}

UDPServer::~UDPServer() {

	if( m_pLog ) delete m_pLog;
}

int UDPServer::getAnswer(pAnswer & _pa, unsigned char _type, uint uiId, const QString & _ip)
{
	if( !m_mapClientsInfo[uiId].m_mapAddrs.contains( _ip ) ) {
		DEBUG_MSG_INFO(QString("received query from unknown client: %1").arg(uiId, 5), CMsg::eMT_Info1);
		return 1;
	}

	QMap<ulong, CParameter*>::iterator p, e = qmapParameters.end();
	switch(_type) {
	case 1:		// send modified vars
	{	
		for(p=qmapParameters.begin();p!=e;++p)
			if( p.value()->isNeedSend( uiId ) ) {
				p.value()->toSend(_pa); p.value()->resetSend( uiId );
			}
	}
	case 2:		// send modified vars
	{	
		QMap<ulong, CParameter*>::iterator p1, e1 = qmapLink.end();
		for(p1=qmapLink.begin();p1!=e1;++p1) {
			if( p1.value()->isNeedSend( uiId ) ) {
				p1.value()->toSend( _pa ); p1.value()->resetSend( uiId );
			}
		}
	}
	break;
	case 100:	// send echo
	break;
	case 200:	// send link parameters
	{
		QMap<ulong, CParameter*>::iterator p1, e1 = qmapLink.end();
		for(p1=qmapLink.begin();p1!=e1;++p1) {
 			p1.value()->toSend( _pa ); p1.value()->resetSend( uiId );
		}
	}
	break;
	case 253:	// send modified vars to viewer
	{	
		for(p=qmapParameters.begin();p!=e;++p) {
			if( p.value()->isNeedSend( uiId ) ) {
				p.value()->toSend(_pa, true); p.value()->resetSend( uiId );
			}
		}
	}
	break;
	case 254:	// send all vars to viewer
	{
		for(p=qmapParameters.begin();p!=e;++p) {
			p.value()->toSend( _pa, true ); p.value()->resetSend( uiId );
		}
	}
	break;
	case 255: // send all vars
	{
		for(p=qmapParameters.begin();p!=e;++p) {
			p.value()->toSend( _pa ); p.value()->resetSend( uiId );
		}
	}
	break;
	default:
		// unknown query type
		DEBUG_MSG_INFO(QString("function getAnswer received abnormal TYPE(%d) parameter").arg(_type), CMsg::eMT_Info1);
		return 2;
    }


	return 0;
}

void UDPServer::sendAnswer( pAnswer & _pa, QHostAddress src_host, uint id )
{
	char data[65536];
	int len;
	_pa.getDatagram(data, &len);

	QByteArray datagram(data, len);
	m_pobjUDPOutSocket->writeDatagram(datagram.data(), datagram.size(), src_host, m_mapClientsInfo[id].m_mapAddrs[ src_host.toString() ] );
}

void UDPServer::processPendingDatagrams()
{
	pQuery pq;
	QHostAddress sFromAddress;

	DEBUG_MSG_INFO(QString("has datagrams..."), CMsg::eMT_Info1);

	while (m_pobjUDPInSocket->hasPendingDatagrams()) {

		QByteArray datagram;

		datagram.resize(m_pobjUDPInSocket->pendingDatagramSize());
		m_pobjUDPInSocket->readDatagram(datagram.data(), datagram.size(), &sFromAddress);

		pq.setDatagram(datagram.data());

		QStringList lstr;
		if(pq.verifyChecksum()) {

			lstr.clear();
			lstr << QString("query  | %1 | %2").arg(sFromAddress.toString()).arg(pq.print());
			DEBUG_MSG_INFO(lstr, CMsg::eMT_Info1);

			pAnswer pa;
			pa.setID(pq.getID());
			pa.setDateTime(QDateTime::currentDateTime());
			pa.setSID( m_usSId );
			if( !getAnswer(pa, pq.getType(), pq.getIDC(), sFromAddress.toString() ) ) {

				sendAnswer( pa, sFromAddress, pq.getIDC() );

				lstr.erase(lstr.begin(), lstr.end());
				lstr << QString("answer | %1.%2 | %3").arg(sFromAddress.toString()).arg(m_mapClientsInfo[ pq.getIDC() ].m_mapAddrs[ sFromAddress.toString() ]).arg(pa.print());
				DEBUG_MSG_INFO(lstr, CMsg::eMT_Info1);

				if( pa.getVariableCount() && m_pLog )
					DEBUG_MSG_INFO( pa.print_vars(), CMsg::eMT_Info5 );
			}
		}
		else {
			DEBUG_MSG_INFO(QString().sprintf("received corrupted datagram from %s\n", sFromAddress.toString().toAscii().data()), CMsg::eMT_Info1);
		}
	}
}

void UDPServer::run() {

	m_pobjUDPInSocket = new QUdpSocket;  m_pobjUDPOutSocket = new QUdpSocket;

	DEBUG_MSG_INFO(QString("bind input port: %1").arg(m_uiInPort), CMsg::eMT_Info2);
	if(!m_pobjUDPInSocket->bind(m_uiInPort)) {
		DEBUG_MSG_INFO(QString("can't bind port: %1").arg(m_uiInPort), CMsg::eMT_Error);
		return;
	}

	// переменная для выдерживания временного такта (счетчик прошедшего времени)
	QTime time;	time.start();
	// счетчик тактов
	unsigned int uiTimeCounter = 0;
	// пока "мягко" не остановили нить 
	while(!m_bStop) {

		// перезапуск счетчика времени
		time.restart();

		// обрабатываем полученные пакеты
		if( m_pobjUDPInSocket->hasPendingDatagrams() ) processPendingDatagrams();

		// определяем затраченное время
		int iSleep = m_uiUpdate - time.elapsed();
		// превышено время такта
		if(iSleep < 0) {
			// ждать уже нечего и так время такта превышено
			iSleep = 0;
			DEBUG_MSG_WARN( "!!! update time exceeded !!!" );
		}
		// отладка
		else if( iSleep < m_uiUpdate ) DEBUG_MSG_INFO1( QString("sleep time: %1").arg(iSleep) );

		// ждем время до окончания такта                                                                                         
		msleep(iSleep);
	}
}

ULog::EMsgType UDPServer::CMsgTypeToULogType( int iMsgType ) {

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

void UDPServer::sendMessage(const QStringList &sList, const QDateTime &oDateTime, int iType) {
	// если тип сообщения попадает в диапазон уровня отладки
	if( iType <= m_iDebugLevel && m_pLog )
		// посылаем сообщение
		m_pLog->addD( sList, oDateTime, CMsgTypeToULogType(iType) );
}
