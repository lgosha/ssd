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
	//! возвращает период обновления
	inline		ushort		getUpdateTime	()			const;
	//! возвращает тип запроса значения параметра
	inline		ePP_QueryType	getQueryType	()			const;
	//! возвращает тип значения параметра
	inline 		ePP_ValueType	getValueType	()			const;
	//! возвращает задержку определения значения параметра
			int		getStart	()			const;
	//! возвращает адрес запроса значения параметра
	inline 		QString		getAddr		()			const;
	//! возвращает свойство класса запроса значения параметра
	inline 		QString		getProperty	()			const;
	//! возвращает идентификатор параметра от которого зависит значение
			ulong		getDepValParId	()			const;
	//! возвращает период вычисления значения
	inline	virtual	uint		getDelay	()			const;
	//! синхронный или асинхронный запрос
	inline		bool		isSync()				const;
	//! параметр или группа
	inline		bool		isGroup()				const;
	// ***
	// *** свойства параметра ***
	// *** значение
	//! возвращает значение параметра
			QString		getStringValue	()			const;
	//! устанавливает значение параметра
		virtual	void		setValue	( const QString &, bool = true);
	//! устанавливает значение параметра
		virtual	void		setValue	( CParameter *, bool = true );
	//! применяет значение параметра
		virtual	void		applyValue	();
	// *** метка времени
	//! возвращает метку времени
			QDateTime	getTime		()			const;
	//! устанавливает метку времени
		virtual	void		setTime		( const QDateTime & );
	// *** флаг посылки значения клиенту
	//! возвращает флаг посылки значения клиенту
		virtual	bool		isNeedSend	( uint )		const;
	//! устанавливает флаг посылки значения всем клиентам
		virtual	void		setNeedSendAll	();
	//! сбрасывает флаг посылки значения клиенту
		virtual	void		resetNeedSend	( uint );
	// *** состояние параметра
	//! возвращает состояние параметра
			EParStatus	status		()			const;
	//! возвращает реальное состояние параметра
			EParStatus	rstatus		()			const;
	//! устанавливает состояние параметра в CParameter::ePUS_NotUpdated
			void		setNotUpdated	();
	// *** флаг обновления значения
	//! возвращает флаг обновления значения
			bool		isNeedUpdate	()			const;
	//! устанавливает флаг обновления значения
			void		setNeedUpdate	( bool );
	// *** флаг безусловного обновления значения
	//! возвращает флаг безусловного обновления значения
			bool		isForceUpdate	()			const;
	//! устанавливает флаг безусловного обновления значения
			void		setForceUpdate	( bool );

	// *** зависимости
	//! *** математическое выражения
	//! добавляет указатель на параметр от которого зависит вычисление математического выражения данного
	virtual void			setMathDepPar	( CParameter *, ulong = 0 );
	//! добавляет указатель на параметр, вычисление математического выражения которого зависит от данного
	virtual void			addMathDepPar	( CParameter * );
	//! *** состояние
	//! добавляет указатель на параметр, состояние которого зависит от данного
	virtual void			addStatusDepPar	( CParameter * );

	// *** передача параметра
	//! запись параметра в объект сетевого пакета
		virtual	void		toSend		( pAnswer &, bool = false );

	// *** сравнение
	//! логическое сравнение
		virtual bool		compare		( const QString &, const QString & );
	// ***

	// *** отладка
	//! запись основных полей параметра в текстовый вид
		virtual	void		print		( QStringList & );
	// ***
protected:
	//! устанавливает состояние параметра
			void		setStatus	( EParStatus s );
private:
	// *** применение правил обработки значения параметра ***
	//! применение правил регулярных выражений
			void		applyRegexpRules();
	//! применение математических правил
			int		applyMathRules	();
	//! применение одного математического правила
			QString		processMathRule	( QString & );
	//! вычисление математического правила
			QString		calcMathRule	( const QString &, const QString &, char );
	//! применение лолгических выражений
			int		applyLogicRules	();
	// ***
	// *** преобразование значения парамеметра ***
	//! преобразовывает строковое значение в тип Т
			T		getValue	( const QString & )	const;
	//! преобразовывает значение типа Т в строковое
			QString		getStringValue	( T )			const;
	// ***
	//! установка значения параметра с применением правил обработки значения, обновления флага изменения значения
	//! и обновления хеша необходимости посылки значения клиентам
			int		setit		( const QString & );

