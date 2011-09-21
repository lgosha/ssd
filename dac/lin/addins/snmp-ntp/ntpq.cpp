
#include "ntpq.h"

#include <QtCore/QProcess>

extern "C" void fill_values();

static int status;
static std::string offset;
static std::string src;
static time_t lUpd = 0;

inline void fill_values() {

	if( (time(0) - lUpd) < 2 ) return;
    
	status		= 0;
	offset		= "0.000";
	src		= "0.0.0.0";
    
	QProcess proc;
	proc.start("ntpq", QStringList() << "-n" << "-p");
	// exec ntpq -p
	if( proc.waitForFinished( 5000 ) && proc.exitCode() == 0 ) {
		QList<QByteArray> lstderr = proc.readAllStandardError().split('\n');
		foreach( QByteArray ba, lstderr ) {
			if( ba.indexOf("refused") != -1 ) { status = 1; break; }
		}
		// ntp up
		if( status == 0 ) {
			QList<QByteArray> lstdout = proc.readAllStandardOutput().split('\n');
			QString str = QString();
			foreach( QByteArray ba, lstdout ) {
				if( ba.indexOf('*') != -1 ) { status = 3; str = ba; break; }
			}
			// ntp sync
			if( !str.isNull() ) {
				QStringList lstr = str.split( QRegExp("\\s+") );
				offset = lstr.at( lstr.size() - 2 ).toStdString();
				QString sTmp = lstr.at( 0 );
				src = sTmp.replace('*', "").toStdString();
			}
			else status = 2;
		}
	}
    
	lUpd = time(0);
}

std::string get_src(){
	fill_values();
	return src;
}

std::string get_offset(){
	fill_values();
	return offset;
}

int get_status(){
	fill_values();
	return status;
}
