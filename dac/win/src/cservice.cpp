// файлы проекта
#include "cservice.h"
#include "mwnd.h"
#include "xmlconfig.h"
#include "exception.h"
#include "parameter_creator.h"
#include "parameter_info_creator.h"
#include "parameter_info.h"
#include "parameter_abstract.h"
#include "datathread_snmp.h"
#include "datathread_wmi.h"
#include "udp_server.h"
#include "ulog_director.h"

// глобальный массив параметров
QMap<ulong, CParameter*> qmapParameters;

CService::CService(int argc, char **argv) : 
	QtService<QApplication>	( argc, argv, "dac" ),
	aptrMainLog				( 0 ),
	pXMLConfig				( 0 ),
	pWnd					( 0 ),
	pLogDirector			( 0 ),
	pUDPServer				( 0 ),
	sDir					( QString::null ),
	sConf					( QString::null ),
	bLog					( false ),
	bWnd					( false )
{
    setServiceDescription("Diagnostic data collecting component");
    setServiceFlags(QtServiceBase::CanBeSuspended);

	// обработка параметров командной строки
	for(int i=1;i<argc;i++) {
		if( QString(argv[i]) == "--directory" ) {
			if( ++i < argc ) { sDir = argv[i]; continue; }
		}
		if( QString(argv[i]) == "--config" ) {
			if( ++i < argc ) { sConf = argv[i]; continue; }
		}
	}
}

CService::~CService()
{
}

//! инициализация XML библиотеки
int CService::XMLInit() {

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
int CService::create_configuration() {

	aptrMainLog->add("try read configuration files");
	try {
		// чтение файла конфигурации
		pXMLConfig = new XMLConfig( QString("%1/etc/%2").arg(sDir).arg(sConf).toAscii() );
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
	(pXMLConfig->gattr("debug:wnd") == "yes") ? bWnd = true : bWnd = false;

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! создание главного окна
int CService::create_main_wnd() {

	aptrMainLog->add("try create main window");
	pWnd = new CMainWnd(pXMLConfig);
	pWnd->setWindowTitle("ARMComponent_v2.0");
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! создание параметров
int CService::create_parameters() {

	aptrMainLog->add("create parameters");

	// get list of clients
	QList<unsigned int> list;
	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "clients", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE)
			list.append(pXMLConfig->attr(pNode, "id").toInt());
		pNode = pXMLConfig->next();
	}

	// get map of threads update time
	QMap<QString, int> map;
	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "threads", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE)
			map[pXMLConfig->attr(pNode, "type")] = pXMLConfig->attr(pNode, "tupdt").toInt();
		pNode = pXMLConfig->next();
	}

#define uid1 CParameterProperties::toULong( pXMLConfig->attr(pNode, "id") )

	CParameterProperties::Init();

	QMap< ulong, QList<ulong> > mapMath, mapStatus;
	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "base", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			// math depends
			if( pXMLConfig->isAttr( pNode, "math" ) ) {
				QString sTmp = pXMLConfig->attr( pNode, "math" );
				int iPos = -1;
				while( (iPos = sTmp.indexOf(',')) != -1 ) {
					QString sId = sTmp.mid( iPos + 1, c_uiIdPrecision*3 );

					mapMath		[ CParameterProperties::toULong( sId ) ].append( uid1 );
					mapStatus	[ CParameterProperties::toULong( sId ) ].append( uid1 );

					sTmp = sTmp.right( sTmp.length() - ( iPos + c_uiIdPrecision*3 + 1 ) );
				}
			}
			// sdep depends
			if( pXMLConfig->isAttr( pNode, "sdep" ) )
				mapStatus[ CParameterProperties::toULong( pXMLConfig->attr( pNode, "sdep" ) ) ].append( uid1 );
		}
		pNode = pXMLConfig->next();
	}

