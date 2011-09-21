#include "udp_server_info.h"

CUDPServerInfo::CUDPServerInfo( uint uiId, bool bLink ) :
	m_uiId			( uiId		),
	m_iQueryState		( -1		),
	m_eServerState		( eSS_Unknown	),
	m_bLink			( bLink		)
{
}

void CUDPServerInfo::addAddress( const QString &sAddr, uint uiPort, uint uiId ) {

	hAddr addr;
	addr.hAddr = QHostAddress( sAddr );
	addr.uiPort = uiPort;
	m_mapAddresses[ sAddr ] = addr;
	m_mapAddressesById[ uiId ] = sAddr;

	m_mapConnectState[ sAddr ] = 256;
}

QString CUDPServerInfo::EServerStateToString( EServerState state ) {

	switch( state ) {
	case 0: return "eSS_Available";		break;
	case 1: return "eSS_CorruptPacket";	break;
	case 2: return "eSS_Connection";	break;
	case 3: return "eSS_NoConnection";	break;
	case 4: return "eSS_Unknown";		break;
	};

	return "err";
}

bool CUDPServerInfo::isValid() {

	if( m_listAlive.size() )
		return true;

	m_eServerState = eSS_NoConnection;

	return false;
}

QString CUDPServerInfo::getAddressById( uint uiId ) {

	if( m_mapAddressesById.contains(uiId) ) return m_mapAddressesById[ uiId ];

	return QString::null;
}

bool CUDPServerInfo::isAddress( const QString &sAddr ) {

	return m_mapAddresses.contains( sAddr );
}

void CUDPServerInfo::updateConnectState() {

	foreach( QString sAddr, m_mapConnectState.keys() ) {

		if( m_mapConnectState[ sAddr ] != -1 )
			m_listAlive.removeAt( m_listAlive.indexOf( sAddr ) );
		else {
			if( m_eServerState == eSS_NoConnection || m_eServerState == eSS_Unknown )
				m_eServerState = eSS_Connection;
			if( m_listAlive.indexOf( sAddr ) == -1 )
				m_listAlive.append( sAddr );
		}
	}
}

void CUDPServerInfo::setQueryState( uchar state ){

	m_iQueryState = state;
}

bool CUDPServerInfo::getQueryState( uchar state ) {

	if( m_iQueryState == state ) {

		m_iQueryState = -1;
		return true;
	}

	return false;
}

bool CUDPServerInfo::getConnectState( uchar id ) {

	QMap<QString, short>::iterator p, e = m_mapConnectState.end();
	for(p=m_mapConnectState.begin();p!=e;++p) {
		if( p.value() == id ) {
			p.value() = -1;
			return true;
		}
	}
	return false;
}

QStringList CUDPServerInfo::print() {

	QStringList lstr; lstr << QString("state for: %1").arg(m_uiId);
	QString sTmp1 = "", sTmp2 = "";
	QMap<QString, short>::const_iterator p, e = m_mapConnectState.end();
	for(p=m_mapConnectState.begin();p!=e;++p) {
		sTmp1 += QString("| %1 ").arg(p.key(),   15);
		sTmp2 += QString("| %1 ").arg(p.value(), 15);
	}
	lstr << sTmp1 << sTmp2;
	QList<QString>::const_iterator lp, le = m_listAlive.end();
	sTmp1 = "";
	for(lp=m_listAlive.begin();lp!=le;++lp)
		sTmp1 += QString("| %1 ").arg(*lp, 15);
	lstr << sTmp1;
	lstr << QString( "Server state: %1" ).arg( EServerStateToString( m_eServerState ) );

	return lstr;
}

