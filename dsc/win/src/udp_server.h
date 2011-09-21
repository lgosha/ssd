//<codepage = utf8
#ifndef __UDP_CLIENT_H__
#define __UDP_CLIENT_H__

#include <QThread>
#include <QTimer>
#include <QHostAddress>
#include <QMap>

#include "udp_pquery.h"
#include "udp_panswer.h"
#include "ulog.h"
#include "parameter_abstract.h"

struct ClientInfo {
	QMap<QString, uint>	m_mapAddrs;
};

class QUdpSocket;
class XMLConfig;

class UDPServer : public QThread
{
	Q_OBJECT
public:
	UDPServer( XMLConfig *pXMLConfig, const QString & = QString::null);
	~UDPServer();

	int getAnswer(pAnswer & _pa, unsigned char _type, uint, const QString & ); //generates the answer (add the vars)

	void sendAnswer(pAnswer & _pa, QHostAddress src_host, uint id );

	inline void stop() { m_bStop = true; }

	//! устанавливает указатель на глобальный "массив" данных
	static void setPtrToParameters(QMap<ulong, CParameter*> &ptrmapParameters) { m_ptrmapParameters = &ptrmapParameters; }
protected:
	void run();

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

	static ULog::EMsgType CMsgTypeToULogType( int );
private:
	QUdpSocket *m_pobjUDPInSocket;
	QUdpSocket *m_pobjUDPOutSocket;
	unsigned int m_uiInPort;
	unsigned int m_uiOutPort;
	int m_iDebugLevel;
	ushort m_usSId;
	bool m_bStop;
	uint m_uiUpdate;

	QMap<CParameter *, unsigned int> m_mapStatus;
	QMap<uint,ClientInfo>		 m_mapClientsInfo;

	/*! \brief */
	ULog  * m_pLog;

	/*! \brief указатель на глобальный "массив" данных (объектов CParameter*) */
	static QMap<ulong, CParameter*> *m_ptrmapParameters;
};

#endif


