#include "exception.h"
#include "xmlconfig.h"
#include "ulog.h"
#include "udp_client.h"
#include "udp_server.h"
#include "parameter_creator.h"
#include "parameter_info_creator.h"
#include "parameter_info.h"
#include "parameter_abstract.h"
#include "opc_server.h"

#include <QDir>
#include <QMap>
#include <QtCore/QProcess>

#include <memory>

// ����� xercesc
#include <xercesc/util/PlatformUtils.hpp>

//! ���� ������ ���������
std::auto_ptr<ULog> aptrMainLog;
//! ���� ����� ���������
std::auto_ptr<ULog> aptrFatalLog;

//! ������������� XML ����������
int XMLInit() {

	// ������� ������������� XML ����������
	aptrMainLog->add("try initialize XML library");
	try {
		XMLPlatformUtils::Initialize();
	}
	// ���������� XML ��� ������������� XML ����������
	catch(const XMLException &ex) {

		aptrMainLog->add("can't initialize XML library: " + XMLConfig::toQString(ex.getMessage()), ULog::eMT_Error);
		return 3;
	}
	// ����������� ���������� ��� ������������� XML ����������
	catch(...) {

		aptrMainLog->add("can't initialize XML library: unknown exception", ULog::eMT_Error);
		return 4;
	}

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! ������ ������������
//! ���� ������������
XMLConfig *pXMLConfig = 0; QString sDir = QString::null; QString sConfig = "/etc/dac/dsc.xml"; QString sFatalFile = QString::null;
bool bLog = false;
int create_configuration() {

	aptrMainLog->add("try read configuration files");
	try {
		// ������ ����� ������������
		pXMLConfig = new XMLConfig( sConfig.toAscii() );
	}
	catch(const ex_base &ex) {
		aptrMainLog->add(QStringList("failed") + ex.getErrors(), ULog::eMT_Error);
		return 5;
	}
	catch(...) {
		aptrMainLog->add(QStringList("failed") + QStringList("Unknown exception during parsing"), ULog::eMT_Error);
		return 6;
	}

	(pXMLConfig->gattr("debug:log") == "yes") ? bLog = true : bLog = false;

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

#define _ulong( X, Y, Z ) CParameterProperties::toULong( X, Y, Z )

QMap<ulong, CParameter*> qmapParameters;
QMap<CParameter *, CParameter *> qmapIndicators;
QList<CParameter*> qlistToSend;
QMap<ulong, CParameter*> qmapLink;
//! �������� ����������
int create_parameters() {
	aptrMainLog->add("create parameters");

	// get list of clients
	QList<unsigned int> list;
	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "clients", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE)
			list.append(pXMLConfig->attr(pNode, "id").toInt());
		pNode = pXMLConfig->next();
	}

	CParameterBuilder pbuilder; CParameterInfoBuilder ibuilder;
	CParameterProperties::Init();

	pNode = XMLConfig::find(pXMLConfig->root(), "network_c", "");
	QStringList lstr; lstr << "state parameters:";

	// �������� ��������� ������� ( ...000001 )
	uchar id_dsc = pXMLConfig->attr(pNode, "id").toUInt();
	ulong id = _ulong( id_dsc, 000, 001 );
	qmapParameters[ id ] = pbuilder.build	(
				ePT_Value,
				CParameterProperties::StrValueTypeToEnum( "int" ),
				ibuilder.build( ePT_Value, id, "int", list )
						);
	qlistToSend.append( qmapParameters[ id ] );
	qmapParameters.value( id )->setValue( "-1" );
	lstr << qmapParameters[ id ]->getStrId();

	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "servers", ""));
	// ��������� ��������� ��������
	QStringList llinks;
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

			// server state ( ...000001 )
			id = _ulong( pXMLConfig->attr(pNode, "id").toUInt(), 000, 001 );
			qmapParameters[ id ] = pbuilder.build	(
					ePT_Value,
					CParameterProperties::StrValueTypeToEnum( "int" ),
					ibuilder.build( ePT_Value, id, "int", list )
										);
			qlistToSend.append( qmapParameters[ id ] );
			qmapParameters.value( id )->setValue( "-1" );
			lstr << qmapParameters[ id ]->getStrId();

			// server logical connect 1 ( ...000..1 )
			id = _ulong( pXMLConfig->attr(pNode, "id").toUInt(), 000, QString("%1").arg( id_dsc, 2, 10, QChar('0') ).toUInt()*10 + 1 );
			qmapParameters[ id ] = pbuilder.build	(
					ePT_Value,
					CParameterProperties::StrValueTypeToEnum( "int" ),
					ibuilder.build( ePT_Value, id, "int", list )
										);
			qlistToSend.append( qmapParameters[ id ] );
			qmapParameters.value( id )->setValue( "-1" );
			qmapLink[ id ] = qmapParameters.value( id );
			lstr << qmapParameters[ id ]->getStrId();

			// server logical connect 2 ( ...000..2 )
			id = _ulong( pXMLConfig->attr(pNode, "id").toUInt(), 000, QString("%1").arg( id_dsc, 2, 10, QChar('0') ).toUInt()*10 + 2 );
			qmapParameters[ id ] = pbuilder.build	(
					ePT_Value,
					CParameterProperties::StrValueTypeToEnum( "int" ),
					ibuilder.build( ePT_Value, id, "int", list )
											);
			qlistToSend.append( qmapParameters[ id ] );
			qmapParameters.value( id )->setValue( "-1" );
			qmapLink[ id ] = qmapParameters.value( id );
			lstr << qmapParameters[ id ]->getStrId();

			if( pXMLConfig->isAttr( pNode, "link" ) && pXMLConfig->attr( pNode, "link" ) == "yes" )
				llinks.append( pXMLConfig->attr( pNode, "id" ) );
		}
		// ��������� � ���������� ��������
		pNode = pXMLConfig->next();
	}
	aptrMainLog->add( lstr );
	aptrMainLog->add("success", ULog::eMT_Success);

	lstr.clear(); lstr << "link parameters:";
	// ��������� ����� ��� ������� dsc
	foreach( QString sId, llinks ) {
		pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "servers", ""));
		while(pNode) {
			if(pNode->getNodeType() == DOMNode::ELEMENT_NODE && ( !pXMLConfig->isAttr( pNode, "link" ) || pXMLConfig->attr( pNode, "link" ) == "no" ) ) {

				// server logical connect 1 ( ...000..1 )
				id = _ulong( pXMLConfig->attr(pNode, "id").toUInt(), 000, QString("%1").arg( sId.toUInt(), 2, 10, QChar('0') ).toUInt()*10 + 1 );
				qmapParameters[ id ] = pbuilder.build	(
						ePT_Value,
						CParameterProperties::StrValueTypeToEnum( "int" ),
						ibuilder.build( ePT_Value, id, "int", list )
										);
				qlistToSend.append( qmapParameters[ id ] );
				qmapParameters.value( id )->setValue( "-1" );
				qmapParameters[_ulong( sId.toUInt(), 000, 001 )]->addStatusDepPar( qmapParameters.value( id ) );
				lstr << qmapParameters[ id ]->getStrId();

				// server logical connect 2 ( ...000..2 )
				id = _ulong( pXMLConfig->attr(pNode, "id").toUInt(), 000, QString("%1").arg( sId.toUInt(), 2, 10, QChar('0') ).toUInt()*10 + 2 );
				qmapParameters[ id ] = pbuilder.build	(
						ePT_Value,
						CParameterProperties::StrValueTypeToEnum( "int" ),
						ibuilder.build( ePT_Value, id, "int", list )
											);
				qlistToSend.append( qmapParameters[ id ] );
				qmapParameters.value( id )->setValue( "-1" );
				qmapParameters[_ulong( sId.toUInt(), 000, 001 )]->addStatusDepPar( qmapParameters.value( id ) );
				lstr << qmapParameters[ id ]->getStrId();
			}
			// ��������� � ���������� ��������
			pNode = pXMLConfig->next();
		}
	}
	aptrMainLog->add( lstr );
	aptrMainLog->add("success", ULog::eMT_Success);

	// ��������� �� ����
	lstr.clear(); lstr << "base parameters:";
	QMap<CParameter *, const DOMNode *> mDelayedCreateParameter;
	QList<ulong> lCommonParIds;
	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "base", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

