///////////////////////////////////////////////////////
//               parameter Variable                  //
//               ------------------                  //
//                                                   //
//                                                   //
///////////////////////////////////////////////////////

/*! 
	\file pvar.h
    	\brief Описание класса pVariable
	\author Демидов Роман Владиславович
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt версии 4 (протестировано для qt 4.3.1)
	\warning Платформной зависимости пока не обнаружено...
*/

#ifndef PARAMETERVARIABLECLASSDEFINITION_H
#define PARAMETERVARIABLECLASSDEFINITION_H

#include <stdio.h>
#include <QDateTime>
#include "udp_pvalue.h"
#include "udp_datetime.h"
#include <QString>

/*! \class pVariable
\brief Класс переменной

<p>Класс предназначен для хранения одной переменной вместе с идентификаторами ПТС и устройства, статусом, номером, меткой времени и, собственно, значением.
</p>

*/

class pVariable {
    unsigned short ptsID;
    unsigned short deviceID;
    unsigned short N;
    unsigned char  status;
    QDateTime      datetime;
    pValue         value;
    
    DateTime  sdt;

public:
    /*! \brief Конструктор класса */
    pVariable();
    /*! \brief Деструктор класса */
   ~pVariable();
    
    void setPtsID(unsigned short _id);
    void setDeviceID(unsigned short _id);
    void setN(unsigned short _n);
    void setStatus(unsigned char _s);
    void setDateTime(QDateTime _dt);
    void setValue(pValue _val);
    
    unsigned short getPtsID();
    unsigned short getDeviceID();
    unsigned short getN();
    unsigned char  getStatus();
    QDateTime      getDateTime();
    pValue         getValue();

    /*! \brief Возвращает все данные класса в виде читаемой строки для последующего вывода */
    QString print();
    
    /*! \brief Преобразует собственные данные в строку байтов */
    void getStr(char* _str, int* _length);
    /*! \brief Преобразует строку байтов в собственные данные */
    void setStr(char* _str, int* _length);
};

#endif

