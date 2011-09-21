//<codepage = utf8
/*!
	\file parameter_template.h
	\brief Описание класса CParameterTemplate
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt4
	\warning Платформной зависимости не замечено
*/

#ifndef __PARAMETER_TYPE_TEMPLATE_H__
#define __PARAMETER_TYPE_TEMPLATE_H__

#include "parameter_abstract.h"
#include "parameter_info.h"

#include <QTextStream>

/*! \class CParameterTemplate
	\brief Шаблонный класс параметра
*/
template<class T> class CParameterTemplate : public CParameter {
public:
	//! конструктор
	CParameterTemplate( const CParameterDefinition *pParameterInfo );
	//! деструктор
	virtual ~CParameterTemplate(void);
public:
	// интерфейсные функции
	// *** свойства параметра из его описания ***
	//! возвращает идентификатор
	inline		ulong		getId		()			const;
	//! возвращает строковый идентификатор
	inline		QString		getStrId	()			const;
	//! возвращает строковый идентификатор устройства
	inline		QString		getDevPath	()			const;
	//! возвращает идентификатор ПТС
	inline		ushort		getSHWSId	()			const;
	//! возвращает идентификатор устройства
	inline		ushort		getDevId	()			const;
	//! возвращает номер параметра
	inline		ushort		getParId	()			const;
	//! возвращает тип значения параметра
	inline 		ePP_ValueType	getValueType	()			const;
	// ***
	// *** свойства параметра ***
	//! пересчет значения обобщенного параметра
		virtual void		update		();
	// *** значение
	//! возвращает значение параметра
			QString		getStringValue	()			const;
	//! устанавливает значение параметра
		virtual	void		setValue	( const QString & );
	//! устанавливает значение параметра
		virtual	void		setValue	( CParameter * );
	//! обновляет значение параметра
		virtual void		updateValue	( const QString & );
	// *** метка времени
	//! возвращает метку времени
			QDateTime	getTime		()			const;
	//! устанавливает метку времени
		virtual	void		setTime		( const QDateTime & );
	// *** состояние параметра
	//! возвращает состояние параметра
			EParStatus	status		()			const;
	//! устанавливает состояние параметра
			void		setStatus	( EParStatus s );

	//! запись параметра в объект сетевого пакета
		virtual	void		toSend		( pAnswer &, bool = false );
	//! установка флага отправки
	inline	virtual void		setSend	();
	//! сброс флага отправки
		virtual void		resetSend	( uint );
	//!
    		virtual bool		isNeedSend	( uint );
	// *** зависимость параметра
	//! добавляет параметр, состояние которого зависит от значения данного
		virtual void		addStatusDepPar	( CParameter * );
	//! добавляет обобщенный параметр
		virtual void		addCommonPar	( CParameter *, bool );
	//! 
	inline	virtual bool		isInvert	() { return m_bInvert; }
	//! добавляет параметр от которого зависит обобщенный
		virtual void		addCommonDepPar	( CParameter * );
	// *** отладка
	//! запись основных полей параметра в текстовый вид
		virtual	void		print		( QStringList & );
	// ***
private:
	// *** преобразование значения парамеметра ***
	//! преобразовывает строковое значение в тип Т
			T		getValue	( const QString & )	const;
	//! преобразовывает значение типа Т в строковое
			QString		getStringValue	( T )			const;
	// ***
protected:
	const CParameterDefinition	* m_pParameterInfo;		 //! первичная информация о параметре
	bool				  m_bInvert;
private:
	bool				  m_bModificationFlag;		 //! флаг изменения значения параметра
	QDateTime			  m_qtTime;			 //! метка времени значения
	T				  m_typeValue;			 //! значение параметра
	EParStatus			  m_eStatus;			 //! состояние параметра
	QMap<uint, bool>		  m_mapSendFlags;
	

	QList<CParameter*>		  m_qlistStatusDepPars;		 //! параметры, состояние которых зависит от значения данного

	QList<CParameter*>		  m_qlistCommonPars;		 //! обобщенные параметры
	QList<CParameter*>		  m_qlistCommonDepPars;		 //! параметры от которых зваисит обобщенный параметр
};

