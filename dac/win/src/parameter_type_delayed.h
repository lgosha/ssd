//<codepage = utf8
/*!
	\file parameter_delayed.h
	\brief Описание класса CParameterDelayed
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt4
	\warning Платформной зависимости не замечено
*/

#ifndef __PARAMETER_TYPE_DELAYED_H__
#define __PARAMETER_TYPE_DELAYED_H__

#include "parameter_type_template.h"

/*! \class CParameterDelayed
	\brief Параметр для определения динамики значений
*/

template<class T> class CParameterDelayed : public CParameterTemplate<T> {
public:
	//! конструктор
	CParameterDelayed( const CParameterDefinition * );
	//! деструктор
	~CParameterDelayed();
	// *** интерфейсные функции ***
	// *** свойства параметра из его описания ***
	//! возвращает период вычисления значения
	uint getDelay		()			const;
	// ***
	// *** свойства параметра ***
	//! устанавливает значение параметра
	void setValue		( const QString & );
	//! применяет значение параметра
	void applyValue		();
	//! устанавливает флаг обновления значения
	void setNeedUpdate	( bool );
	// ***
private:
	CParameterTemplate<T> 	*m_pValue1;	//! значение 1 (перед задержкой m_uiDelay)
	CParameterTemplate<T> 	*m_pValue2;	//! значение 2 (после задержки m_uiDelay)
	uint			 m_uiDelay;	//! период вычисления значения
};

//! конструктор
template<class T> CParameterDelayed<T>::CParameterDelayed(const CParameterDefinition *pParameterInfo) : 
	CParameterTemplate<T>(pParameterInfo),
	m_uiDelay(pParameterInfo->m_pProps->m_uiDelay)	
{
	m_pValue1 = new CParameterTemplate<T>(new CParameterDefinition(pParameterInfo));
	m_pValue2 = new CParameterTemplate<T>(new CParameterDefinition(pParameterInfo));
}

//! деструктор
template<class T> CParameterDelayed<T>::~CParameterDelayed() {

	delete m_pValue1; delete m_pValue2;
}

//! возвращает период вычисления значения
template<class T> uint CParameterDelayed<T>::getDelay() const {

	return m_uiDelay;
}

//! устанавливает значение параметра
template<class T> void CParameterDelayed<T>::setValue(const QString &value) {

	if( CParameterTemplate<T>::status() == CParameter::ePUS_UpdateStart )
		m_pValue1->setValue(value);

	if( CParameterTemplate<T>::status() == CParameter::ePUS_UpdateStop )
		m_pValue2->setValue(value);
}

//! применяет значение параметра
template<class T> void CParameterDelayed<T>::applyValue() {

	if( CParameterTemplate<T>::status() == CParameter::ePUS_UpdateStart )
		m_pValue1->applyValue();

	if( CParameterTemplate<T>::status() == CParameter::ePUS_UpdateStop ) {

		m_pValue2->applyValue();

		if( m_pValue1->status() == CParameter::ePUS_Updated && m_pValue2->status() == CParameter::ePUS_Updated ) {

			T v1, v2; QString sTmp;
			sTmp = m_pValue1->getStringValue(); QTextStream s1(&sTmp); s1 >> v1;
			sTmp = m_pValue2->getStringValue(); QTextStream s2(&sTmp); s2 >> v2;
			T v = v2 - v1;
			QTextStream s3(&sTmp); s3 << v;

			CParameterTemplate<T>::setValue(sTmp);
			CParameterTemplate<T>::applyValue();

			CParameterTemplate<T>::setStatus(CParameter::ePUS_Updated);
		}
		else
			CParameterTemplate<T>::setStatus(CParameter::ePUS_NotUpdated);
	}
}

//! устанавливает флаг обновления значения
template<class T> void CParameterDelayed<T>::setNeedUpdate(bool bUpdate) {

	CParameterTemplate<T>::setNeedUpdate(bUpdate);

	if(bUpdate) {

		if( CParameterTemplate<T>::status() == CParameter::ePUS_UpdateStart )
			CParameterTemplate<T>::setStatus( CParameter::ePUS_UpdateStop );
		else
			CParameterTemplate<T>::setStatus( CParameter::ePUS_UpdateStart );
	}
}

#endif

