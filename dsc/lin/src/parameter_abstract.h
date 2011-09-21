//<codepage = utf8
/*!
	\file parameter_abstract.h
	\brief Описание класса CParameter
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt4
	\warning Платформной зависимости не замечено
*/

#ifndef __PARAMETER_ABSTRACT_H__
#define __PARAMETER_ABSTRACT_H__

#include "parameter_enum.h"
#include "udp_panswer.h"

#include <QString>
#include <QDateTime>
#include <QMap>

using namespace std;

/*! \class CParameter
	\brief Абстрактный класс параметра
*/
class CParameter {
public:

	/*! \brief состояние параметра
		\details Состояние параметра в контексте периодического определения значения
	*/
	enum EParStatus
	{
		/*!	при очередном обновлении значение параметра не определено */
		ePUS_NotUpdated		= 0,
		/*!	при очередном обновлении значение параметра определено успешно */
		ePUS_Updated		= 1,
		/*!	состояние параметра при создании объекта класса */
		ePUS_Unknown		= 2,
	};
	
	static QString EParStatusToQString( EParStatus status ) {
	
		switch( status ) {
		case ePUS_NotUpdated:
			return "nupdt";
		break;
		case ePUS_Updated:
			return " updt";
		break;
		case ePUS_Unknown:
			return " unkn";
		break;
		}
		
		return "  err";
	}


	//! конструктор 
	CParameter(void) {};
	//! деструктор
	virtual ~CParameter(void) {};

	// интерфейсные функции
	// *** свойства параметра из его описания
	//! возвращает идентификатор
	virtual ulong		getId		()			const		= 0;
	//! возвращает строковый идентификатор
	virtual QString		getStrId	()			const		= 0;
	//! возвращает строковый идентификатор устройства
	virtual QString		getDevPath	()			const		= 0;
	//! возвращает идентификатор ПТС
	virtual ushort		getSHWSId	()			const		= 0;
	//! возвращает идентификатор устройства
	virtual ushort		getDevId	()			const		= 0;
	//! возвращает номер параметра	
	virtual ushort		getParId	()			const		= 0;
	//! возвращает тип значения параметра
	virtual ePP_ValueType	getValueType	()			const		= 0;
	// ***
	// *** свойства параметра ***
	//! пересчет значения обобщенного параметра
	virtual void		update		()					= 0;
	// *** значение
	//! возвращает значение параметра
	virtual QString		getStringValue	()			const		= 0;
	//! устанавливает значение параметра
	virtual void		setValue	( const QString & )			= 0;
	//! обновляет значение параметра
	virtual void		updateValue	( const QString & )			= 0;
	// *** метка времени
	//! возвращает метку времени
	virtual QDateTime	getTime		()			const		= 0;
	//! устанавливает метку времени
	virtual void		setTime		( const QDateTime & )			= 0;
	// *** состояние параметра
	//! возвращает состояние параметра
	virtual EParStatus	status		()			const		= 0;
	//! устанавливает состояние параметра
	virtual	void		setStatus	( EParStatus s )			= 0;
	// *** передача параметра
	//! запись параметра в объект сетевого пакета
	virtual void		toSend		( pAnswer &, bool = false )		= 0;
	//! установка флага отправки
	virtual void		setSend	()						= 0;
	//! сброс флага отправки
	virtual void		resetSend	( uint )				= 0;
	//!
	virtual bool		isNeedSend	( uint )				= 0;
	// *** зависимость параметра
	//! добавляет параметр, состояние которого зависит от значения данного
	virtual void		addStatusDepPar	( CParameter * )			= 0;
	//! добавляет обобщенный параметр
	virtual void		addCommonPar	( CParameter *, bool )			= 0;
	//! 
	virtual bool		isInvert	()					= 0;
	//! добавляет параметр от которого зависит обобщенный
	virtual void		addCommonDepPar	( CParameter * )			= 0;
	// *** отладка
	//! запись основных полей параметра в текстовый вид
	virtual void		print		( QStringList & )			= 0;
	// ***
};

#endif

