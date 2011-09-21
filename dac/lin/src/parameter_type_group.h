//<codepage = utf8
/*!
	\file parameter_group.h
	\brief Описание класса CParameterGroup
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt4
	\warning Платформной зависимости не замечено
*/

#ifndef __PARAMETER_TYPE_GROUP_H__
#define __PARAMETER_TYPE_GROUP_H__

#include "parameter_type_template.h"
#include "parameter_creator.h"

/*! \class CParameterGroup
	\brief Группа параметров
*/
template<class T> class CParameterGroup : public CParameterTemplate<T> {
public:
	//! конструктор
	 CParameterGroup( const CParameterDefinition * );
	//! деструктор
	~CParameterGroup();
	// *** интерфейсные функции ***
	// свойства параметра из его описания
	//! параметр или группа
	bool		isGroup		()				const;
	// ***
	// *** свойства параметра ***
	// *** значение
	//! применяет значение параметра
	void		applyValue	();
	// *** метка времени
	//! устанавливает метку времени
	void		setTime		( const QDateTime & );
	// *** флаг изменения значения
//	//! возвращает состояние флага изменения значения
//	bool		isModified	()				const;
//	//! сбрасывает флаг изменения значения
//	void		resetModified	();
	// *** флаг посылки значения клиенту
	//! возвращает флаг посылки значения клиенту
	bool		isNeedSend	( uint ) 			const;
	//! сбрасывает флаг посылки значения клиенту
	void		resetNeedSend	( uint );
	// *** хеш параметров от которых зависит данный параметр
	//! добавляет указатель на параметр в хэш параметров от которого завист данный
	void  		setMathDepPar	( CParameter *, ulong = 0 );
	// *** передача параметра
	//! запись параметра в объект сетевого пакета
	void 		toSend		( pAnswer & );
	// *** отладка
	//! запись основных полей параметра в текстовый вид
	void		print		( QStringList & );
	// ***
	// *** свойства шаблонного параметра ***
	//! возвращает параметр по идентификатору
	CParameter *	getById		( ulong )			const;
	//! возвращает параметр по значению
	CParameter * 	getByVal	( const QString & )		const;
	//! возвращает количество обновленных параметров
	uint 		getNumOfUpdated	()				const;
	//! возвращает идентификатор группы для идентификации параметров данной группы
	ulong		getIdentId	()				const;
	//! возвращает номер первого параметра в группе 
	uint		getBasePar	()				const;
	//! возвращает максимальное количество параметров данной группы 
	uint		getMaxParCount	()				const;
private:
	QMap<ulong, CParameter *>	 m_mapParameters; //! параметры группы
	const CGroupDefinition 		*m_pGroupInfo;    //! первичная информация о группе
};

//! конструктор
template<class T> CParameterGroup<T>::CParameterGroup( const CParameterDefinition *pGroupInfo ) :
	CParameterTemplate<T>(reinterpret_cast<const CGroupDefinition*>(pGroupInfo)->m_pDefinition),
	m_pGroupInfo(reinterpret_cast<const CGroupDefinition*>(pGroupInfo))
{
	CParameterBuilder builder;
	QList<CParameterDefinition *>::const_iterator p, e = m_pGroupInfo->m_listParDefinition.end();
	for(p=m_pGroupInfo->m_listParDefinition.begin();p!=e;++p) {

		m_mapParameters[(*p)->m_ulId] = builder.build	(
					ePT_Value,
					m_pGroupInfo->m_pDefinition->m_pProps->m_eValueType,
					*p
								);
	}
}

//! деструктор
template<class T> CParameterGroup<T>::~CParameterGroup() {

	// удаляем содержимое
	QMap<ulong, CParameter *>::iterator p, e = m_mapParameters.end();
	for(p=m_mapParameters.begin();p!=e;++p)
		delete p.value();
}

//! параметр или группа
template<class T> bool CParameterGroup<T>::isGroup() const {

	return true;
}

//! применяет значение параметра
template<class T> void CParameterGroup<T>::applyValue() {

	QMap<ulong, CParameter *>::iterator p, e = m_mapParameters.end();
	bool bStatus = false;
	for(p=m_mapParameters.begin();p!=e;++p) {
		p.value()->applyValue();
		if( p.value()->status()  == CParameter::ePUS_Updated )
			bStatus = true;
	}

	CParameterTemplate<T>::setStatus(CParameter::ePUS_NotUpdated);

	if(bStatus) CParameterTemplate<T>::setStatus(CParameter::ePUS_Updated);
}

//! устанавливает метку времени
template<class T> void CParameterGroup<T>::setTime( const QDateTime &qtTime ) {

	QMap<ulong, CParameter *>::iterator p, e = m_mapParameters.end();
	for(p=m_mapParameters.begin();p!=e;++p)
		(*p)->setTime(qtTime);
}

