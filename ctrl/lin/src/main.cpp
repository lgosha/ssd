#include <QtGui/QApplication>
#include "ctrl.h"

#include "exception.h"
#include "xmlconfig.h"
#include "ulog.h"

// файлы xerces
#include <xercesc/util/PlatformUtils.hpp>

// файлы stl
#include <memory>

//! файл отчета программы
auto_ptr<ULog> aptrMainLog;

//! инициализация XML библиотеки
int XMLInit() {

	//// попытка инициализации XML библиотеки
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
XMLConfig *pXMLConfig = 0; QString sDir = QString::null; QString sConfig = "../etc/proc.xml";
int create_configuration() {

	aptrMainLog->add("try read configuration files");
	try {
		// чтение файла конфигурации
		pXMLConfig = new XMLConfig( sConfig.toAscii(), false );
	}
	catch(const ex_base &ex) {
		aptrMainLog->add(QStringList("failed") + ex.getErrors(), ULog::eMT_Error);
		return 4;
	}
	catch(...) {
		aptrMainLog->add(QStringList("failed") + QStringList("Unknown exception during parsing"), ULog::eMT_Error);
		return 5;
	}

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}


int main(int argc, char *argv[])
{
	// создание файла отчета программы
	aptrMainLog = auto_ptr<ULog>( new ULog( QString("../ctrl.txt"), "utf8") );
	// первое сообщение
	aptrMainLog->add("start report file");
	aptrMainLog->add( QString("working directory is %1").arg(sDir) );
	aptrMainLog->add( QString("configuration file is %1").arg(sConfig) );

	// инициализация XML библиотеки
	int iResult = XMLInit();
	// проверка выполнения предыдущего действия
	if(!iResult) {
		// чтение конфигурационных файлов
		iResult = create_configuration();
		// конфигурационные файлы прочитаны
		if(!iResult) {

			QApplication a(argc, argv);
			ctrl w( pXMLConfig );
			w.show();
			a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
			aptrMainLog->add("application is running...");
			iResult = a.exec();
		}
	}

	// удаление объектов файлов конфигурации
	aptrMainLog->add("destroy configuration objects");
	delete pXMLConfig;
	aptrMainLog->add("success", ULog::eMT_Success);

	// деинициализация XML библиотеки
	aptrMainLog->add("deinitialize XML library");
	XMLPlatformUtils::Terminate();
	aptrMainLog->add("success", ULog::eMT_Success);

	// последнее сообщение
	aptrMainLog->add("stop  report file");


	return iResult;
}
