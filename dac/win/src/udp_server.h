//<codepage = utf8
#ifndef CLIENTCLASSDEFENITION_H
#define CLIENTCLASSDEFENITION_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QMap>

#include "udp_pquery.h"
#include "udp_panswer.h"

struct ClientInfo {
	QMap<QString, uint>	m_mapAddrs;
};

class QUdpSocket;
class XMLConfig;

class UDPServer : public QObject
{
    Q_OBJECT

public:
    UDPServer(XMLConfig *pXMLConfig, QObject *parent = 0);
	~UDPServer();

	int getAnswer(pAnswer & _pa, unsigned char _type, uint, const QString & );

	void sendAnswer(pAnswer & _pa, QHostAddress src_host, uint id );
	void setHost(QHostAddress _ha);
	void setPort(int _p);

	//! устанавливает указатель на объект QObject
	void setParent(QObject *pParent)					{ m_pParent = pParent; }
	//! устанавливает идентификатор нити
	void setId(const QString &sId)						{ m_uiId = sId.right(sId.length() - 1).toInt(); }	
	//! устанавливает уровень отладки
	void setDebugLevel(int iDebugLevel)					{ m_iDebugLevel = iDebugLevel; }

private slots:
	void processPendingDatagrams();

private:
	/*! \brief Посылка отладочного сообщения
		\param lstr cообщение
		\param date дата и метка времени сообщения
		\param type тип сообщения
		\sa ULog для формирования однострочного или многострочного сообщения
		\sa CMsg для установки типа сообщения
	*/
	void sendMessage(const QStringList &lstr, const QDateTime &date = QDateTime::currentDateTime(), int type = 1);

private:
	QObject *m_pParent;
	QUdpSocket *m_pobjUDPInSocket;
	QUdpSocket *m_pobjUDPOutSocket;
	unsigned int m_uiOutPort;
	int m_iDebugLevel;
	unsigned int m_uiId;
	ushort m_usSId;

	QMap<uint,ClientInfo>		 m_mapClientsInfo;
};

#endif


