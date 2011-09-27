#include "exception.h"
#include "xmlconfig.h"
#include "ulog.h"
#include "udp_client.h"
#include "udp_server.h"
#include "parameter_creator.h"
#include "parameter_info_creator.h"
#include "parameter_info.h"
#include "parameter_abstract.h"
#include "ulog_director.h"

#include <QtGui/QApplication>
#include <QDir>

#include <memory>

// файлы xercesc
#include <xercesc/util/PlatformUtils.hpp>

//! файл отчета программы
std::auto_ptr<ULog> aptrMainLog;

//! инициализация XML библиотеки
int XMLInit() {

	// попытка инициализации XML библиотеки
	aptrMainLog->add("try initialize XML library");
	try {
		XMLPlatformUtils::Initialize();
	}
	// исключение XML при инициализации XML библиотеки
	catch(const XMLException &ex) {

		aptrMainLog->add("can't initialize XML library: " + XMLConfig::toQString(ex.getMessage()), ULog::eMT_Error);
		return 2;
	}
	// неизвестное исключение при инициализации XML библиотеки
	catch(...) {

		aptrMainLog->add("can't initialize XML library: unknown exception", ULog::eMT_Error);
		return 3;
	}

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! чтение конфигурации
//! файл конфигурации
XMLConfig *pXMLConfig = 0; QString sDir = QString::null; QString sConfig = "/etc/dac/dsc.xml";
bool bLog = false;
int create_configuration() {

	aptrMainLog->add("try read configuration files");
	try {
		// чтение файла конфигурации
		pXMLConfig = new XMLConfig( sConfig.toAscii() );
	}
	catch(const ex_base &ex) {
		aptrMainLog->add(QStringList("failed") + ex.getErrors(), ULog::eMT_Error);
		return 4;
	}
	catch(...) {
		aptrMainLog->add(QStringList("failed") + QStringList("Unknown exception during parsing"), ULog::eMT_Error);
		return 5;
	}

	(pXMLConfig->gattr("debug:log") == "yes") ? bLog = true : bLog = false;

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

#define _ulong( X, Y, Z ) CParameterProperties::toULong( X, Y, Z )

QMap<ulong, CParameter*> qmapParameters;
QMap<ulong, CParameter*> qmapLink;
//! создание параметров
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

	// параметр состояния клиента ( ...000001 )
	uchar id_dsc = pXMLConfig->attr(pNode, "id").toUInt();
	ulong id = _ulong( id_dsc, 000, 001 );
	qmapParameters[ id ] = pbuilder.build	(
				ePT_Value,
				CParameterProperties::StrValueTypeToEnum( "int" ),
				ibuilder.build( ePT_Value, id, "int", list )
						);
	qmapParameters.value( id )->setValue( "-1" );
	lstr << qmapParameters[ id ]->getStrId();

	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "servers", ""));
	// параметры состояния серверов
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
			qmapParameters.value( id )->setValue( "-1" );
			lstr << qmapParameters[ id ]->getStrId();

			// server logical connect 1 ( ...000..1 )
			id = _ulong( pXMLConfig->attr(pNode, "id").toUInt(), 000, QString("%1").arg( id_dsc, 2, 10, QChar('0') ).toUInt()*10 + 1 );
			qmapParameters[ id ] = pbuilder.build	(
					ePT_Value,
					CParameterProperties::StrValueTypeToEnum( "int" ),
					ibuilder.build( ePT_Value, id, "int", list )
										);
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
			qmapParameters.value( id )->setValue( "-1" );
			qmapLink[ id ] = qmapParameters.value( id );
			lstr << qmapParameters[ id ]->getStrId();

			if( pXMLConfig->isAttr( pNode, "link" ) && pXMLConfig->attr( pNode, "link" ) == "yes" )
				llinks.append( pXMLConfig->attr( pNode, "id" ) );
		}
		// переходим к следующему элементу
		pNode = pXMLConfig->next();
	}
	aptrMainLog->add( lstr );
	aptrMainLog->add("success", ULog::eMT_Success);

	lstr.clear(); lstr << "link parameters:";
	// параметры связи для смежных dsc
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
				qmapParameters.value( id )->setValue( "-1" );
				qmapParameters[_ulong( sId.toUInt(), 000, 001 )]->addStatusDepPar( qmapParameters.value( id ) );
				lstr << qmapParameters[ id ]->getStrId();
			}
			// переходим к следующему элементу
			pNode = pXMLConfig->next();
		}
	}
	aptrMainLog->add( lstr );
	aptrMainLog->add("success", ULog::eMT_Success);

	// параметры из базы
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

	// разрешение зависимостей
	aptrMainLog->add("resolve depends");
	foreach( CParameter *pPar, mDelayedCreateParameter.keys() ) {
		const DOMNode *pSubNode = pXMLConfig->first( mDelayedCreateParameter[pPar] );
		while( pSubNode ) {
			if( pSubNode->getNodeType() == DOMNode::ELEMENT_NODE ) {

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

//! уничтожение параметров
int delete_parameters() {

	aptrMainLog->add("delete parameters");

	QMap<ulong, CParameter*>::iterator p, e = qmapParameters.end();
	for(p = qmapParameters.begin();p!=e;++p)
		delete p.value();
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

ULogDirector * pLogDirector = 0;
//! создание логов
int create_logs() {

	if( bLog ) {

		pLogDirector = new ULogDirector;

		aptrMainLog->add("try logs objects");
		const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "logs", ""));
		while(pNode) {
			if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

				pLogDirector->add	(
						pXMLConfig->attr(pNode, "id").toUShort(),
						QString("%1/%2/%3")
								.arg(sDir)
								.arg(pXMLConfig->gattr("dirs:dir", "name", "log", "path"))
								.arg(pXMLConfig->attr(pNode, "caption")),
						pXMLConfig->attr(pXMLConfig->root(), "codepage"),
						0
							);
			}
			pNode = pXMLConfig->next();
		}

		aptrMainLog->add("success", ULog::eMT_Success);
	}

	return 0;
}

//! создание и запуск UDP-клиента
UDPClient *pUDPClient = 0;
void create_client() {

	aptrMainLog->add("create and start UDP client");

	UDPClient::setPtrToParameters( qmapParameters );
	pUDPClient = new UDPClient( pLogDirector, pXMLConfig );
	pUDPClient->start();

	aptrMainLog->add("success", ULog::eMT_Success);
}

//! создание создание и запуск UDP-сервера
UDPServer * pUDPServer = 0;
void create_server() {

	aptrMainLog->add("create and start UDP server");

	pUDPServer = new UDPServer( pXMLConfig, pLogDirector );

	aptrMainLog->add("success", ULog::eMT_Success);
}

//! зачистка
bool bStop = false;
void stop()
{
	if( bStop ) return;

	bStop = true;

	// удаление UDPServer
	aptrMainLog->add("destroy UDPServer");
	if( pUDPServer ) delete pUDPServer;
	aptrMainLog->add("success", ULog::eMT_Success);

	// остановка и удаление UDP клиента
	aptrMainLog->add("stop and destroy UDP client");
	if( pUDPClient ) {
	    pUDPClient->stop();
	    pUDPClient->wait( 5000 );
	}
	aptrMainLog->add("success", ULog::eMT_Success);

	// удаление логов
	if( bLog ) {
		aptrMainLog->add("destroy logs");
		if( pLogDirector ) delete pLogDirector;
		aptrMainLog->add("success", ULog::eMT_Success);
	}

	// удаление параметров
	delete_parameters();

	// удаление объектов файлов конфигурации
	aptrMainLog->add("destroy configuration objects");
	if( pXMLConfig ) delete pXMLConfig;
	aptrMainLog->add("success", ULog::eMT_Success);

	// деинициализация XML библиотеки
	aptrMainLog->add("deinitialize XML library");
	XMLPlatformUtils::Terminate();
	aptrMainLog->add("success", ULog::eMT_Success);

	// последнее сообщение
	aptrMainLog->add("stop  report file");
}

#include <signal.h>
// перехватчик сигналов
void signal_handler( int )
{
	stop();

	QCoreApplication::quit();
}

void daemon() {

	int pid;
	if( (pid = fork()) < 0 ) exit( 1 );
	if( pid != 0 ) exit( 0 );
}

int main( int argc, char **argv ) {

	// флаг запуска сервисом
	bool bDaemon = false;

	// обработка параметров командной строки
	uint uiResult = 0;
	for(int i=1;i<argc;i++) {
		if( QString(argv[i]) == "--directory" ) {
			if( ++i < argc ) { sDir = argv[i]; uiResult++; continue; }
			else exit( 1 );
		}
		if( QString(argv[i]) == "--daemon" ) { bDaemon = true; continue; }
		if( QString(argv[i]) == "--config" ) {
			if( ++i < argc ) { sConfig = argv[i]; uiResult++; continue; }
			else exit( 1 );
		}
	}
	if( uiResult < 2 ) exit( 1 );

	// создание файла отчета программы
	aptrMainLog = std::auto_ptr<ULog>( new ULog( QString("%1/dsc.txt").arg(sDir), "utf8") );
	// первое сообщение
	aptrMainLog->add("start report file");
	aptrMainLog->add( QString("working directory is %1").arg(sDir) );
	aptrMainLog->add( QString("configuration file is %1").arg(sConfig) );

	// запускаем демона :-))
	if( bDaemon ) daemon();

	// перехват сигналов
	struct sigaction sa;
	memset( &sa, 0, sizeof( sa ) );
	sa.sa_handler = &signal_handler;
	sigaction( SIGTERM, &sa, NULL );
	sigaction( SIGINT, &sa, NULL );

	// результат действия
	int iResult = 0;
	// инициализация XML библиотеки
	iResult = XMLInit();
	// проверка выполнения предыдущего действия
	if(!iResult) {
		// чтение конфигурационных файлов
		iResult = create_configuration();
		// конфигурационные файлы прочитаны
		if(!iResult) {

			QCoreApplication app( argc, argv );

			try {
				// создание параметров
				create_parameters();
				// создание объектов логов
				create_logs();
				// создание и запуск UDP клиента
				create_client();
				// создание и запуск UDP сервера
				create_server();
			}
			catch( const ex_base &ex ) {
				aptrMainLog->add(QStringList("failed") + QStringList(ex.get_what()), ULog::eMT_Error);
				return 6;
			}

			aptrMainLog->add("application is running...");
			// delete config (objects is created)
			delete pXMLConfig;
			iResult = app.exec();
		}
	}

	stop();

	return iResult;
}

