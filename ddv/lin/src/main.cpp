#include <QApplication>
#include <QObject>

#include "mwnd.h"
#include "ulog.h"
#include "xmlconfig.h"
#include "exception.h"

//! файл отчета программы
auto_ptr<ULog> aptrMainLog;

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
XMLConfig *pXMLConfig = 0;
bool bLog = false;
int create_configuration() {

	aptrMainLog->add("try read configuration files");
	try {
		// чтение файла конфигурации
		pXMLConfig = new XMLConfig( "../etc/ddv.xml" );
	}
	catch(const ex_base &ex) {
		aptrMainLog->add(QStringList("failed") + ex.getErrors(), ULog::eMT_Error);
		return 4;
	}
	catch(...) {
		aptrMainLog->add(QStringList("failed") + QStringList("Unknown exception during parsing"), ULog::eMT_Error);
		return 5;
	}

	(pXMLConfig->attr("debug:log") == "yes") ? bLog = true : bLog = false;

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}


int main( int argc, char **argv ) {

	int iResult = 0;
	// создание файла отчета программы
	aptrMainLog = auto_ptr<ULog>( new ULog( "../ddv.txt", "cp1251") );
	// первое сообщение
	aptrMainLog->add("start report file");

	// инициализация XML библиотеки
	iResult = XMLInit();
	// проверка выполнения предыдущего действия
	if(!iResult) {
		// чтение конфигурационных файлов
		iResult = create_configuration();
		// конфигурационные файлы прочитаны
		if(!iResult) {
			// создание объектов приложения и главного окна
			QApplication app( argc, argv );
			try {
				CMainWnd wnd( pXMLConfig );
				wnd.show();
				iResult = app.exec();
			}
			catch(const ex_base &ex) {
				// исключение
				aptrMainLog->add(
						QStringList("failed") + QStringList(QString("%1: %2").arg(ex.get_where()).arg(ex.get_what())),
						ULog::eMT_Error
						);
				iResult = 6;
			}
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

