//<codepage = utf8
/*!
	\file parameter_integrated.h
	\brief Описание класса CParameterIntegrated
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt4
	\warning Платформной зависимости не замечено
*/
#ifndef __PARAMETER_TYPE_INTEGRATED_H__
#define __PARAMETER_TYPE_INTEGRATED_H__

#include "parameter_type_group.h"

/*! \class CParameterIntegrated
	\brief Интегральный параметр состояния группы
*/

template<class T> class CParameterIntegrated : public CParameterTemplate<T> {
public:
	//! конструктор
	CParameterIntegrated(const CParameterDefinition * );
	// *** интерфейсные функции ***
	// *** свойства параметра ***
	//! устанавливает значение параметра
	void setValue(CParameter * );
	// ***
};

//! конструктор
template<class T> CParameterIntegrated<T>::CParameterIntegrated(const CParameterDefinition *pParameterInfo) : 
	CParameterTemplate<T>(pParameterInfo)
{
}

//! устанавливает значение параметра
template<class T> void CParameterIntegrated<T>::setValue(CParameter *pPar) {

	// если переданный параметр действительно группа
	if( dynamic_cast<CParameterGroup<int> *>(pPar) ) {

		// преобразуем указатель на абстрактный параметр в указатель на группу
		CParameterGroup<int> *pGrPar = reinterpret_cast<CParameterGroup<int> *>(pPar);

		// подсчет количества значений параметров в группе из списка разрешенных значений (accept)
		int iAcceptedPreset = 0;
		// перебираем список accepted
		QStringList::const_iterator p, e = CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistAccept.constEnd();
		for(p = CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistAccept.constBegin(); p != e; ++p)
			// значение существует и обновлено на очередном периоде
			if( pGrPar->getByVal(*p) && pGrPar->getByVal(*p)->status() == CParameter::ePUS_Updated )
				// увеличиваем количество присутствующих разрешенных значений
				iAcceptedPreset++;

		// учитываем список игнорируемых значений (ignore)
		// берем общее количество значений в группе
		int iWholeNumber = pGrPar->getNumOfUpdated();
		// перебираем список ignore
		e = CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistIgnore.constEnd();
		for(p = CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistIgnore.constBegin(); p != e; ++p)
			// значение существует и обновлено на очередном периоде
			if( pGrPar->getByVal(*p) && pGrPar->getByVal(*p)->status() == CParameter::ePUS_Updated )
				 // уменьшаем общее количество значений в группе
				 iWholeNumber--;

		// штатные все, не штатных нет
		// кол-во присутствующих значений из списка accept = кол-ву значений в списке accept
		// общее кол-во присутствующих значений = присутствующих значений из списка accept
		if( iAcceptedPreset == CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistAccept.count() && iWholeNumber == iAcceptedPreset )
			CParameterTemplate<T>::setValue("0");

		// штатные не все, не штатных нет
		// кол-во присутствующих значений из списка accept < кол-ва значений в списке accept
		// общее кол-во присутствующих значений = кол-ву присутствующих значений из списка accept
		if( iAcceptedPreset <  CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistAccept.count() && iWholeNumber == iAcceptedPreset )
			CParameterTemplate<T>::setValue("1");

		// штатные все, есть не штатные
		// кол-во присутствующих значений из списка accept = кол-ва значений в списке accept
		// общее кол-во присутствующих значений != кол-ву присутствующих значений из списка accept
		if( iAcceptedPreset == CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistAccept.count() && iWholeNumber != iAcceptedPreset )
			CParameterTemplate<T>::setValue("2");

		// штатные не все, есть не штатные
		// кол-во присутствующих значений из списка accept < кол-ва значений в списке accept
		// общее кол-во присутствующих значений != кол-ву присутствующих значений из списка accept
		if( iAcceptedPreset <  CParameterTemplate<T>::m_pParameterInfo->m_pProps->m_qlistAccept.count() && iWholeNumber != iAcceptedPreset )
			CParameterTemplate<T>::setValue("3");
	}
}

#endif

