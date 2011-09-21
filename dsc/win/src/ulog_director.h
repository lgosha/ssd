//<codepage = utf8
/*!
	\file ulog_director.h
	\brief Описание класса ULogDirector
	\author Лепехин Игорь Юрьевич
	\date 2010
	\warning Для использования класса необходимо наличие библиотеки Qt4
	\warning Платформной зависимости не замечено
*/

#ifndef __USERLOG_DIRECTOR_H__
#define __USERLOG_DIRECTOR_H__

#include <QObject>
#include <QMap>

#include "ulog.h"

class ULogDirector : public QObject {
	Q_OBJECT
public:
	 ULogDirector();
	~ULogDirector();

	void add( ushort, const QString &, const QString &, QListWidget * = 0 );
private:
	void customEvent( QEvent *pCustomEvent );
	static ULog::EMsgType CMsgTypeToULogType( int );
private:
	QMap<ushort, ULog*>	  m_mLogs;
};

#endif

