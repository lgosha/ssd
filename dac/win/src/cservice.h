//<codepage = utf8
// файлы проекта
#include "qtservice.h"

/*! файлы библиотеки Qt */
#include <QApplication>

/*! предобъ€вление классов проекта */
class ULog;
class XMLConfig;
class CMainWnd;
class UDPServer;
class ULogDirector;

class CService : public QtService<QApplication>
{
public:
    CService(int argc, char **argv);
    ~CService();

protected:

    void start();
    void stop();
    void pause();
    void resume();
    void processCommand(int code);

private:

	int XMLInit();
	int create_configuration();
	int create_main_wnd();
	int create_parameters();
	int delete_parameters();
	int create_logs();
	int create_threads();
	int start_threads();
	int terminate_and_delete_threads();
	void clean();

private:
	//! файл отчета программы
	ULog			* aptrMainLog;
	XMLConfig		* pXMLConfig;
	CMainWnd		* pWnd;
	ULogDirector	* pLogDirector;
	QList<QThread*>   listThreads;
	UDPServer		* pUDPServer;
	QString			  sDir;
	QString			  sConf;
	bool			  bLog;
	bool			  bWnd;
};
