#include <QtGui/QApplication>
#include "ctrl.h"

#include "exception.h"
#include "xmlconfig.h"
#include "ulog.h"

// ����� xerces
#include <xercesc/util/PlatformUtils.hpp>

// ����� stl
#include <memory>

//! ���� ������ ���������
auto_ptr<ULog> aptrMainLog;

//! ������������� XML ����������
int XMLInit() {

	//// ������� ������������� XML ����������
	aptrMainLog->add("try initialize XML library");
	try {
		XMLPlatformUtils::Initialize();
	}
	// ���������� XML ��� ������������� XML ����������
	catch(const XMLException &ex) {

		aptrMainLog->add("can't initialize XML library: " + XMLConfig::toQString(ex.getMessage()), ULog::eMT_Error);
		return 2;
	}
	// ����������� ���������� ��� ������������� XML ����������
	catch(...) {

		aptrMainLog->add("can't initialize XML library: unknown exception", ULog::eMT_Error);
		return 3;
	}

	aptrMainLog->add("success", ULog::eMT_Success);

	return 0;
}

//! ������ ������������
//! ���� ������������
XMLConfig *pXMLConfig = 0; QString sDir = QString::null; QString sConfig = "../etc/proc.xml";
int create_configuration() {

	aptrMainLog->add("try read configuration files");
	try {
		// ������ ����� ������������
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
	// �������� ����� ������ ���������
	aptrMainLog = auto_ptr<ULog>( new ULog( QString("../ctrl.txt"), "utf8") );
	// ������ ���������
	aptrMainLog->add("start report file");
	aptrMainLog->add( QString("working directory is %1").arg(sDir) );
	aptrMainLog->add( QString("configuration file is %1").arg(sConfig) );

	// ������������� XML ����������
	int iResult = XMLInit();
	// �������� ���������� ����������� ��������
	if(!iResult) {
		// ������ ���������������� ������
		iResult = create_configuration();
		// ���������������� ����� ���������
		if(!iResult) {

			QApplication a(argc, argv);
			ctrl w( pXMLConfig );
			w.show();
			a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
			aptrMainLog->add("application is running...");
			iResult = a.exec();
		}
	}

	// �������� �������� ������ ������������
	aptrMainLog->add("destroy configuration objects");
	delete pXMLConfig;
	aptrMainLog->add("success", ULog::eMT_Success);

	// ��������������� XML ����������
	aptrMainLog->add("deinitialize XML library");
	XMLPlatformUtils::Terminate();
	aptrMainLog->add("success", ULog::eMT_Success);

	// ��������� ���������
	aptrMainLog->add("stop  report file");


	return iResult;
}