protected:
	const CParameterDefinition	*m_pParameterInfo;		 //! первичная информация о параметре
private:
	QMap<uint, bool>		 m_mapSendFlags;		 //! map флагов необходимости посылки значения клиенту
	bool				 m_bUpdate;			 //! флаг обновления параметра
	QDateTime			 m_qtTime;			 //! метка времени значения
	EParStatus			 m_iUpdateStatus;		 //! состояние значения параметра
	EParStatus			 m_iRStatus;			 //! состояние значения параметра (реальное)
	T				 m_typeValue;			 //! значение параметра
	T				 m_typeNewValue;		 //! новое значение параметра
	QString				 m_sNewValue;			 //! строковое новое значение параметра
	bool				 m_bIsNew;			 //! флаг получения нового значение параметра
	QMap<ulong, CParameter*>	 m_mapMathDepend;		 //! параметры для вычисления значения
	bool				 m_bForceUpdate;		 //! безусловное обновление параметра на следующем такте
	QList<CParameter *>		 m_lStatusDepend;		 //! список параметров, состояние которых зависит от данного
	QList<CParameter *>		 m_lMathDepend;			 //! список параметров, математическое выражение которых зависит от данного
};

//! конструктор
template<class T> CParameterTemplate<T>::CParameterTemplate( const CParameterDefinition *pParameterInfo ) :
	m_pParameterInfo	( pParameterInfo ),
	m_bUpdate		( false ),
	m_iUpdateStatus		( CParameter::ePUS_Unknown ),
	m_bIsNew		( false ),
	m_bForceUpdate		( false )

