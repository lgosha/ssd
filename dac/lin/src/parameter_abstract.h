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
		/*!	обновление значения начато */
		ePUS_UpdateStart	= 3,
		/*!	обновление значения закончено */
		ePUS_UpdateStop		= 4
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
		case ePUS_UpdateStart:
			return "bupdt";
		break;
		case ePUS_UpdateStop:
			return "eupdt";
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
	//! возвращает период обновления
	virtual ushort		getUpdateTime	()			const		= 0;
	//! возвращает тип запроcа значения параметра
	virtual ePP_QueryType	getQueryType	()			const		= 0;
	//! возвращает тип значения параметра
	virtual ePP_ValueType	getValueType	()			const		= 0;
	//! возвращает задержку определения значения параметра
	virtual int		getStart	()			const		= 0;
	//! возвращает адрес запроса значения параметра
	virtual QString		getAddr		()			const		= 0;
	//! возвращает свойство класса запроса значения параметра
	virtual QString		getProperty	()			const		= 0;
	//! возвращает идентификатор параметра от которого зависит значение данного
	virtual ulong		getDepValParId	()			const		= 0;
	//! возвращает период вычисления значения
	virtual uint		getDelay	()			const		= 0;
	//! синхронный или асинхронный запрос
	virtual bool		isSync		()			const		= 0;
	//! параметр или группа
	virtual bool		isGroup		()			const		= 0;
	// ***

	// *** свойства параметра ***
	// *** значение
	//! возвращает значение параметра
	virtual QString		getStringValue	()			const		= 0;
	//! устанавливает значение параметра
	virtual void		setValue	( const QString &, bool = true )	= 0;
	//! устанавливает значение параметра
	  virtual void		setValue	( CParameter *, bool = true )		= 0;
	//! применяет значение параметра
	virtual void		applyValue	()					= 0;
	// *** метка времени
	//! возвращает метку времени
	virtual QDateTime	getTime		()			const		= 0;
	//! устанавливает метку времени
	virtual void		setTime		( const QDateTime & )			= 0;
	// *** флаг посылки значения клиенту
	//! возвращает флаг посылки значения клиенту
	virtual bool		isNeedSend	( uint )		const		= 0;
	//! устанавливает флаг посылки значения всем клиентам
	virtual	void		setNeedSendAll	()					= 0;
	//! сбрасывает флаг посылки значения клиенту
	virtual	void		resetNeedSend	( uint )				= 0;
	// *** состояние параметра
	//! возвращает состояние параметра
	virtual EParStatus	status		()			const		= 0;
	//! возвращает реальное состояние параметра
	virtual	EParStatus	rstatus		()			const		= 0;
	//! устанавливает состояние параметра в CParameter::ePUS_NotUpdated
	virtual void		setNotUpdated	()					= 0;
	// *** флаг обновления значения
	//! возвращает флаг обновления значения
	virtual bool		isNeedUpdate	()			const		= 0;
	//! устанавливает флаг обновления значения
	virtual void		setNeedUpdate	( bool )				= 0;
	// *** флаг безусловного обновления значения
	//! возвращает флаг безусловного обновления значения
	virtual bool		isForceUpdate	()			const		= 0;
	//! устанавливает флаг безусловного обновления значения
	virtual void		setForceUpdate	( bool )				= 0;

	// *** зависимости
	//! *** математическое выражения
	//! добавляет указатель на параметр от которого зависит вычисление математического выражения данного
	virtual void			setMathDepPar	( CParameter *, ulong = 0 )		= 0;
	//! добавляет указатель на параметр, вычисление математического выражения которого зависит от данного
	virtual void			addMathDepPar	( CParameter * )			= 0;
	//! *** состояние
	//! добавляет указатель на параметр, состояние которого зависит от данного
	virtual void			addStatusDepPar	( CParameter * )			= 0;

	// *** передача параметра
	//! запись параметра в объект сетевого пакета
	virtual void		toSend		( pAnswer &, bool = false )			= 0;

	// *** сравнение
	//! логическое сравнение
	virtual bool		compare		( const QString &, const QString & )		= 0;
	// ***

	// *** отладка
	//! запись основных полей параметра в текстовый вид
	virtual void		print		( QStringList & )			= 0;
	// ***

};

#endif

