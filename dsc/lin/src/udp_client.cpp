
#include "udp_client.h"

#include "ulog.h"
#include "xmlconfig.h"
#include "parameter_creator.h"
#include "parameter_info_creator.h"
#include "parameter_info.h"

#include <memory>

using namespace std;

// отладочные функции
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_MSG_INFO1(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Info1)

#define DEBUG_MSG_GOOD(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Success)

#define DEBUG_MSG_WARN(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Warning)

#define DEBUG_MSG_ERR(X) \
	sendMessage(QStringList(X), QDateTime::currentDateTime(), CMsg::eMT_Error)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// сокращения
#define _ulong( X, Y, Z ) CParameterProperties::toULong( X, Y, Z )

// инициализация статических переменных
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QMap<ulong, CParameter*> * 		UDPClient::m_ptrmapParameters	= 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UDPClient::UDPClient( QObject *parent, XMLConfig *pXMLConfig ) :
	QThread			( parent ),
	m_pParent		( parent ),
	m_bStop			( false ),
	m_iDebugLevel		( pXMLConfig->gattr("debug:level").toInt() ),
	m_uiId			( pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network_c", ""), "logid").toInt() )

{
	DEBUG_MSG_INFO1( "starting client..." );

	// ports
	QStringList lstr; lstr << "ports:";
	const DOMNode *pNode = XMLConfig::find(pXMLConfig->root(), "network_c", "");
	uint uiTPort = pXMLConfig->attr(pNode, "port_t").toInt();
	m_uiRPort = pXMLConfig->attr(pNode, "port_r").toInt();
	// DEBUG
	lstr << QString("receive  port: %1").arg(m_uiRPort);
	lstr << QString("transmit port: %1").arg(uiTPort);
	DEBUG_MSG_INFO1( lstr );
	// identifier
	lstr.clear(); lstr << "identifier:";
	m_uiCId = pXMLConfig->attr(pNode, "id").toInt();
	// DEBUG
	lstr << QString("client id: %1").arg(m_uiCId, 3, 10, QChar('0'));
	DEBUG_MSG_INFO1( lstr );
	// updates
	lstr.clear(); lstr << "updates:";
	m_uiUpdate	= pXMLConfig->attr(pNode, "updt").toInt();
	m_uiTestUpdate	= pXMLConfig->attr(pNode, "lupdt").toInt();;
	m_uiVarUpdate	= pXMLConfig->attr(pNode, "vupdt").toInt();;
	m_uiTestUpdate > m_uiVarUpdate ? m_uiMaxUpdate = m_uiTestUpdate : m_uiMaxUpdate = m_uiVarUpdate;
	// DEBUG
	lstr	<< QString(" updt : %1").arg(m_uiUpdate)
		<< QString("lupdt : %1").arg(m_uiTestUpdate)
		<< QString("vupdt : %1").arg(m_uiVarUpdate)
		<< QString("mupdt : %1").arg(m_uiMaxUpdate);
	DEBUG_MSG_INFO1( lstr );
	// check update parameters
	lstr.clear(); DEBUG_MSG_INFO1( "check update parameters" );
	if( m_uiTestUpdate % m_uiUpdate || m_uiVarUpdate < m_uiTestUpdate || m_uiVarUpdate % m_uiTestUpdate ) {
		m_bStop = true;
		DEBUG_MSG_ERR( "update parameters must be: lupdt % updt = 0 && vupdt >= lupdt && vupdt % updt == 0" );
	}
	else DEBUG_MSG_GOOD( "success" );
	// servers
	lstr.clear(); lstr << "servers:";
	// перебираем все сервера из конфигурационного файла
	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "servers", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

			uint id = pXMLConfig->attr(pNode, "id").toUInt();
			CUDPServerInfo info( id, pXMLConfig->attr(pNode, "link") == "yes" ? true : false );

			info.addAddress(
					pXMLConfig->attr(pNode, "addr_b"),
					pXMLConfig->isAttr(pNode, "port") ? pXMLConfig->attr(pNode, "port").toUInt() : uiTPort,
					QString("%1").arg( m_uiCId, 2, 10, QChar('0') ).toUInt()*10 + 1
					);
			info.addAddress(
					pXMLConfig->attr(pNode, "addr_r"),
					pXMLConfig->isAttr(pNode, "port") ? pXMLConfig->attr(pNode, "port").toUInt() : uiTPort,
					QString("%1").arg( m_uiCId, 2, 10, QChar('0') ).toUInt()*10 + 2
					);
			m_mapHosts[ id ] = info;

			// DEBUG
			lstr << QString("%1 | %2 | %3")
				.arg(pXMLConfig->attr(pNode, "id"))
				.arg(pXMLConfig->attr(pNode, "addr_b"), 15)
				.arg(pXMLConfig->attr(pNode, "addr_r"), 15);
		}
		// переходим к следующему элементу
		pNode = pXMLConfig->next();
	}
	DEBUG_MSG_INFO1( lstr );
}

