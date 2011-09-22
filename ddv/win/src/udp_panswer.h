///////////////////////////////////////////////////////
//                 parameter Answer                  //
//                 ----------------                  //
//                                                   //
//                                                   //
///////////////////////////////////////////////////////

/*! 
	\file pAnswer.h
    	\brief Описание класса pAnswer
	\author Демидов Роман Владиславович
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt версии 4 (протестировано для qt 4.3.1)
	\warning Платформной зависимости пока не обнаружено...
*/

#ifndef PARAMETERANSWERCLASSDEFINITION_H
#define PARAMETERANSWERCLASSDEFINITION_H

#define MAX_VAR_COUNT 10000

#include <stdio.h>
#include <QDateTime>
#include "udp_pvar.h"
#include "udp_datetime.h"

/*! \class pAnswer
\brief Класс ответа

<p>Класс предназначен для формирования ответа на запрос, его представления в виде строки байтов, а также перевода из строки байтов в ответ для последующего извлечения параметров.
</p>

*/

class pAnswer {
    unsigned char id;
    QDateTime     datetime;
    pVariable*    vars[MAX_VAR_COUNT];
    ushort sid;
    int           var_count;

    void     set_datetime(long int _date, long int _time, char* _data);
    long int get_date(char* _data);
    long int get_time(char* _data);
    
    void     set_size(long int _size, char* _data);
    long int get_size(char* _data);
    
    DateTime sdt;
    
public:
    /*! \brief Конструктор класса */
    pAnswer();
    /*! \brief Деструктор класса */
   ~pAnswer();
    /*! \brief Конструктор копирования класса */
    pAnswer(const pAnswer & _c);
    
    /*! \brief Добавляет в ответ переменную */
    void      addVariable(pVariable _pv);
    /*! \brief Вытаскивает из ответа переменную с порядковым номером <b>_num</b> */
    pVariable getVariable(int _num);
    /*! \brief Возвращает количество переменных в ответе */
    int       getVariableCount();
    
    /*! \brief Устанавливает метку времени */
    void setDateTime(QDateTime _dt);
    /*! \brief Устанавливает ID ответа */
    void setID(unsigned char _id);
    /*! \brief Устанавливает ID сервера */
    void setSID(unsigned short _id);

    /*! \brief Возвращает ID ответа (т.е. запроса) */
    unsigned char getID();
    /*! \brief Возвращает ID сервера */
    unsigned short getSID();
    /*! \brief Возвращает метку времени */
    QDateTime     getDateTime();
    
    /*! \brief Вычисляет контрольную сумму для <b>_datagram</b> */
    void calculateChecksum(char* _datagram);
    /*! \brief Проверяет контрольную сумму для <b>_datagram</b> (<i>true = успешно</i>) */
    bool verifyChecksum(char* _datagram);

    /*! \brief Возвращает заголовок пакета */
    QString print();
    /*! \brief Возвращает данные */
    QStringList print_vars();
    
    /*! \brief Преобразует дейтаграмму <b>_data</b> длиной <b>_len</b> в данные класса (<i>true = успешно</i>)*/
    bool setDatagram(char* _data, int  _len);
    /*! \brief Преобразует данные класса в дейтаграмму <b>_data</b>, возвращает ее длину <b>_len</b>*/
    void getDatagram(char* _data, int* _len);
    
    /*! \brief Очистка данных */
    void clear();
};

#endif

