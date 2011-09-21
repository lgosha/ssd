//<codepage = utf8
///////////////////////////////////////////////////////
//               parameter Variable                  //
//               ------------------                  //
//                                                   //
//                                                   //
///////////////////////////////////////////////////////

#ifndef PARAMETERVARIABLECLASSDEFINITION_H
#define PARAMETERVARIABLECLASSDEFINITION_H

#include <stdio.h>
#include <QDateTime>
#include <QString>

#include "udp_pvalue.h"
#include "udp_datetime.h"

class CParameter;

class pVariable {
    unsigned short ptsID;
    unsigned short deviceID;
    unsigned short N;
    unsigned char status;
    QDateTime     datetime;
    pValue        value;
    
    DateTime  sdt;

public:
    pVariable();
	pVariable(CParameter *pPar);
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
    unsigned char getStatus();
    QDateTime     getDateTime();
    pValue        getValue();

    QString print();
    
    void getStr(char* _str, int* _length);
    void setStr(char* _str, int* _length);
};

#endif


