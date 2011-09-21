//<codepage = utf8
#include <QtGui>
#include <QtNetwork>

#include "udp_server.h"
#include "cevent.h"
#include "parameter_abstract.h"
#include "xmlconfig.h"

extern QMap<ulong, CParameter*> qmapParameters;

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

UDPServer::UDPServer(XMLConfig *pXMLConfig, QObject *parent) : 
	QObject			(parent),
	m_pParent		(parent),
	m_pobjUDPInSocket	(new QUdpSocket(this)),
	m_pobjUDPOutSocket	(new QUdpSocket(this)),
	m_uiOutPort		(pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network", ""), "port_t").toInt()),
	m_iDebugLevel		(pXMLConfig->gattr("debug:level").toInt()),
	m_uiId			(pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network", ""), "logid").toInt()),
	m_usSId			(pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network", ""), "id").toInt())
{
	DEBUG_MSG_INFO("create...", CMsg::eMT_Info1);

	unsigned int m_uiInPort = pXMLConfig->attr(XMLConfig::find(pXMLConfig->root(), "network", ""), "port_r").toInt();
	DEBUG_MSG_INFO(QString("bind input port: %1").arg(m_uiInPort), CMsg::eMT_Info2);
	if(!m_pobjUDPInSocket->bind(m_uiInPort))
		DEBUG_MSG_INFO(QString("can't bind port: %1").arg(m_uiInPort), CMsg::eMT_Error);

	QStringList lstr; lstr << "clients:";
	// сохраняем идентификаторы клиентов
	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "clients", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			ClientInfo cinfo;
			cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_b")] = m_uiOutPort;
			if(pXMLConfig->isAttr( pNode, "port" ))
				cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_b")] = pXMLConfig->attr( pNode, "port" ).toUInt();
			cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_r")] = m_uiOutPort;
			if(pXMLConfig->isAttr( pNode, "port" ))
				cinfo.m_mapAddrs[pXMLConfig->attr(pNode, "addr_r")] = pXMLConfig->attr( pNode, "port" ).toUInt();
			m_mapClientsInfo[pXMLConfig->attr(pNode, "id").toUInt()] = cinfo;

			lstr << QString("%1 | %2:%3 | %4:%5")
						.arg(pXMLConfig->attr(pNode, "id").toInt())
						.arg(pXMLConfig->attr(pNode, "addr_b"), 15)
						.arg(m_mapClientsInfo[pXMLConfig->attr(pNode, "id").toUInt()].m_mapAddrs[pXMLConfig->attr(pNode, "addr_b")], 5)
						.arg(pXMLConfig->attr(pNode, "addr_r"), 15)
						.arg(m_mapClientsInfo[pXMLConfig->attr(pNode, "id").toUInt()].m_mapAddrs[pXMLConfig->attr(pNode, "addr_r")], 5);
		}
		pNode = pXMLConfig->next();
	}
	DEBUG_MSG_INFO(lstr, CMsg::eMT_Info1);

	connect(m_pobjUDPInSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
}

UDPServer::~UDPServer() {
}

int UDPServer::getAnswer(pAnswer & _pa, unsigned char _type, uint uiId, const QString & _ip)
{
	if( !m_mapClientsInfo[uiId].m_mapAddrs.contains( _ip ) ) {
		DEBUG_MSG_INFO(QString("received query from unknown client: %1.%2").arg(uiId, 5).arg(_ip, 15), CMsg::eMT_Info1);
		return 1;
	}

	QMap<ulong, CParameter*>::iterator p, e = qmapParameters.end();
	switch(_type) {
	case 1:		// send modified vars
	{	
		for(p=qmapParameters.begin();p!=e;++p) {
			if( p.value()->isNeedSend(uiId) ) {
				p.value()->toSend( _pa ); p.value()->resetNeedSend( uiId );
			}
		}
	}
	break;
	case 100:	// send echo
	break;
	case 253:	// send modified vars to viewer
	{
		for(p=qmapParameters.begin();p!=e;++p) {
			if( p.value()->isNeedSend(uiId) ) {
				p.value()->toSend( _pa, true ); p.value()->resetNeedSend( uiId );
			}
		}
	}
	break;
	case 254:	// send all vars to viewer
	{
		for(p=qmapParameters.begin();p!=e;++p) {
			p.value()->toSend( _pa, true ); p.value()->resetNeedSend( uiId );
		}
	}
	break;
	case 255:	// send all vars
	{
		for(p=qmapParameters.begin();p!=e;++p) {
			p.value()->toSend( _pa ); p.value()->resetNeedSend( uiId );
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
			lstr << QString("query  | %1 | %2").arg(sFromAddress.toString(), 15).arg(pq.print());
			DEBUG_MSG_INFO(lstr, CMsg::eMT_Info1);

			pAnswer pa;
			pa.setID(pq.getID());
			pa.setDateTime(QDateTime::currentDateTime());
			pa.setSID( m_usSId );
			if( !getAnswer(pa, pq.getType(), pq.getIDC(), sFromAddress.toString() ) ) {

				sendAnswer( pa, sFromAddress, pq.getIDC() );

				lstr.erase(lstr.begin(), lstr.end());
				lstr << QString("answer | %1 | %2").arg(sFromAddress.toString(), 15).arg(pa.print());
				DEBUG_MSG_INFO(lstr, CMsg::eMT_Info1);

				if(pa.getVariableCount())
					DEBUG_MSG_INFO(pa.print_vars(), CMsg::eMT_Info1);
			}
		}
		else {
			DEBUG_MSG_INFO(QString().sprintf("received corrupted datagram from %s\n", sFromAddress.toString().toAscii().data()), CMsg::eMT_Info1);
		}
	}
}

void UDPServer::sendMessage(const QStringList &sList, const QDateTime &oDateTime, int iType) {
	// если тип сообщения попадает в диапазон уровня отладки
	if( iType <= m_iDebugLevel && m_pParent )
		// посылаем сообщение
		QCoreApplication::postEvent(m_pParent, new CMsg(m_uiId, oDateTime, sList, iType));
}