//! конструктор
template<class T> CParameterTemplate<T>::CParameterTemplate( const CParameterDefinition *pParameterInfo ) :
	m_pParameterInfo	( pParameterInfo ),
	m_bInvert		( false ),
	m_eStatus		( CParameter::ePUS_Unknown )
{
	foreach(uint i, m_pParameterInfo->m_pProps->m_qlistClientsId )
	    m_mapSendFlags[i] = false;
}

//! деструктор
template<class T> CParameterTemplate<T>::~CParameterTemplate(void) {
	delete m_pParameterInfo;
}

//! возвращает идентификатор
template<class T> inline ulong CParameterTemplate<T>::getId() const {

	return m_pParameterInfo->m_ulId;
}

//! возвращает строковый идентификатор
template<class T> inline QString CParameterTemplate<T>::getStrId() const {

	return CParameterProperties::toStr(m_pParameterInfo->m_ulId);
}

//! возвращает строковый идентификатор устройства
template<class T> inline QString CParameterTemplate<T>::getDevPath() const {

	return CParameterProperties::toStr(m_pParameterInfo->m_ulId).left(2*c_uiIdPrecision);
}

//! возвращает идентификатор ПТС
template<class T> inline ushort CParameterTemplate<T>::getSHWSId() const {

	return m_pParameterInfo->m_usSHWSId;
}

//! возвращает идентификатор устройства
template<class T> inline ushort CParameterTemplate<T>::getDevId() const {

	return m_pParameterInfo->m_usDevId;
}

//! возвращает номер параметра
template<class T> inline ushort CParameterTemplate<T>::getParId() const {

	return m_pParameterInfo->m_usParId;
}

//! возвращает тип параметра
template<class T> inline ePP_ValueType CParameterTemplate<T>::getValueType() const {

	return m_pParameterInfo->m_pProps->m_eValueType;
}

//! пересчет значения обобщенного параметра
template<class T> void CParameterTemplate<T>::update() {
}

template<> void CParameterTemplate<short>::update() {

	if( m_pParameterInfo->m_pProps->m_sParType == "common" ) {

		if( status() != CParameter::ePUS_Updated ) setSend();

		ushort usResult = 0;
		for( int i=0;i<m_qlistCommonDepPars.count();i++ ) {
			if( m_qlistCommonDepPars[i]->status() == CParameter::ePUS_Updated ) {
				
				usResult += m_qlistCommonDepPars[i]->isInvert() ? !getValue( m_qlistCommonDepPars[i]->getStringValue() ) :
						 getValue( m_qlistCommonDepPars[i]->getStringValue() );
			}
		}

		if( usResult != getValue( getStringValue() ) ) {
			m_typeValue = usResult;
			setTime( QDateTime::currentDateTime().toUTC() );
			setSend();
		}

		setStatus( CParameter::ePUS_Updated );
	}
}

//! возвращает значение параметра
template<class T> QString CParameterTemplate<T>::getStringValue() const {

	return getStringValue(m_typeValue);
}

//! спецификация функций для типа QString
template<> QString CParameterTemplate<QString>::getStringValue() const {

	return m_typeValue;
}

//! устанавливает значение параметра
template<class T> void CParameterTemplate<T>::setValue( const QString &value ) {

	m_typeValue = getValue( value );
	setSend();
}

//! устанавливает значение параметра
template<class T> void CParameterTemplate<T>::setValue( CParameter *pPar ) {

	setValue( pPar->getStringValue() );
}

//! обновляет значение параметра
template<class T> void CParameterTemplate<T>::updateValue( const QString &value ) {

	if( m_typeValue != getValue( value ) ) {
		m_typeValue = getValue( value );
		setTime( QDateTime::currentDateTime().toUTC() );
		setStatus( CParameter::ePUS_Updated );
		setSend();

		if( !m_typeValue ) {
			foreach( CParameter *pPar, m_qlistStatusDepPars ) {
				pPar->setStatus( CParameter::ePUS_Unknown );
				pPar->setSend();
			}
		}
	}
}