{
	foreach(uint i, m_pParameterInfo->m_pProps->m_qlistClientId)
		m_mapSendFlags[i] = false;

	// разделяем правила по разделителю '...'
	QStringList lrules = m_pParameterInfo->m_pProps->m_sMath.split("...");
	// перебираем правила
	foreach(QString str, lrules) {
		// ищем параметры
		int iPos = -1;
		while( (iPos = str.indexOf(',', iPos + 1 )) != -1 )
			m_mapMathDepend[CParameterProperties::toULong(str.mid(iPos + 1, 9))] = 0;
	}
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

//! возвращает идентификатор нити
template<class T> inline ushort CParameterTemplate<T>::getUpdateTime() const {

	return m_pParameterInfo->m_pProps->m_uiUpdateTime;
}

//! возвращает тип запроса значения параметра
template<class T> inline ePP_QueryType CParameterTemplate<T>::getQueryType() const {

	return m_pParameterInfo->m_pProps->m_eQueryType;
}

//! возвращает тип параметра
template<class T> inline ePP_ValueType CParameterTemplate<T>::getValueType() const {

	return m_pParameterInfo->m_pProps->m_eValueType;
}

//! возвращает задержку определения значения параметра
template<class T> int CParameterTemplate<T>::getStart() const {

	return m_pParameterInfo->m_pProps->m_iStart;
}

//! возвращает адрес запроса значения параметра
template<class T> inline QString CParameterTemplate<T>::getAddr() const {

	return m_pParameterInfo->m_pProps->m_sAddr;
}

//! возвращает свойство класса запроса значения параметра
template<class T> inline QString CParameterTemplate<T>::getProperty() const {

	return m_pParameterInfo->m_pProps->m_sProperty;
}

//! возвращает идентификатор параметра от которого зависит значение данного
template<class T> ulong CParameterTemplate<T>::getDepValParId() const {

	if( !m_pParameterInfo->m_pProps->m_sValDepParId.isEmpty() )
		return CParameterProperties::toULong( m_pParameterInfo->m_pProps->m_sValDepParId );

	return 0;
}

//! возвращает период вычисления значения
template<class T> inline uint CParameterTemplate<T>::getDelay() const {

	return 0;
}

//! синхронный или асинхронный запрос параметра
template<class T> inline bool CParameterTemplate<T>::isSync() const {

	return m_pParameterInfo->m_pProps->m_bSync;
}

//! параметр или группа
template<class T> inline bool CParameterTemplate<T>::isGroup() const {

	return false;
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
template<class T> void CParameterTemplate<T>::setValue( const QString &value, bool bSet ) {

	if( bSet ) {

		// если нет фиксированного значения
		if( m_pParameterInfo->m_pProps->m_sGoodValue.isNull() )
			// сохраняем значение
			m_sNewValue = value;
		else m_sNewValue = m_pParameterInfo->m_pProps->m_sGoodValue;
	}

	// значение получено
	m_bIsNew = true;
}

//! спецификация функций для типа QString
template<> void CParameterTemplate<QString>::setValue( const QString &value, bool bSet ) {

	if( bSet )
		m_sNewValue = value;

	m_bIsNew = true;
}

//! устанавливает значение параметра
template<class T> void CParameterTemplate<T>::setValue( CParameter *pPar, bool bSet ) {

	setValue( pPar->getStringValue(), bSet );
}

//! применение значение параметра (фактическая установка значения)
template<class T> void CParameterTemplate<T>::applyValue() {

	if( m_bIsNew ) {
		if( setit( m_sNewValue ) ) { 
			if( status() != CParameter::ePUS_Updated ) { 
				setStatus( CParameter::ePUS_Updated );
				setNeedSendAll();
			}
		}
		// инициируем принудительное обновление параметров, состояние которых зависит от данного
		foreach( CParameter *pPar, m_lStatusDepend )
			if( pPar->status() == CParameter::ePUS_NotUpdated ) pPar->setForceUpdate( true );
	}
	else {
		if(m_pParameterInfo->m_pProps->m_sFailValue.isNull()) {

			if( status() != CParameter::ePUS_NotUpdated ) {
				setStatus(CParameter::ePUS_NotUpdated);
				setNeedSendAll();
			}

			// устанавливаем состояние в CParameter::ePUS_NotUpdated для параметров, математическое выражение которых зависит от данного
			foreach( CParameter *pPar, m_lMathDepend ) {
				if( pPar->status() != CParameter::ePUS_NotUpdated ) {
					pPar->setNotUpdated();
					pPar->setNeedSendAll();
				}
			}
		}
		else {
			setit(m_pParameterInfo->m_pProps->m_sFailValue);
			setStatus(CParameter::ePUS_Updated); m_iRStatus = CParameter::ePUS_NotUpdated;
			// устанавливаем состояние в CParameter::ePUS_NotUpdated для параметров, состояние которых зависит от данного
			foreach( CParameter *pPar, m_lStatusDepend ) {
				if( pPar->status() != CParameter::ePUS_NotUpdated && !m_lMathDepend.contains( pPar ) ) {
					pPar->setNotUpdated();
					pPar->setNeedSendAll();
				}
			}
		}
	}

	m_bIsNew = false;
}

//! возвращает метку времени
template<class T> QDateTime CParameterTemplate<T>::getTime() const {

	return m_qtTime;
}

//! устанавливает метку времени
template<class T> void CParameterTemplate<T>::setTime( const QDateTime &qtTime ) {

	m_qtTime = qtTime;

	// обновляем параметры, математическое выражение которых зависит от данного
	foreach( CParameter *pPar, m_lMathDepend ) {
		pPar->setValue( this, false );
		pPar->applyValue();
		if( pPar->status() == CParameter::ePUS_Updated )
			pPar->setTime( qtTime );
	}
}

//! флаг необходимости посылки значения клиенту
template<class T> bool CParameterTemplate<T>::isNeedSend(uint id) const {

	return m_mapSendFlags[id];
}

//! устанавливает флаг посылки значения всем клиентам
template<class T> void CParameterTemplate<T>::setNeedSendAll() {

	foreach( uint uiId, m_pParameterInfo->m_pProps->m_qlistClientId )
		m_mapSendFlags[uiId] = true;
}

//! снимает флаг необходимости посылки значения клиенту
template<class T> void CParameterTemplate<T>::resetNeedSend(uint id) {

	m_mapSendFlags[id] = false;
}

//! возвращает состояние параметра в контексте периодического обновления
template<class T> CParameter::EParStatus CParameterTemplate<T>::status() const {

	return m_iUpdateStatus;
}

//! возвращает состояние параметра в контексте периодического обновления для параметров
//! с полями fval и gval состояние которых не бывает CParameter::ePUS_NotUpdated 
template<class T> CParameter::EParStatus CParameterTemplate<T>::rstatus() const {

	return m_iRStatus;
}

//! устанавливает состояние параметра в CParameter::ePUS_NotUpdated 
template<class T> void CParameterTemplate<T>::setNotUpdated() {

	m_iUpdateStatus = ePUS_NotUpdated;
}

//! возвращает флаг необходимости обновления
template<class T> bool CParameterTemplate<T>::isNeedUpdate() const {

	return m_bUpdate;
}

//! устанавливает параметр для обновления
template<class T> void CParameterTemplate<T>::setNeedUpdate( bool bUpdate ) {

	m_bUpdate = bUpdate;
}

//! возвращает флаг  необходимости безусловного обновления
template<class T> bool CParameterTemplate<T>::isForceUpdate() const {

	return m_bForceUpdate;
}

//! устанавливает параметр для безусловного обновления
template<class T> void CParameterTemplate<T>::setForceUpdate( bool bUpdate ) {

	m_bForceUpdate = bUpdate;
}

//! устанавливает указатель на параметр от которого зависит данный параметр
template<class T> void CParameterTemplate<T>::setMathDepPar( CParameter *pPar, ulong ulId ) {

	ulong ulTmp;
	!ulId ? ulTmp = pPar->getId() : ulTmp = ulId;
	if( m_mapMathDepend.contains(ulTmp) ) m_mapMathDepend[ulTmp] = pPar;
}

//! добавляет указатель на параметр, вычисление математического выражения которого зависит от данного
template<class T> void CParameterTemplate<T>::addMathDepPar( CParameter *pPar ) {

	m_lMathDepend.append( pPar );
}

//! добавляет указатель на параметр, состояние которого зависит от данного
template<class T> void CParameterTemplate<T>::addStatusDepPar( CParameter *pPar ) {

	m_lStatusDepend.append( pPar );
}

//! запись параметра в объект сетевого пакета
template<class T> void CParameterTemplate<T>::toSend( pAnswer &_pa, bool bForce ) {

	if( m_pParameterInfo->m_pProps->m_bToSend || bForce ) _pa.addVariable( pVariable(this) );
}

//! запись основных полей параметра в текстовый вид
template<class T> void CParameterTemplate<T>::print( QStringList &lstr ) {

	QString sTmp = QString::null;
	foreach( uint uiId, m_pParameterInfo->m_pProps->m_qlistClientId )
		sTmp += QString("%1,%2 - ").arg(uiId).arg(m_mapSendFlags[uiId]);

	lstr << QString	("%1 | %2 | %3 | %4 | %5 | %6 | %7 | %8")
			.arg(getId())
			.arg(getStringValue(), 45)
			.arg(CParameter::EParStatusToQString(status()))
			.arg(getQueryType(), 10)
			.arg(m_pParameterInfo->m_pProps->m_bToSend ? "send" : "no" )
			.arg( isForceUpdate() )
			.arg( getStart() )
			.arg( sTmp );
}

//! устанавливает состояние параметра
template<class T> void CParameterTemplate<T>::setStatus ( EParStatus status ) {

	m_iUpdateStatus = status;
	m_iRStatus = status;
}

//! применение правил регулярных выражений
template<class T> void CParameterTemplate<T>::applyRegexpRules() {

	// есть правила
	if( !m_pParameterInfo->m_pProps->m_sRexp.isEmpty() ) {

		// разделяем правила по разделителю '...'
		QStringList lrules = m_pParameterInfo->m_pProps->m_sRexp.split("...");
		// перебираем правила
		for(QStringList::iterator it = lrules.begin(); it != lrules.end(); ++it)
			m_sNewValue = m_sNewValue.remove(QRegExp(*it));
	}
}

//! применение математических правил
template<class T> int CParameterTemplate<T>::applyMathRules() {

	// есть правила
	if( !m_pParameterInfo->m_pProps->m_sMath.isEmpty() ) {

		bool bIsUnknown = false;
		bool bIsNotUpdated = false;
		foreach( CParameter *pPar, m_mapMathDepend.values() ) {
			if( pPar->status() == CParameter::ePUS_Unknown ) bIsUnknown = true;
			if( pPar->status() == CParameter::ePUS_NotUpdated ) {
				bIsNotUpdated = true;
				break;
			}
		}

		if( bIsNotUpdated ) { 
			if( status() != CParameter::ePUS_NotUpdated ) {
				setNotUpdated();
				setNeedSendAll();
			}
			return 0;
		}
		if( bIsUnknown ) {  
			if( status() != CParameter::ePUS_Unknown ) {
				setStatus( CParameter::ePUS_Unknown );
				setNeedSendAll();
			}
			return 0;
		 }

		// разделяем правила по разделителю '...'
		QStringList lrules = m_pParameterInfo->m_pProps->m_sMath.split("...");
		// перебираем правила
		for(QStringList::iterator it = lrules.begin(); it != lrules.end(); ++it)
			// применяем правило
			m_sNewValue = processMathRule(*it);
	}

	return 1;
}

#define _toULong(X) CParameterProperties::toULong(X)

//! применение одного математического правила
template<class T> QString CParameterTemplate<T>::processMathRule( QString &expr ) {

	expr = expr.remove(QRegExp("^\\s+"));
	expr = expr.remove(QRegExp("\\s+$"));

	QRegExp rexp("^\\(");
	if(rexp.indexIn(expr) != -1) {
		expr = expr.remove(rexp);
		expr = expr.remove(QRegExp("\\)$"));
	}

	QMap<char, int> mapSupportedSymbols;
	mapSupportedSymbols[','] = 1;
	mapSupportedSymbols[';'] = 1;
	mapSupportedSymbols['('] = 1;
	mapSupportedSymbols[')'] = 1;
	mapSupportedSymbols['+'] = 1;
	mapSupportedSymbols['-'] = 1;
	mapSupportedSymbols['*'] = 1;
	mapSupportedSymbols['/'] = 1;

	QTextStream stream(&expr);

	char cTmp; QString right, left; QString sTmp;
	char op = 0;
	while( !stream.atEnd() ) {

		stream.skipWhiteSpace();

		stream >> cTmp;

		if( !mapSupportedSymbols.contains(cTmp) ) continue;

		if( cTmp == ',' ) {

			stream >> sTmp;

			if(left.isEmpty()) { left = m_mapMathDepend[_toULong(sTmp)]->getStringValue(); continue; }

			right = m_mapMathDepend[_toULong(sTmp)]->getStringValue();
		}
		else {
			if(cTmp == ';' ) {

				stream >> sTmp;

				if(left.isEmpty()) { left = sTmp; continue; }

				right = sTmp;
			}
			else {

				if(cTmp == '(') {

					sTmp = stream.string()->mid(stream.pos(), stream.string()->lastIndexOf(')') - stream.pos());

					right = processMathRule(sTmp);

					expr = stream.string()->mid(stream.string()->lastIndexOf(')') + 1);
					stream.setString(&expr);
				}
				else {
					op = cTmp;
					if(left.isEmpty()) left = m_sNewValue;
				}
			}
		}

		if( !left.isEmpty() && !right.isEmpty() ) {
			left = calcMathRule(left, right, op); 
			right = "";
		}
	}

	return left;
}

//! вычисление математического правила
template<class T> QString CParameterTemplate<T>::calcMathRule( const QString & left, const QString & right, char op ) {

	// в завсисимости от операции
	switch(op) {
		case '+':
			return getStringValue(getValue(left) + getValue(right));
		break;
		case '-':
			return getStringValue(getValue(left) - getValue(right));
		break;
		case '*':
			return getStringValue(getValue(left) * getValue(right));
		break;
		case '/':
			return getStringValue(getValue(left) / getValue(right));
		break;
	}
	// операция не поддерживается
	return QString::null;
}

//! спецификация функций для типа QString
template<> QString CParameterTemplate<QString>::calcMathRule(const QString &left, const QString & right, char op) {

	return m_sNewValue;
}

#include "ulog.h"

//! применение лолгических выражений
template<class T> int CParameterTemplate<T>::applyLogicRules() {

	m_sNewValue = "0";

#define pPar m_mapMathDepend.begin().value()
#define exprArr m_pParameterInfo->m_pProps->m_vExpressions

	// проверки
	if( !m_mapMathDepend.count() ) {
		if( status() != CParameter::ePUS_Unknown ) {
			setStatus( CParameter::ePUS_Unknown );
			setNeedSendAll();
		}
		return 0;
	}
	if( pPar->status() == CParameter::ePUS_Unknown ) { 
		if( status() != CParameter::ePUS_Unknown ) {
			setStatus( CParameter::ePUS_Unknown );
			setNeedSendAll();
		}
		return 0;
	}
	if( pPar->status() == CParameter::ePUS_NotUpdated ) {
		if( status() != CParameter::ePUS_Unknown ) {
			setNotUpdated(); return 0;
			setNeedSendAll();
		}
		return 0;
	}
	// вычисление выражения
	bool bResult = false;
	for( int i=0;i<exprArr.count();i++ ) {
		if(!i) bResult = pPar->compare( exprArr[i].second, exprArr[i].first );
		else {
			if( m_pParameterInfo->m_pProps->m_vLogicOperations[i-1] == "&&" ) bResult = bResult && pPar->compare( exprArr[i].second, exprArr[i].first );
			if( m_pParameterInfo->m_pProps->m_vLogicOperations[i-1] == "||" ) bResult = bResult || pPar->compare( exprArr[i].second, exprArr[i].first );
		}
	}

	if( bResult ) m_sNewValue = "1";

	return 1;

#undef pPar
#undef exprArr
}

//! логическое сравнение
template<class T> bool CParameterTemplate<T>::compare( const QString &sVal, const QString &sOp ) {

	if( sOp == ">"  ) return getValue( getStringValue() ) > getValue( sVal );
	if( sOp == "<"  ) return getValue( getStringValue() ) < getValue( sVal );
	if( sOp == ">=" ) return getValue( getStringValue() ) >= getValue( sVal );
	if( sOp == "<=" ) return getValue( getStringValue() ) <= getValue( sVal );
	if( sOp == "==" ) return getValue( getStringValue() ) == getValue( sVal );

	return false;
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
template<class T> QString CParameterTemplate<T>::getStringValue( T value ) const {

	// строковый поток для чтения нового значения
	QString sValue; QTextStream stream(&sValue);
	// читаем значение 
	stream << value;

	return sValue;
}

//! спецификация функций для типа QString
template<> QString CParameterTemplate<QString>::getStringValue(QString value) const {

	return value;
}

//! установка значения параметра с применением правил обработки значения, обновления флага изменения значения
//! и обновления хеша необходимости посылки значения клиентам
template<class T> int CParameterTemplate<T>::setit( const QString &value ) {

	int iRes = 1;
	if( !m_pParameterInfo->m_pProps->m_vExpressions.count() ) {

		m_sNewValue = value;

		// применяем rexp правила
		applyRegexpRules();

		// применяем математические правила
		iRes = applyMathRules();
	}
	else iRes = applyLogicRules();

	if( iRes ) {
		// интерпретируем значение из строкового представления в представление текущего типа
		QMap<QString, QString>::const_iterator p;
		if((p = m_pParameterInfo->m_pProps->m_qmapValuesList.find(m_sNewValue)) != m_pParameterInfo->m_pProps->m_qmapValuesList.end())
			m_typeNewValue = getValue(p.value());
		else
			m_typeNewValue = getValue(m_sNewValue);
		
		if( m_pParameterInfo->m_pProps->m_bInvert ) m_typeNewValue = !m_typeNewValue;

		// сравнение с предыдущим и установка флага модификации
		if( m_typeNewValue != m_typeValue ) setNeedSendAll();

		// устанавливаем новое значение
		m_typeValue = m_typeNewValue;
	}

	return iRes;
}

//! установка значения параметра с применением правил обработки значения, обновления флага изменения значения
//! и обновления хеша необходимости посылки значения клиентам
template<> int CParameterTemplate<QString>::setit( const QString &value ) {

	int iRes = 1;
	if( !m_pParameterInfo->m_pProps->m_vExpressions.count() ) {

		m_sNewValue = value;

		// применяем rexp правила
		applyRegexpRules();

		// применяем математические правила
		iRes = applyMathRules();
	}
	else iRes = applyLogicRules();

	if( iRes ) {
		// интерпретируем значение из строкового представления в представление текущего типа
		QMap<QString, QString>::const_iterator p;
		if((p = m_pParameterInfo->m_pProps->m_qmapValuesList.find(m_sNewValue)) != m_pParameterInfo->m_pProps->m_qmapValuesList.end())
			m_typeNewValue = getValue(p.value());
		else
			m_typeNewValue = getValue(m_sNewValue);
		
		// сравнение с предыдущим и установка флага модификации
		if( m_typeNewValue != m_typeValue ) setNeedSendAll();

		// устанавливаем новое значение
		m_typeValue = m_typeNewValue;
	}

	return iRes;
}

#endif