#define valtype CParameterProperties::StrValueTypeToEnum( pXMLConfig->attr(pNode, "vtype") )
#define uid CParameterProperties::toULongWithClean( pXMLConfig->attr(pNode, "id") )

			qmapParameters[uid] = pbuilder.build	(
				ePT_Value,
				valtype,
				ibuilder.build(ePT_Value, uid, pXMLConfig, pNode, list)
								);
			qlistToSend.append( qmapParameters[ uid ] );
			if( pXMLConfig->isAttr(pNode, "serv") ) {
				if( qmapParameters[_ulong( pXMLConfig->attr(pNode, "serv").toUInt(), 000, 001 )] )
					qmapParameters[_ulong( pXMLConfig->attr(pNode, "serv").toUInt(), 000, 001 )]->addStatusDepPar( qmapParameters[uid] );
				else throw ex_base	(
						ex_base::error,
						QString("no server state parameter for: %1 and serv = %2").arg(uid).arg(pXMLConfig->attr(pNode, "serv")),
						"main::create_parameters"
							);
			}

			if( pNode->getFirstChild() ) {
				mDelayedCreateParameter[qmapParameters[uid]] = pNode ;
				lCommonParIds.append( uid );
			}

			lstr << qmapParameters[ uid ]->getStrId();
;
#undef valtype
#undef uid
		}
		pNode = pXMLConfig->next();
	}
	aptrMainLog->add( lstr );
	aptrMainLog->add("success", ULog::eMT_Success);

