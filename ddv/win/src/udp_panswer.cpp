#include <stdlib.h>
#include <string.h>
#include <QtGlobal>
#include <QByteArray>
#include <QStringList>

#include "udp_panswer.h"

pAnswer::pAnswer()
{
    var_count = 0;
    for(int i = 0; i < MAX_VAR_COUNT; i++)
	vars[i] = NULL;
}

pAnswer::~pAnswer()
{
    for(int i = 0; i < var_count; i++) {
	delete vars[i];
	vars[i] = NULL;
    }
    var_count = 0;
}

pAnswer::pAnswer(const pAnswer & _c)
{
    id = _c.id;
    datetime = _c.datetime;

    var_count = 0;
    for(int i = 0; i < _c.var_count; i++)
	addVariable(*_c.vars[i]);
}

void pAnswer::set_datetime(long int _date, long int _time, char* _data)
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    tod.dig = _date;
    for(int i = 3; i < 7; i++)
	_data[i] = tod.str[i - 3];
    
    tod.dig = _time;
    for(int i = 7; i < 11; i++)
	_data[i] = tod.str[i - 7];
}

long int pAnswer::get_date(char* _data)
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    for(int i = 3; i < 7; i++)
	tod.str[i - 3] = _data[i];
    
    return tod.dig;
}

long int pAnswer::get_time(char* _data)
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    for(int i = 7; i < 11; i++)
	tod.str[i - 7] = _data[i];
    
    return tod.dig;
}

void pAnswer::set_size(long int _size, char* _data)
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    tod.dig = _size;
    for(int i = 11; i < 15; i++)
	_data[i] = tod.str[i - 11];
}

long int pAnswer::get_size(char* _data)
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    for(int i = 11; i < 15; i++)
	tod.str[i - 11] = _data[i];
    
    return tod.dig;
}

void pAnswer::addVariable(pVariable _pv)
{
    if(var_count < MAX_VAR_COUNT) {
	vars[var_count] = new pVariable();
	*vars[var_count++] = _pv;
    }
}

pVariable pAnswer::getVariable(int _num)
{
    if(_num >= 0 && _num < var_count)
	return *vars[_num];
    return *vars[0];
}

int pAnswer::getVariableCount()
{
    return var_count;
}

void pAnswer::setDateTime(QDateTime _dt)
{    
    datetime = _dt;
}

void pAnswer::setID(unsigned char _id)
{
    id = _id;
}

void pAnswer::setSID(unsigned short _id)
{
    sid = _id;
}

unsigned char pAnswer::getID()
{
    return id;
}

unsigned short pAnswer::getSID()
{
    return sid;
}

QDateTime pAnswer::getDateTime()
{
    return datetime;
}

void pAnswer::calculateChecksum(char* _data)
{
    union checksum{
	short int dig;
	char str[2];
    };
    checksum chksm;
    
    chksm.dig = qChecksum(_data + 2, get_size(_data) + 16 - 2);
    for(int i = 0; i < 2; i++)
	_data[i] = chksm.str[i];
}

bool pAnswer::verifyChecksum(char* _data)
{
    union checksum{
	short int dig;
	char str[2];
    };
    checksum chksm;
    
    short int tdig = qChecksum(_data + 2, get_size(_data) + 16 - 2);
    for(int i = 0; i < 2; i++)
	chksm.str[i] = _data[i];

    if(tdig == chksm.dig)
	return true;
    return false;
}

QString pAnswer::print()
{
	return QString("%1 | %2 | %3 | %4")
				.arg(getID(), 3)
				.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"))
				.arg(var_count, 5)
				.arg(sid, 5);
}

QStringList pAnswer::print_vars()
{
	QStringList lstr;
	for(int i = 0; i < var_count; i++)
		lstr << vars[i]->print();
	return lstr;
}

bool pAnswer::setDatagram(char* _data, int _len)
{
    if(!verifyChecksum(_data))
	return false;

    id = _data[2];
    sid = *(ushort*)(_data + 15);

    datetime.setDate(sdt.inttodate(get_date(_data)));
    datetime.setTime(sdt.inttotime(get_time(_data)));
    
    int size = get_size(_data);
    
    pVariable tvar;
    int cur_ptr = 17, len = 0;
    
    for(; size > 0; size -= len) {
	tvar.setStr(_data + cur_ptr, &len);
	cur_ptr += len;
	addVariable(tvar);
    }
    
    return true;
}

void pAnswer::getDatagram(char* _data, int* _len)
{
    int end_ptr = 17, tmplen;
    char tmpstr[512];
    
    int size = 0;
    
    _data[2] = id;
    *(ushort*)(_data + 15) = sid;
    set_datetime(sdt.datetoint(datetime.date()), sdt.timetoint(datetime.time()), _data);
    
    for(int i = 0; i < var_count; i++) {
	vars[i]->getStr(tmpstr, &tmplen);
	for(int i = 0; i < tmplen; i++)
	    _data[end_ptr++] = tmpstr[i];
	size += tmplen;
    }
    
    set_size(size, _data);
    calculateChecksum(_data);
    *_len = end_ptr;
}

void pAnswer::clear()
{
    for(int i = 0; i < var_count; i++) {
	delete vars[i];
	vars[i] = NULL;
    }
    var_count = 0;
}

