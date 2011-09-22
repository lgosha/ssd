
#include "udp_client.h"

#include "ulog.h"
#include "xmlconfig.h"

#include <QCoreApplication>
#include <QTimer>

#include <memory>

using namespace std;

auto_ptr<ULog> ptrMainLog = auto_ptr<ULog>( new ULog("../log/client.txt", "cp1251", 0, true) );

UDPClient::UDPClient( CMainWnd *parent, XMLConfig *pXMLConfig ) :
	m_pMainWnd( parent ),
	m_udpTSocket( new QUdpSocket ),
	m_udpRSocket( new QUdpSocket ),
	m_pTimer( new QTimer( this ) )
{
	ptrMainLog->add("starting client...");
	// ports
	QStringList lstr; lstr << "ports:";
	const DOMNode *pNode = XMLConfig::find(pXMLConfig->root(), "network", "");
	m_uiRPort = pXMLConfig->attr(pNode, "port").toInt();
	// DEBUG
	lstr << QString("receive  port: %1").arg(m_uiRPort);
	ptrMainLog->add(lstr);
	// identifier
	lstr.clear(); lstr << "identifier:";
	m_uiId = pXMLConfig->attr(pNode, "id").toInt();
	// DEBUG
	lstr << QString("client id: %1").arg(m_uiId);
	ptrMainLog->add(lstr);
	// servers
	lstr.clear(); lstr << "servers:";
	// перебираем все сервера из конфигурационного файла
	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "servers", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

			ServInfo info;
			info.sAddr = QHostAddress( pXMLConfig->attr(pNode, "addr") );
			info.uiPort = pXMLConfig->attr(pNode, "port").toUInt();
			info.bStatus = false;

			m_mapServers[pXMLConfig->attr(pNode, "id").toUInt()] = info;

			// DEBUG
			lstr << QString("%1 | %2 | %3")
				.arg(pXMLConfig->attr(pNode, "id"))
				.arg(pXMLConfig->attr(pNode, "addr"), 15)
				.arg(pXMLConfig->attr(pNode, "port"));
		}
		// переходим к следующему элементу
		pNode = pXMLConfig->next();
	}
	ptrMainLog->add(lstr);
	// динамические характеристики запросов
	m_uiUpdate = pXMLConfig->attr("dynamic:update").toUInt();
	ptrMainLog->add( QString("update time is %1").arg( m_uiUpdate ) );
	// bind port
	if(!m_udpRSocket->bind(m_uiRPort))
		ptrMainLog->add( QString("can't bind receive port %1").arg( m_uiRPort ) );
	// start update timer
	connect( m_pTimer, SIGNAL( timeout() ), this, SLOT( gatherVars() ) );\
	m_pTimer->start( m_uiUpdate );
}

void UDPClient::gatherVars()
{
	processPendingDatagrams();

	pQuery pq;
	int iQueryType = -1;

	QStringList lstr; lstr << "query data:";
	QMap<uint, ServInfo>::iterator p, e = m_mapServers.end();
	for(p = m_mapServers.begin();p!=e;++p) {

		if( !p.value().bStatus ) { 
			m_pMainWnd->setConnectStatus( p.key(), 0 );
			iQueryType = 254;
		}
		else iQueryType = 253;

		pq.setID( rnd++ );
		pq.setDateTime(QDate::currentDate(), QTime::currentTime());
		pq.setType( iQueryType );
		pq.setIDC( m_uiId );

		lstr << pq.print();

		sendQuery(pq, p.value().sAddr, p.value().uiPort);

		p.value().bStatus = false;

		lstr << QString("query | %1 | %2")
						.arg(p.value().sAddr.toString(), 15)
						.arg(p.value().uiPort);
	}
	ptrMainLog->add(lstr);
}

void UDPClient::processPendingDatagrams()
{
	ptrMainLog->add("process datagrams:");
	// is data
	while(m_udpRSocket->hasPendingDatagrams()) {

		QByteArray datagram;
		pAnswer pa; pVariable pv;
		QHostAddress sFromAdress;
		uint port;

		// read data
		datagram.resize(m_udpRSocket->pendingDatagramSize());
		m_udpRSocket->readDatagram(datagram.data(), datagram.size(), &sFromAdress, (quint16*)&port);

		// set datagram to answer
		pa.setDatagram(datagram.data(), datagram.size());

		// check packet
		if(pa.verifyChecksum(datagram.data())) {

			m_mapServers[ pa.getSID() ].bStatus = true;
			m_pMainWnd->setData( pa.getSID(), pa );
			m_pMainWnd->setConnectStatus( pa.getSID(), 1 );
			QStringList lstr;
			lstr << QString("answer from | %1 | %2 | %3")
				.arg(sFromAdress.toString(), 15)
				.arg(pa.getSID())
				.arg(pa.getVariableCount());
			ptrMainLog->add(lstr);

		}
		// invalid ckecksum
		else
			ptrMainLog->add(QString("received corrupted datagram from %1").arg(sFromAdress.toString()), ULog::eMT_Error);
	}
}

void UDPClient::sendQuery(pQuery _pq, const QHostAddress & _host, unsigned int port)
{
	QByteArray datagram(_pq.getDatagram(), 14);
	m_udpTSocket->writeDatagram(datagram.data(), datagram.size(), _host, port);
}