void UDPClient::gatherVars()
{
	pQuery pq;
	QStringList lstr; lstr << "query data:";
	QMap<uint, CUDPServerInfo>::iterator p, e =  m_mapHosts.end();
	for( p = m_mapHosts.begin();p!=e;++p ) {

		p.value().updateConnectState();

		// DEBUG servers state
		DEBUG_MSG_INFO1( p.value().print() );

		if( p.value().isValid() ) {

			pq.setID(rnd++);
			pq.setDateTime(QDate::currentDate(), QTime::currentTime());
			pq.setIDC( m_uiCId );
			switch(p.value().getServerState()) {
				case 0:	{ if( p.value().isLink() ) pq.setType( pQuery::eQT_MLink ); else pq.setType( pQuery::eQT_Modified ); } break;
				case 1:	{ if( p.value().isLink() ) pq.setType( pQuery::eQT_Link ); else pq.setType( pQuery::eQT_All ); }  break;
				case 2: { if( p.value().isLink() ) pq.setType( pQuery::eQT_Link ); else pq.setType( pQuery::eQT_All ); }  break;
				case 3:	{ if( p.value().isLink() ) pq.setType( pQuery::eQT_Link ); else pq.setType( pQuery::eQT_All ); }  break;
				case 4:	{ if( p.value().isLink() ) pq.setType( pQuery::eQT_Link ); else pq.setType( pQuery::eQT_All ); }  break;
				default:
					DEBUG_MSG_ERR( QString("incorrect server state: %1").arg(p.value().getServerState()) );
					return;
			}

			sendQuery(pq, p.value().getAddress(), p.value().getPort());
			p.value().setQueryState( pq.getID() );

			lstr << QString("query | %1.%2 | %3")
						.arg(p.value().getAddress().toString(), 15)
						.arg(p.value().getPort(), 4)
						.arg(pq.print());
		}
	}
	DEBUG_MSG_INFO1( lstr );
}

void UDPClient::testChanels()
{
	pQuery pq;
	pq.setDateTime(QDate::currentDate(), QTime::currentTime()); pq.setType( pQuery::eQT_Test );

	QStringList lstr; lstr << "link control:";
	QMap<uint, CUDPServerInfo>::iterator p, e =  m_mapHosts.end();
	for( p = m_mapHosts.begin();p!=e;++p ) {

		// update connect status for servers addresses
		p.value().updateConnectState();

		// update parameters
		m_ptrmapParameters->value( _ulong( p.value().getId(), 000, 001 ) )->
			updateValue( p.value().getServerState() == CUDPServerInfo::eSS_Available ? "1" : "0" );
		m_ptrmapParameters->value( _ulong( p.value().getId(), 000, QString("%1").arg( m_uiCId, 2, 10, QChar('0') ).toUInt()*10 + 1 ) )->
			updateValue( p.value().getConnectState( p.value().getAddressById( QString("%1").arg( m_uiCId, 2, 10, QChar('0') ).toUInt()*10 + 1 ) ) == -1 ? "1" : "0" );
		m_ptrmapParameters->value( _ulong( p.value().getId(), 000, QString("%1").arg( m_uiCId, 2, 10, QChar('0') ).toUInt()*10 + 2 ) )->
			updateValue( p.value().getConnectState( p.value().getAddressById( QString("%1").arg( m_uiCId, 2, 10, QChar('0') ).toUInt()*10 + 2 ) ) == -1 ? "1" : "0" );

		// foreach address
		foreach( CUDPServerInfo::hAddr addr, p.value().getAddreses() ) {

			pq.setID(rnd++);
			pq.setIDC( m_uiCId );

			sendQuery( pq, addr.hAddr, addr.uiPort );

			p.value().setConnectState( addr.hAddr.toString(), pq.getID() );

			lstr << QString("test  | %1.%2 | %3")
						.arg(addr.hAddr.toString(), 15)
						.arg(addr.uiPort, 4)
						.arg(pq.print());
		}
	}
	DEBUG_MSG_INFO1( lstr );
}