#define uid CParameterProperties::toULongWithClean( pXMLConfig->attr(pSubNode, "id") )

	// ���������� ������������
	aptrMainLog->add("resolve depends");
	foreach( CParameter *pPar, mDelayedCreateParameter.keys() ) {
		const DOMNode *pSubNode = pXMLConfig->first( mDelayedCreateParameter[pPar] );
		while( pSubNode ) {
			if( pSubNode->getNodeType() == DOMNode::ELEMENT_NODE ) {

				aptrMainLog->add( pPar->getStrId() );

				if( qmapParameters.contains( uid ) ) {
					qmapParameters[uid]->addCommonPar( pPar );
					qmapParameters[uid]->setCommonValue( pPar, pXMLConfig->isAttr(pSubNode, "value") ? pXMLConfig->attr(pSubNode, "value") : "1" );
					pPar->addCommonDepPar( qmapParameters[uid] );
				}
				else throw ex_base	(
						ex_base::error,
						QString("no parameter: %1 for common parameter: %2").arg(uid).arg(pPar->getStrId()),
						"main::create_parameters.resolve_depends"
									);
			}

			pSubNode = pXMLConfig->next();
		}
	}
	aptrMainLog->add("success", ULog::eMT_Success);

#undef uid

	return 0;
}

//! ����������� ����������
int delete_parameters() {

	aptrMainLog->add("delete parameters");

	QMap<ulong, CParameter*>::iterator p, e = qmapParameters.end();
	for(p = qmapParameters.begin();p!=e;++p)
		delete p.value();
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! �������� � ������ UDP-�������
UDPClient *pUDPClient = 0;
void create_client() {

	aptrMainLog->add("create and start UDP client");

	UDPClient::setPtrToParameters( qmapParameters );
	UDPClient::setPtrToModified( qlistToSend );
	pUDPClient = new UDPClient( pXMLConfig, bLog ? QString("%1/%2/udpclient").arg(sDir).arg(pXMLConfig->gattr("dirs:dir", "name", "log", "path")) : QString::null );
	pUDPClient->start();

	aptrMainLog->add("success", ULog::eMT_Success);
}

//! �������� �������� � ������ UDP-�������
UDPServer * pUDPServer = 0;
void create_server() {

	aptrMainLog->add("create and start UDP server");

	UDPServer::setPtrToParameters( qmapParameters );
	pUDPServer = new UDPServer( pXMLConfig,  bLog ? QString("%1/%2/udpserver").arg(sDir).arg(pXMLConfig->gattr("dirs:dir", "name", "log", "path")) : QString::null );
	pUDPServer->start();

	aptrMainLog->add("success", ULog::eMT_Success);
}

//! ��������
bool bStop = false;
void stop()
{
	if( bStop ) return;

	bStop = true;

	// �������� UDPServer
	aptrMainLog->add("destroy UDPServer");
	if( pUDPServer ) {
	    pUDPServer->stop();
	    pUDPServer->wait( 5000 );
		delete pUDPServer;
	}
	aptrMainLog->add("success", ULog::eMT_Success);

	// ��������� � �������� UDP �������
	aptrMainLog->add("stop and destroy UDP client");
	if( pUDPClient ) {
	    pUDPClient->stop();
	    pUDPClient->wait( 5000 );
		delete pUDPClient;
	}
	aptrMainLog->add("success", ULog::eMT_Success);

	// �������� ����������
	delete_parameters();

	// �������� �������� ������ ������������
	aptrMainLog->add("destroy configuration objects");
	if( pXMLConfig ) delete pXMLConfig;
	aptrMainLog->add("success", ULog::eMT_Success);

	// ��������������� XML ����������
	aptrMainLog->add("deinitialize XML library");
	XMLPlatformUtils::Terminate();
	aptrMainLog->add("success", ULog::eMT_Success);

	// ��������� ���������
	aptrMainLog->add("stop  report file");
}

// ***
// *** ������ �� �������� �������
bool isStarted() {

	QProcess proc;
	proc.start("tasklist");
	proc.waitForFinished( 5000 );
	QByteArray ba = proc.readAll();

	int iStart = -1; int n = 0;
	while( (iStart = ba.indexOf( "dsc.exe", iStart + 1 )) != -1 )
		n++;

	if( n <= 1 ) return false;

	return true;
}

// ***
void _fatal_( bool bResult, int iExit = 0 ) {

	!bResult ? aptrFatalLog->add("success", ULog::eMT_Success) : aptrFatalLog->add("failed", ULog::eMT_Error);
	aptrFatalLog->add("stop report file");

	if( iExit ) exit( iExit );
}

extern "C" int main(int argc, char *argv[])
{
	// ������ ���� ����� �������
	sFatalFile = QString("%1/fatal.txt").arg(QDir::tempPath());

  	// ������� ���������� ���� �������
	QFile::remove( sFatalFile );

	// ������� ����� ���� �������
	aptrFatalLog = auto_ptr<ULog>(new ULog(sFatalFile, "cp1251"));
	aptrFatalLog->add("start report file");

	// ������������ ��������� ��������� ������
	aptrFatalLog->add("check command line parameters");
	uint uiResult = 0;
	bool bRegister = false;
	for(int i=1;i<argc;i++) {

		if( QString(argv[i]) == "/r" || QString(argv[i]) == "/u" ) {
			bRegister = true; break;
		}
	
		if( QString(argv[i]) == "--directory" ) {
			if( ++i < argc ) { sDir = argv[i]; uiResult++; continue; }
			else aptrFatalLog->add("no working directory", ULog::eMT_Error);
		}
		if( QString(argv[i]) == "--config" ) {
			if( ++i < argc ) { sConfig = argv[i]; uiResult++; continue; }
			else aptrFatalLog->add("no config file", ULog::eMT_Error);
		}
	}

	// �������� ���������� ��������� ������
	if( !bRegister && uiResult < 2 ) _fatal_( true, 1 );
	else aptrFatalLog->add("success", ULog::eMT_Success);

	// �������� �� ���������� ��������� ����������
	aptrFatalLog->add("check for started");
	if( isStarted() ) _fatal_( true, 2 );
	aptrFatalLog->add( "success", ULog::eMT_Success );

	std::auto_ptr<ULog> aptrOPCServerLog;
	// ��������� ��������
	int iResult = 0;
	// ������ ���������
	if( !bRegister ) {

		// �������� ����� ������ ���������
		aptrMainLog = std::auto_ptr<ULog>( new ULog( QString("%1/dsc.txt").arg(sDir), "utf8") );

		// ������ ���������
		aptrMainLog->add("start report file");
		aptrMainLog->add( QString("working directory is %1").arg(sDir) );
		aptrMainLog->add( QString("configuration file is %1").arg(sConfig) );

		// ������������� XML ����������
		iResult = XMLInit();
		// �������� ���������� ���������� ��������
		if( iResult ) _fatal_( true, iResult );

		// ������ ���������������� ������
		iResult = create_configuration();
		// ���������������� ����� ���������

		if( iResult ) _fatal_( true, iResult );

		try {
			// �������� ����������
			create_parameters();
			// �������� � ������ UDP �������
			create_client();
			// �������� � ������ UDP �������
			create_server();
		}
		catch( const ex_base &ex ) {
			aptrMainLog->add(QStringList("failed") + QStringList(ex.get_what()), ULog::eMT_Error);
			return 7;
		}

		// ������� lock-���� (�������� ��� ����� �������� � �������� ���������)
		aptrMainLog->add("create lock file");
		QFile::remove( QString("%1/dsc.lock").arg(sDir) ); 
		if( !QFile::copy( QString("%1/dsc.lock.template").arg(sDir), QString("%1/dsc.lock").arg(sDir) ) ) _fatal_ ( true, 8 );
		aptrMainLog->add("success", ULog::eMT_Success);
				
		// ������� ���� ������ ��� ������ OPCServer
		aptrMainLog->add("create OPCServer object");
		if( bLog )
			aptrOPCServerLog = auto_ptr<ULog>(new ULog( QString("%1/%2/opcserver").arg(sDir).arg(pXMLConfig->gattr("dirs:dir", "name", "log", "path")), "cp1251" ));
			try {
				OPCServer::setDebugLevel( pXMLConfig->gattr("debug:level").toInt() );
				OPCServer::setLogger( aptrOPCServerLog.get() );
				OPCServer::setWDir( sDir );
				OPCServer::setPtrToParameters( qmapParameters );
				OPCServer::setPtrToModified( qlistToSend );
			}
			catch(const ex_base &ex) {
				aptrMainLog->add(QStringList("failed") + QStringList(ex.get_what()), ULog::eMT_Error);
				_fatal_ ( true, 9 );
			}
			aptrMainLog->add("success", ULog::eMT_Success);

			aptrMainLog->add("application is running...");
	}
	else
		OPCServer::setLogger( aptrFatalLog.get() );

	// ��������� ������� OPCServer
	iResult = OPCServer::mymine( GetModuleHandle(NULL), argc, argv );

	if( !bRegister ) stop();

	aptrFatalLog->add("application result");
	iResult ? _fatal_( true, iResult ) : _fatal_( false, iResult );
}