#undef uid1

	// process sdep depend
	QMap< ulong, QList<ulong> >::iterator it;
	bool m_bStop = false;
	while( !m_bStop ) {
		aptrMainLog->add( "step..." );
		m_bStop = true;
		for( it = mapStatus.begin();it != mapStatus.end();++it ) {
			foreach( ulong val, *it ) {
				QMap< ulong, QList<ulong> >::iterator p;
				for( p = mapStatus.begin(); p != mapStatus.end();++p ) {
					if( it.key() != p.key() && val == p.key() ) {
						aptrMainLog->add( "yes" );
						m_bStop = false;
						foreach( ulong tmp, p.value() )
							if( !it->contains( tmp ) ) it->append( tmp );
						p = mapStatus.erase( p );
					}
				}
			}
		}
	}

	// check sdep depends
	QMap< ulong, QList<ulong> >::iterator it1, it2;
	QList<ulong>::iterator p1, p2;
	for( it1 = mapStatus.begin();it1 != mapStatus.end();++it1 )
		for(p1=it1->begin();p1!=it1->end();++p1)
			for( it2 = mapStatus.begin();it2 != mapStatus.end();++it2 )
				if( it1.key() != it2.key() ) {
					for(p2=it2->begin();p2!=it2->end();++p2)
						if( *p1 == *p2 ) {
							throw ex_base	(
									ex_base::error,
									QString("bad status depend for: %1").arg( *p1 )
									);
						}
				}

	aptrMainLog->add( "math" );
	foreach( ulong key, mapMath.keys() ) {
		QStringList lstr; lstr << CParameterProperties::toStr( key );
		foreach( ulong val, mapMath[key] )
			lstr << CParameterProperties::toStr( val );
		aptrMainLog->add(lstr);
	}
	aptrMainLog->add( "sdep" );
	foreach( ulong key, mapStatus.keys() ) {
		QStringList lstr; lstr << CParameterProperties::toStr( key );
		foreach( ulong val, mapStatus[key] )
			lstr << CParameterProperties::toStr( val );
		aptrMainLog->add(lstr);
	}

	CParameterBuilder pbuilder; CParameterInfoBuilder ibuilder;
	pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "base", ""));
//	srand( time( 0 ) );
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

#define partype		CParameterProperties::StrParameterTypeToEnum	( pXMLConfig->attr(pNode, "ptype")	)
#define valtype		CParameterProperties::StrValueTypeToEnum		( pXMLConfig->attr(pNode, "vtype")	)
#define uid			CParameterProperties::toULong					( pXMLConfig->attr(pNode, "id")		)

			qmapParameters[uid] = pbuilder.build	(
				partype,
				valtype,
				ibuilder.build(partype, uid, pXMLConfig, pNode, list, map)
								);
//			qmapParameters[uid]->check();

#undef partype
#undef valtype
#undef uid

		}
		pNode = pXMLConfig->next();
	}

	aptrMainLog->add("success", ULog::eMT_Success);

	aptrMainLog->add("resolve math depends");
	// resolve math depends
	for( it = mapMath.begin();it != mapMath.end();++it ) {
		if( !qmapParameters.contains( it.key() ) )
			throw ex_base( ex_base::error, QString( "unknown key parameter: %1 with math depend" ).arg( it.key() ));
		foreach( ulong val, *it ) {
			if( !qmapParameters.contains( val ) )
				throw ex_base( ex_base::error, QString( "unknown math depend parameter: %1 for: %2" ).arg( it.key() ).arg( val ));
			qmapParameters[ val ]->setMathDepPar( qmapParameters[ it.key() ] );
			qmapParameters[ it.key() ]->addMathDepPar( qmapParameters[ val ] );
		}
	}
	aptrMainLog->add("success", ULog::eMT_Success);

	aptrMainLog->add("resolve status depends");
	// resolve status depends
	for( it = mapStatus.begin();it != mapStatus.end();++it ) {
		if( !qmapParameters.contains( it.key() ) )
			throw ex_base( ex_base::error, QString( "unknown key parameter: %1 with status depend" ).arg( it.key() ));
		foreach( ulong val, *it ) {
			if( !qmapParameters.contains( val ) )
				throw ex_base( ex_base::error, QString( "unknown status depend parameter: %1 for: %2" ).arg( it.key() ).arg( val ));
			qmapParameters[ it.key() ]->addStatusDepPar( qmapParameters[ val ] );
		}
	}
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! уничтожение параметров
int CService::delete_parameters() {

	aptrMainLog->add("delete parameters");

	QMap<ulong, CParameter*>::iterator p, e = qmapParameters.end();
	for(p = qmapParameters.begin();p!=e;++p)
		delete p.value();
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! создание логов
int CService::create_logs() {

	if( bLog ) {

		pLogDirector = new ULogDirector;

		aptrMainLog->add("try logs objects");
		const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "logs", ""));
		while(pNode) {
			if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

				pLogDirector->add	(
						pXMLConfig->attr(pNode, "id").toUShort(),
						QString("%1/log/%2")
								.arg(sDir)
								.arg(pXMLConfig->attr(pNode, "caption")),
						pXMLConfig->attr(pXMLConfig->root(), "codepage"),
						bWnd ? pWnd->getView( pXMLConfig->attr(pNode, "id").toUShort() ) : 0
							);
			}
			pNode = pXMLConfig->next();
		}

		aptrMainLog->add("success", ULog::eMT_Success);
	}

	return 0;
}

