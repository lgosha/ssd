#ifndef __UDP_SERVER_INFO__
#define __UDP_SERVER_INFO__

#include <QHostAddress>
#include <QStringList>
#include <QMap>

class CUDPServerInfo {
public:
	struct hAddr {
		QHostAddress hAddr;
		uint	uiPort;
	};

	/*! \brief состояние сервера
		\details Состояние сервера в контексте получения ответов на запросы
	*/
	enum EServerState
	{
		/*!	с сервером есть связь хотя бы по одному интерфейсу,
			при предыдущем запросе данных от сервера получен корректный пакет ответа
		*/
		eSS_Available		= 0,
		/*!	при предыдущем запросе данных от сервера получен искаженный пакет ответа */
		eSS_CorruptPacket	= 1,
		/*!	связь с сервером утановлена, данные не получены */
		eSS_Connection		= 2,
		/*!	с сервером нет связи ни по одному из интерфейсов */
		eSS_NoConnection	= 3,
		/*!	начальное состояние */
		eSS_Unknown		= 4,
	};

	static QString EServerStateToString( EServerState );

	CUDPServerInfo( uint = 0, bool = false );

	void addAddress( const QString &, uint, uint );
	QString getAddressById( uint );
	bool isAddress( const QString & );

	inline bool isLink() { return m_bLink; }

	bool getQueryState( uchar );
	void setQueryState( uchar );

	inline EServerState getServerState() { return m_eServerState; }
	inline void setServerState( EServerState state ) { m_eServerState = state; }

	inline int getConnectState( QString addr ) { return m_mapConnectState[addr]; }
	inline void setConnectState(QString addr, short state) { m_mapConnectState[addr] = state; }
	bool getConnectState( uchar );

	inline	const QMap<QString, hAddr> & getAddreses() { return m_mapAddresses; }

	inline	QHostAddress getAddress() { return QHostAddress( m_listAlive.first() ); }
	inline	uint getPort() { return m_mapAddresses[ m_listAlive.first() ].uiPort; }

	inline	uint getId() { return m_uiId; }

	bool isValid();

	void updateConnectState();

	QStringList print();

private:
	uint				m_uiId;
	QMap<QString, hAddr>		m_mapAddresses;
	QMap<uint, QString>		m_mapAddressesById;
	QMap<QString, short>		m_mapConnectState;
	QList<QString>			m_listAlive;
	int				m_iQueryState;
	EServerState			m_eServerState;
	bool				m_bLink;
};

#endif

