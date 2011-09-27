#ifndef __UDPCLIENT_H__
#define __UDPCLIENT_H__

#include <QThread>
#include <QHostAddress>
#include <QtNetwork>

#include "udp_pquery.h"
#include "udp_panswer.h"
#include "udp_server_info.h"
#include "cevent.h"
#include "parameter_abstract.h"

class QUdpSocket;
class XMLConfig;

class UDPClient : public QThread
{
	Q_OBJECT
public:
	UDPClient( QObject *, XMLConfig * );
	~UDPClient();
	void sendQuery( pQuery, QHostAddress, unsigned int );
	inline void stop() { m_bStop = true; }
	//! устанавливает указатель на глобальный "массив" данных
	static void setPtrToParameters(QMap<ulong, CParameter*> &ptrmapParameters) { m_ptrmapParameters = &ptrmapParameters; }
private slots:
	void processPendingDatagrams();
	void gatherVars();
	void gatherHard();
	void testChanels();
protected:
	void run();
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
	QObject * m_pParent;
	uint m_uiCId;
	QMap<uint, CUDPServerInfo*> m_mapHosts;
	QUdpSocket *m_udpTSocket, *m_udpRSocket;
	uint m_uiRPort;
	uchar rnd;
	bool m_bStop;
	uint m_uiMaxUpdate;
	uint m_uiUpdate;
	uint m_uiTestUpdate;
	uint m_uiVarUpdate;
	int m_iDebugLevel;
	unsigned int m_uiId;
	/*! \brief указатель на глобальный "массив" данных (объектов CParameter*) */
	static QMap<ulong, CParameter*> *m_ptrmapParameters;
	/*! \brief */
	QMap<uint, CUDPServerInfo*>::iterator current;
	/*! \brief */
	uint m_uiMaxClients;
	/*! \brief */
	bool m_bContinue;
	/*! \brief */
	QList<CUDPServerInfo*> m_listHardQueries;
	/*! \brief */
	bool m_bHard;
	/*! \brief */
	uint m_uiMaxHardQueries;
};

#endif