void UDPClient::processPendingDatagrams()
{
	pAnswer pa; pVariable pv;
	QHostAddress sFromAdress;
	quint16 port;

	DEBUG_MSG_INFO1( "process datagrams" );
	// is data
	while(m_udpRSocket->hasPendingDatagrams()) {

		pa.clear();
		QByteArray datagram;

		// read data
		datagram.resize(m_udpRSocket->pendingDatagramSize());
		m_udpRSocket->readDatagram(datagram.data(), datagram.size(), &sFromAdress, &port);

		// set datagram to answer
		pa.setDatagram(datagram.data(), datagram.size());

		// check packet
		if( pa.verifyChecksum(datagram.data()) ) {

			// find server
			if( !m_mapHosts.contains( pa.getSID() ) ) {
				DEBUG_MSG_ERR( QString("received answer from unknown server: %1").arg(pa.getSID(), 3) );
				continue;
			}

			CUDPServerInfo *pInfo = &m_mapHosts[ pa.getSID() ];

			// check for address
			if( !pInfo->isAddress( sFromAdress.toString() ) ) {
				DEBUG_MSG_ERR( QString("received answer from unknown address: %1").arg(sFromAdress.toString(), 15) );
				continue;
			}

			// is test ?
			if( pInfo->getConnectState( pa.getID() ) ) { // test

				pInfo->setConnectState( sFromAdress.toString(), -1 );

				DEBUG_MSG_INFO1	(
					QString("test  | %1 | %2")
						.arg(sFromAdress.toString(), 15)
						.arg(pa.print())
						);
				continue;
			}

			// is data ?
			if( pInfo->getQueryState( pa.getID() ) ) { // data

				// set server state
				pInfo->setServerState( CUDPServerInfo::eSS_Available );
				// update server state parameter
				m_ptrmapParameters->value( _ulong( pInfo->getId(), 000, 001 ) )->updateValue( "1" );

				// extract the vars
				for(int i = 0; i < pa.getVariableCount(); i++) {

					pv = pa.getVariable(i);
					QString sId = QString("%1%2%3")
									.arg( pv.getPtsID(),	3, 10, QChar('0') )
									.arg( pv.getDeviceID(), 3, 10, QChar('0') )
									.arg( pv.getN(),	3, 10, QChar('0') );

					if( m_ptrmapParameters->contains( sId.toUInt() )) {
						m_ptrmapParameters->value( sId.toUInt() )->setValue	( pv.getValue().toStr() );
						m_ptrmapParameters->value( sId.toUInt() )->setTime	( pv.getDateTime() );
						m_ptrmapParameters->value( sId.toUInt() )->setStatus	( static_cast<CParameter::EParStatus>(pv.getStatus()) );
					}
					else
						DEBUG_MSG_ERR( QString("received parameter with unknown id: %1").arg(sId) );
				}

				DEBUG_MSG_INFO1	(
						QString("answer | %1 | %2")
							.arg(sFromAdress.toString(), 15)
							.arg(pa.print())
						);
				if( m_pParent ) DEBUG_MSG_INFO1( pa.print_vars() );
			}
			// invalid packet identifier
			else {
				DEBUG_MSG_ERR	(
					QString("received datagram at unexpected identifier [%1] from %2")
							.arg(pa.getID())
							.arg(sFromAdress.toString())
						);
			}
		}
		else {
			DEBUG_MSG_ERR( QString("received corrupted datagram from %1").arg( sFromAdress.toString() ) );
		}
	}
}

void UDPClient::sendQuery(pQuery _pq, QHostAddress _host, unsigned int port)
{
    QByteArray datagram(_pq.getDatagram(), 14);
    m_udpTSocket->writeDatagram(datagram.data(), datagram.size(), _host, port);
}


void UDPClient::run() {

	m_udpTSocket = new QUdpSocket;  m_udpRSocket = new QUdpSocket;

	if(!m_udpRSocket->bind(m_uiRPort)) {
		m_ptrmapParameters->value( _ulong( m_uiCId, 000, 001 ) )->updateValue( "0" );
		return;
	}
	else
		m_ptrmapParameters->value( _ulong( m_uiCId, 000, 001 ) )->updateValue( "1" );

	// переменная для выдерживания временного такта (счетчик прошедшего времени)
	QTime time;	time.start();
	// счетчик тактов
	unsigned int uiTimeCounter = 0;
	// пока "мягко" не остановили нить 
	while(!m_bStop) {

		// перезапуск счетчика времени
		time.restart();
		// сбрасываем счетчик тактов, если достигнуто максимальное значение
		if(uiTimeCounter == m_uiMaxUpdate) uiTimeCounter = 0;

		processPendingDatagrams();		

		if( !(uiTimeCounter % m_uiTestUpdate) || !(uiTimeCounter % m_uiVarUpdate) )
			DEBUG_MSG_INFO1( "UPDATE..." );

		if( !(uiTimeCounter % m_uiVarUpdate) ) // запрос данных
			gatherVars();

		if( !(uiTimeCounter % m_uiTestUpdate) ) // проверка связи
			testChanels();

		// определяем затраченное время
		int iSleep = m_uiUpdate - time.elapsed();
		// превышено время такта
		if(iSleep < 0) {
			// ждать уже нечего и так время такта превышено
			iSleep = 0;
			DEBUG_MSG_WARN( "!!! update time exceeded !!!" );
		}
		// отладка
		else DEBUG_MSG_INFO1( QString("sleep time: %1").arg(iSleep) );

		// ждем время до окончания такта
		msleep(iSleep);

		// увеличиваем счетчик тактов на время такта
		uiTimeCounter += m_uiUpdate;
	}
}

void UDPClient::sendMessage(const QStringList &sList, const QDateTime &oDateTime, int iType) {
	// если тип сообщения попадает в диапазон уровня отладки
	if( iType <= m_iDebugLevel && m_pParent )
		// посылаем сообщение
		QCoreApplication::postEvent(m_pParent, new CMsg(m_uiId, oDateTime, sList, iType));
}