template<> void CParameterTemplate<QString>::updateValue( const QString &value ) {
}

template<class T> QDateTime CParameterTemplate<T>::getTime() const {

	return m_qtTime;
}

//! устанавливает метку времени
template<class T> void CParameterTemplate<T>::setTime( const QDateTime &qtTime ) {

	m_qtTime = qtTime;
}

//! возвращает состояние параметра в контексте периодического обновления
template<class T> CParameter::EParStatus CParameterTemplate<T>::status() const {

	return m_eStatus;
}

//! устанавливает состояние параметра
template<class T> void CParameterTemplate<T>::setStatus( EParStatus status ) {

	m_eStatus = status;

	foreach( CParameter *pPar, m_qlistCommonPars ) {
		if( pPar->status() != status || getValue( pPar->getStringValue() ) != m_bInvert ? !getValue( getStringValue() ) : getValue( getStringValue() ) )
			pPar->update();
	}
}

//! устанавливает состояние параметра
template<> void CParameterTemplate<QString>::setStatus( EParStatus status ) {
	m_eStatus = status;
}

//! запись параметра в объект сетевого пакета
template<class T> void CParameterTemplate<T>::toSend( pAnswer &_pa, bool bForce ) {

	if( bForce ) _pa.addVariable( pVariable( this ) );
	else 
	    if( m_pParameterInfo->m_pProps->m_bToSend )
		_pa.addVariable( pVariable(this) );
}

//! установка флага отправки
template<class T> void CParameterTemplate<T>::setSend() {

	foreach(uint i, m_pParameterInfo->m_pProps->m_qlistClientsId )
	    m_mapSendFlags[i] = true;
}

//! сброс флага отправки
template<class T> void CParameterTemplate<T>::resetSend( uint i ) {

	m_mapSendFlags[i] = false;
}

//!
template<class T> bool CParameterTemplate<T>::isNeedSend( uint i ) {

    return m_mapSendFlags[i];
}
//! добавляет параметр, состояние которого зависит от значения данного
template<class T> void CParameterTemplate<T>::addStatusDepPar( CParameter *pPar ) {

	m_qlistStatusDepPars.append( pPar );
}

//! добавляет обобщенный параметр
template<class T> void CParameterTemplate<T>::addCommonPar( CParameter *pPar, bool bInvert ) {

	m_qlistCommonPars.append( pPar );
	
	m_bInvert = bInvert;
}

//! добавляет параметр от которого зависит обобщенный
template<class T> void CParameterTemplate<T>::addCommonDepPar( CParameter *pPar ) {

	m_qlistCommonDepPars.append( pPar );
}

//! запись основных полей параметра в текстовый вид
template<class T> void CParameterTemplate<T>::print( QStringList &lstr ) {

	lstr << QString	("%1 | %2 | %3")
			.arg(getStrId())
			.arg(getStringValue(), 45)
			.arg(CParameter::EParStatusToQString(status()));
}

//! преобразовывает значение типа Т в строковое
template<class T> T CParameterTemplate<T>::getValue( const QString &value ) const {

	T typeValue;
	// строковый поток для чтения нового значения
	QString sValue = value;
	QTextStream stream(&sValue);
	// читаем значение 
	stream >> typeValue;

	return typeValue;
}

//! спецификация функций для типа QString
template<> QString CParameterTemplate<QString>::getValue( const QString &value ) const {

	return value;
}

//! преобразовывает значение типа Т в строковое
template<class T> QString CParameterTemplate<T>::getStringValue ( T value ) const {

	// строковый поток для чтения нового значения
	QString sValue; QTextStream stream(&sValue);
	// читаем значение 
	stream << value;

	return sValue;
}

//! спецификация функций для типа QString
template<> QString CParameterTemplate<QString>::getStringValue( QString value ) const {

	return value;
}

#endif