////! возвращает состояние флага изменения значения
//template<class T> bool CParameterGroup<T>::isModified() const {
//
//	QMap<ulong, CParameter *>::const_iterator p, e = m_mapParameters.end();
//	bool bIsModified = false;
//	for(p=m_mapParameters.begin();p!=e;++p) {
//		if((*p)->isModified()) {
//			bIsModified = true;
//			break;
//		}
//	}
//	return bIsModified;
//}

////! сбрасывает флаг изменения значения
//template<class T> void CParameterGroup<T>::resetModified() {
//
//	QMap<ulong, CParameter *>::iterator p, e = m_mapParameters.end();
//	for(p=m_mapParameters.begin();p!=e;++p)
//		(*p)->resetModified();
//}

//! возвращает флаг посылки значения клиенту
template<class T> bool CParameterGroup<T>::isNeedSend( uint id ) const {

	QMap<ulong, CParameter *>::const_iterator p, e = m_mapParameters.end();
	bool bNeedSend = false;
	for(p=m_mapParameters.begin();p!=e;++p) {
		if( p.value()->isNeedSend(id) ) {
			bNeedSend = true;
			break;
		}
	}

	return bNeedSend;
}

//! сбрасывает флаг посылки значения клиенту
template<class T> void CParameterGroup<T>::resetNeedSend( uint id ) {

	QMap<ulong, CParameter *>::iterator p, e = m_mapParameters.end();
	for(p=m_mapParameters.begin();p!=e;++p)
		p.value()->resetNeedSend(id);
}

//! устанавливает указатель на параметр от которого зависит вычисление значения данного параметра
template<class T> void CParameterGroup<T>::setMathDepPar( CParameter *pPar, ulong ulId ) {

	CParameterGroup *PGrPar = reinterpret_cast<CParameterGroup *>(pPar);
	QMap<ulong, CParameter *>::iterator p, e = PGrPar->m_mapParameters.end();
	for(p=PGrPar->m_mapParameters.begin();p!=e;++p) {

		QString sId = QString("%1%2")
			.arg(CParameterTemplate<T>::getDevPath())
			.arg(getBasePar() + p.value()->getParId() - PGrPar->getBasePar());

		getById(CParameterProperties::toULong(sId))->setMathDepPar(p.value(), pPar->getId());
	}
}

//! запись параметра в объект сетевого пакета
template<class T> void CParameterGroup<T>::toSend( pAnswer &_pa ) {

	QMap<ulong, CParameter *>::iterator p, e = m_mapParameters.end();
	for(p=m_mapParameters.begin();p!=e;++p)
		(*p)->toSend(_pa);
}

//! запись основных полей параметра в текстовый вид
template<class T> void CParameterGroup<T>::print( QStringList &lstr ) {

	QMap<ulong, CParameter *>::iterator p, e = m_mapParameters.end();
	for(p=m_mapParameters.begin();p!=e;++p)
		p.value()->print(lstr);
}

//! возвращает параметр по идентификатору
template<class T> CParameter * CParameterGroup<T>::getById( ulong ulId ) const {

	if(!m_mapParameters.contains(ulId)) return 0;

	return m_mapParameters[ulId];
}

//! возвращает параметр по значению
template<class T> CParameter * CParameterGroup<T>::getByVal( const QString &sVal ) const {

	if(sVal.isEmpty()) return 0;

	QMap<ulong, CParameter *>::const_iterator p, e = m_mapParameters.end();
	for(p=m_mapParameters.begin();p!=e;++p)
		if( (*p)->getStringValue().indexOf(sVal) != -1) break;

	if( p != m_mapParameters.end() ) return *p;

	return 0;
}

//! возвращает количество обновленных параметров
template<class T> uint CParameterGroup<T>::getNumOfUpdated () const {

	uint uiCounter = 0;
	QMap<ulong, CParameter *>::const_iterator p, e = m_mapParameters.end();
	for(p=m_mapParameters.begin();p!=e;++p)
		if(p.value()->status() == CParameter::ePUS_Updated)
			uiCounter++;

	return uiCounter;
}

//! возвращает идентификатор группы для идентификации параметров данной группы
template<class T> ulong CParameterGroup<T>::getIdentId() const {

	return m_pGroupInfo->m_ulIdentId;
}

//! возвращает номер первого параметра в группе 
template<class T> uint CParameterGroup<T>::getBasePar() const {

	return m_pGroupInfo->m_uiBaseParNumber;
}

//! возвращает максимальное количество параметров данной группы 
template<class T> uint CParameterGroup<T>::getMaxParCount() const {

	 return m_pGroupInfo->m_uiParMaxCount;
}

#endif