//! создание нитей данных
int CService::create_threads() {

	aptrMainLog->add("try create data threads");
	DOMNode *pCurNode = 0;
	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "threads", ""));
	QThread *pDataThread = 0;
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {			
			// SNMP Thread
			if(pXMLConfig->attr(pNode, "type") == "snmp") {

				CSNMPThread::setPtrToParameters( qmapParameters );
				CSNMPThread::setParent( pLogDirector );
				CSNMPThread::setId( pXMLConfig->attr(pNode, "logid") );
				pCurNode = pXMLConfig->current();
				pDataThread = new CSNMPThread	(
					pXMLConfig,
					CParameterProperties::StrQueryTypeToEnum(pXMLConfig->attr(pNode, "type")),
					pXMLConfig->attr(pNode, "tupdt").toUInt(),
					pXMLConfig->attr(pNode, "mupdt").toUInt()
												);
				listThreads.push_back(pDataThread);
				pXMLConfig->setCurrent(pCurNode);
			}
			// WMI Thread
			if(pXMLConfig->attr(pNode, "type") == "wmi") {

				CWMIThread::setPtrToParameters( qmapParameters );
				CWMIThread::setParent( pLogDirector );
				CWMIThread::setId( pXMLConfig->attr(pNode, "logid") ); 
				pCurNode = pXMLConfig->current();
				pDataThread = new CWMIThread	(
					pXMLConfig,
					CParameterProperties::StrQueryTypeToEnum(pXMLConfig->attr(pNode, "type")),
					pXMLConfig->attr(pNode, "tupdt").toUInt(),
					pXMLConfig->attr(pNode, "mupdt").toUInt()
												);
				listThreads.push_back(pDataThread);
				pXMLConfig->setCurrent(pCurNode);
			}
		}
		pNode = pXMLConfig->next();
	}
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! запуск нитей
int CService::start_threads() {

	aptrMainLog->add("start threads");
	QList<QThread*>::const_iterator cp, ce = listThreads.end();
	for(cp = listThreads.begin();cp!=ce;++cp)
		(*cp)->start();
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! остановка и уничтожение нитей данных
int CService::terminate_and_delete_threads() {

	aptrMainLog->add("stop data threads");
	// остановка нитей
	QList<QThread*>::const_iterator cp, ce = listThreads.end();
	for(cp = listThreads.begin();cp!=ce;++cp) {
		reinterpret_cast<CSNMPThread*>(*cp)->stop();
		(*cp)->wait( 5000 );
	}

	aptrMainLog->add("success", ULog::eMT_Success);

	aptrMainLog->add("delete data threads");
	// уничтожение объектов нитей
	QList<QThread*>::iterator p, e = listThreads.end();
	for(p = listThreads.begin();p!=e;++p)
		delete (*p);
	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

// остановка нитей о зачистка
void CService::clean() {

	aptrMainLog->add("catch stop signal");

	// удаление UDPServer
	aptrMainLog->add("destroy UDPServer");
	if( pUDPServer ) delete pUDPServer;
	aptrMainLog->add("success", ULog::eMT_Success);

	// остановка и уничтожение нитей данных
	terminate_and_delete_threads();
	// удаление параметров
	delete_parameters();
	// удаление главного окна
	if( bWnd ) {
		aptrMainLog->add("destroy main window");
		if( pWnd ) delete pWnd;
		aptrMainLog->add("success", ULog::eMT_Success);
	}
	// удаление логов
	if( bLog ) {
		aptrMainLog->add("destroy logs");
		if( pLogDirector ) delete pLogDirector;
		aptrMainLog->add("success", ULog::eMT_Success);
	}

	// удаление объектов файлов конфигурации
	aptrMainLog->add("destroy configuration objects");
	if( pXMLConfig ) delete pXMLConfig;
	aptrMainLog->add("success", ULog::eMT_Success);

	// деинициализация XML библиотеки
	aptrMainLog->add("deinitialize XML library");
	XMLPlatformUtils::Terminate();
	aptrMainLog->add("success", ULog::eMT_Success);

	// деинициализация сокетов средствами библиотеки netsnmp
	CSNMPThread::SOCK_STOP();

	// последнее сообщение
	aptrMainLog->add("stop  report file");

	if( aptrMainLog ) delete aptrMainLog;
}

void CService::start()
{
	if( sDir.isNull() ) return;

    qApp->setQuitOnLastWindowClosed(false);

	// создание файла отчета программы
	try {
		aptrMainLog = new ULog( QString("%1/dac.txt").arg(sDir), "utf8");
	}
	catch(const ex_base &ex) {
		logMessage( QString("error: %1").arg(ex.get_what()), QtServiceBase::Error );
		exit( 1 );
	}
	// первое сообщение
	aptrMainLog->add("start report file");
	aptrMainLog->add( QString("working directory is %1").arg(sDir) );

	// результат действия
	int iResult = 0;
	// инициализация сокетов средствами библиотеки netsnmp
	CSNMPThread::SOCK_START();
	// инициализация XML библиотеки
	iResult = XMLInit();
	// проверка выполнения предыдущего действия
	if(!iResult) {
		// чтение конфигурационных файлов
		iResult = create_configuration();
		// конфигурационные файлы прочитаны
		if(!iResult) {

			try {
				// создание объекта главного окна
				 if( bWnd ) create_main_wnd();
				// создание объектов логов
				create_logs();
				// создание параметров
				create_parameters();
				// создание объектов нитей
				create_threads();
			}
			catch(const ex_base &ex) {
				// исключение
				aptrMainLog->add(
						QStringList("failed") + QStringList(QString("%1: %2").arg(ex.get_where()).arg(ex.get_what())),
						ULog::eMT_Error
						);
				iResult = 6;
			}

			if(!iResult) {

				// создание и запуск сетевого сервера
				aptrMainLog->add("create and start UDPServer");
				pUDPServer = new UDPServer( pXMLConfig, pLogDirector );
				aptrMainLog->add("success", ULog::eMT_Success);
				// отображение главного окна
				if( bWnd ) {
					aptrMainLog->add( "show window..." );
					pWnd->show();
				}
				// запуск нитей
				start_threads();
				// запуск приложения
				aptrMainLog->add("application is running...");
			} 
			else { clean(); exit( 1 ); }
		}
		else { clean(); exit( 1 ); }
	}
	else { clean(); exit( 1 ); }
}

void CService::stop()
{
	clean();
}

void CService::pause()
{
}

void CService::resume()
{
}

void CService::processCommand( int code )
{
	if( code == 1 )
		if( pWnd ) pWnd->show();
}
