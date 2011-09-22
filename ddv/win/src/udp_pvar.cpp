#include <stdlib.h>
#include <string.h>
#include "udp_pvar.h"

#include "ulog.h"

#include <memory>

using namespace std;

extern auto_ptr<ULog> ptrMainLog;

pVariable::pVariable()
{ 
}

pVariable::~pVariable()
{
}

void pVariable::setPtsID(unsigned short _id)
{
    ptsID = _id;
}

void pVariable::setDeviceID(unsigned short _id)
{
    deviceID = _id;
}

void pVariable::setN(unsigned short _n)
{
    N = _n;
}

void pVariable::setStatus(unsigned char _s)
{
    status = _s;
}

void pVariable::setDateTime(QDateTime _dt)
{
    datetime = _dt;
}

void pVariable::setValue(pValue _val)
{
    value = _val;
}

unsigned short pVariable::getPtsID()
{
    return ptsID;
}

unsigned short pVariable::getDeviceID()
{
    return deviceID;
}

unsigned short pVariable::getN()
{
    return N;
}

unsigned char pVariable::getStatus()
{
    return status;
}

QDateTime pVariable::getDateTime()
{
    return datetime;
}

pValue pVariable::getValue()
{
    return value;
}

QString pVariable::print()
{
	QString sTmp =	QString("%1%2%3 | %4 | %5 | ")
						.arg(ptsID,		3, 10, QChar('0'))
						.arg(deviceID,	3, 10, QChar('0'))
						.arg(N,			3, 10, QChar('0'))
						.arg(status)
						.arg(datetime.toString("dd.MM.yyyy hh:mm:ss.zzz"));

	int vi;
    char vs[512];
    double vd;
    switch(value.getType()) {
	case pValue::INTEGER:
	    value.get(&vi);
		sTmp += QString("%1").arg(QString::number(vi));
	    break;
	case pValue::SHORTINT:
	    value.get(&vi);
		sTmp += QString("%1").arg(QString::number(vi));
	    break;
	case pValue::STRING:
	    value.get(vs);
		sTmp += QString("%1").arg(vs);
	    break;
	case pValue::DOUBLE:
	    value.get(&vd);
		sTmp += QString("%1").arg(QString::number(vd));
	    break;
	default:
		sTmp += "INVALID";
    }

	return sTmp;
}

void pVariable::getStr(char* _str, int* _length)
{
    *_length = 17 + value.getLength();
    
    union intchar{
	short int dig;
	char str[2];
    };
    intchar ich;
    
    ich.dig = ptsID;
    _str[0] = ich.str[0];
    _str[1] = ich.str[1];
    
    ich.dig = deviceID;
    _str[2] = ich.str[0];
    _str[3] = ich.str[1];
    
    ich.dig = N;
    _str[4] = ich.str[0];
    _str[5] = ich.str[1];
    
    _str[6] = status;
        
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    tod.dig = sdt.datetoint(datetime.date());
    for(int i = 7; i < 11; i++)
	_str[i] = tod.str[i - 7];
    
    tod.dig = sdt.timetoint(datetime.time());
    for(int i = 11; i < 15; i++)
	_str[i] = tod.str[i - 11];

    _str[15] = value.getType();
    _str[16] = value.getLength();

    for(int i = 0; i < value.getLength(); i++)
	_str[i + 17] = value.get()[i];
}

void pVariable::setStr(char* _str, int* _length)
{
    union intchar{
	short int dig;
	char str[2];
    };
    intchar ich;

    ich.str[0] = _str[0];
    ich.str[1] = _str[1];
    ptsID = ich.dig;
    
    ich.str[0] = _str[2];
    ich.str[1] = _str[3];
    deviceID = ich.dig;
    
    ich.str[0] = _str[4];
    ich.str[1] = _str[5];
    N = ich.dig;
    
    status = _str[6];
    
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    for(int i = 7; i < 11; i++)
	tod.str[i - 7] = _str[i];
    datetime.setDate(sdt.inttodate(tod.dig));
    
    for(int i = 11; i < 15; i++)
	tod.str[i - 11] = _str[i];
    datetime.setTime(sdt.inttotime(tod.dig));
    
    value.setData(_str + 17, _str[15]);
    
    *_length = 17 + value.getLength();
}

