//<codepage = utf8
///////////////////////////////////////////////////////
//                 parameter Value                   //
//                 ---------------                   //
//                                                   //
//                                                   //
///////////////////////////////////////////////////////

/*! 
	\file pvalue.h
    	\brief Описание класса pValue
	\author Демидов Роман Владиславович
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt версии 4 (протестировано для qt 4.3.1)
	\warning Платформной зависимости пока не обнаружено
*/

#ifndef PARAMETERVALUECLASSDEFINITION_H
#define PARAMETERVALUECLASSDEFINITION_H

#include <stdio.h>

/*! \class pValue
\brief Класс значения

<p>Класс предназначен для хранения значения переменной с изначально неопределенным типом (м. б. SHORTINT [2 байта], INTEGER [4 байта], STRING, DOUBLE [8 байтов]).
</p>

*/

class pValue {
public:
    char* data;
    char type;
public:
    /*! \brief Конструктор класса */
    pValue();
    /*! \brief Деструктор класса */
   ~pValue();
    /*! \brief Конструктор копирования класса */
    pValue(const pValue & _pv);
    
    /*! \brief Возможные типы данных */
    enum types {NODATA = 0, SHORTINT, INTEGER, STRING, DOUBLE};
    
    /*! \brief Устанавливает значение целочисленного типа: <i>_type = INTEGER | SHORTINT</i> */
    void  set(int _i, char _type = INTEGER);
    /*! \brief Устанавливает значение типа <b>double</b> */
    void  set(double _d);
    /*! \brief Устанавливает значение строковаго типа из <b>char*</b> */
    void  set(const char* _c);
    /*! \brief Возвращает массив байтовых данных (как оно и хранится в памяти) */
    char* get();
    /*! \brief Возвращает значение целочисленного типа
        \details Если тип несоответствующий, то не изменяет переменную */
    void  get(int* _i);
    /*! \brief Возвращает значение типа <b>double</b>
        \details Если тип несоответствующий, то не изменяет переменную */
    void  get(double* _d);
    /*! \brief Возвращает значение строкового типа
        \details Если тип несоответствующий, то не изменяет переменную */
    void  get(char* _d);
    /*! \brief Возвращает тип хранимого значения */
    char  getType();
    /*! \brief Возвращает количество затрачиваемых на хранение переменной байтов */
    char  getLength();
    /*! \brief Переопределенный оператор присваивания */
    pValue & operator=(const pValue & _pv);
    
    /*! \brief Устанавливает значение переменной с типом <b>_type</b>, копируя сырые данные из <b>_data</b> */
    void  setData(char* _data, char _type);
    
    /*! \brief Преобразует данные к визуально читабельному виду */
    char* toStr();
};

#endif


