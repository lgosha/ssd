//<codepage = utf8
///////////////////////////////////////////////////////
//                 parameter Query                   //
//                 ---------------                   //
//                                                   //
//                                                   //
///////////////////////////////////////////////////////

//<codepage = utf8
/*! 
	\file pquery.h
    	\brief Описание класса pQuery
	\author Демидов Роман Владиславович
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt версии 4 (протестировано для qt 4.3.1)
	\warning Платформной зависимости не замечено
*/

#ifndef PARAMETERQUERYCLASSDEFINITION_H
#define PARAMETERQUERYCLASSDEFINITION_H

#include <stdio.h>
#include <QDateTime>
#include "udp_datetime.h"

/*! \class pQuery
\brief Класс запроса

<p>Класс предназначен для формирования запроса, его представления в виде строки байтов, а также перевода из строки байтов в запрос для последующего извлечения параметров.
</p>

*/

class pQuery {
    char datagram[13];

    void set_datetime(long int _date, long int _time);
    long int get_date();
    long int get_time();

    DateTime sdt;
    
    /*! \brief Подсчитывает контрольную сумму */
    void calculateChecksum();
    
public:

	enum EQueryType
	{
		/*!	передать только изменившиеся */
		eQT_Modified		= 1,
		/*!	передать только изменившиеся link */
		eQT_MLink		= 2,
		/*!	тестовый эхо-ответ */
		eQT_Test		= 100,
		/*!	передать all link */
		eQT_Link		= 200,
		/*!	передать modified просмоторщику */
		eQT_MViewer		= 253,
		/*!	передать все данные просмоторщику */
		eQT_Viewer		= 254,
		/*!	 передать все данные */
		eQT_All			= 255
	};


    /*! \brief Конструктор класса */
    pQuery();
    /*! \brief Деструктор класса */
   ~pQuery();

    /*! \brief Присвоение ID пакета */
    void setID(unsigned char _id);
    /*! \brief Присвоение ID клиента */
    void setIDC(unsigned short _id);
    /*! \brief Присвоение Типа Запроса*/
    void setType( EQueryType _st );
    /*! \brief Присвоение Даты и Времени */
    void setDateTime(QDate _date, QTime _time);
    
    /*! \brief Возвращает ID пакета */
    unsigned char getID();
    /*! \brief Возвращает ID клиента */
    unsigned short getIDC();
    /*! \brief Возвращает Тип Запроса */
    unsigned char getType();
    /*! \brief Возвращает дату */
    QDate         getDate();
    /*! \brief Возвращает время */
    QTime         getTime();
    
    /*! \brief Проверка контрольной суммы дейтаграммы */
    bool verifyChecksum();

    /*! \brief Возвращает запрос в виде строки для печати */
    QString print();
    
    /*! \brief Запихивает в класс дейтаграмму */
    void  setDatagram(char* _d); //push the datagram in pQuery
    /*! \brief Создает дейтаграмму */
    char* getDatagram();         //gets datagram from pQuery
};

#endif


