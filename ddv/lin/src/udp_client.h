#ifndef __UDPCLIENT_H__
#define __UDPCLIENT_H__

#include <QThread>
#include <QHostAddress>
#include <QtNetwork>

#include "udp_pquery.h"
#include "udp_panswer.h"
#include "mwnd.h"

#define MAX_HOST_COUNT 16

class ServInfo {
public:
	QHostAddress sAddr;
	uint uiPort;
	bool bStatus;
};

class QUdpSocket;
class XMLConfig;
class QTimer;

class UDPClient : public QObject
{
	Q_OBJECT
public:
	UDPClient( CMainWnd *parent, XMLConfig * );
//	void setConfiguration(XMLConfig *pXMLConfig);
//	void stop() { m_bStop = true; }
private slots:
	void processPendingDatagrams();
	void gatherVars();
protected:
//	void run();
private:
	void sendQuery(pQuery, const QHostAddress &, unsigned int);
private:
	CMainWnd *m_pMainWnd;
	QMap<uint, ServInfo> m_mapServers;
	QUdpSocket *m_udpTSocket, *m_udpRSocket;
	unsigned int m_uiRPort;
	unsigned char rnd;
	pVariable pve[10000];
	QTimer *m_pTimer;
	uint m_uiId;
//	bool m_bStop;
//	unsigned int m_uiMaxUpdate;
	unsigned int m_uiUpdate;
//	unsigned int m_uiVarUpdate;
};

#endif

