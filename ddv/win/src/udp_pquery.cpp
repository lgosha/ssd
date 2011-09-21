//<codepage = utf8
#include <stdlib.h>
#include <string.h>
#include <QtGlobal>
#include <QByteArray>
#include "udp_pquery.h"

pQuery::pQuery()
{
    for(int i = 0; i < 14; i++)
	datagram[i] = 0;
}

pQuery::~pQuery()
{
    for(int i = 0; i < 14; i++)
	datagram[i] = 0;
}

void pQuery::set_datetime(long int _date, long int _time)
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    tod.dig = _date;    
    for(int i = 3; i < 7; i++)
	datagram[i] = tod.str[i - 3];
    
    tod.dig = _time;
    for(int i = 7; i < 11; i++)
	datagram[i] = tod.str[i - 7];
}

void pQuery::setDateTime(QDate _date, QTime _time)
{
    set_datetime(sdt.datetoint(_date), sdt.timetoint(_time));
}

void pQuery::setID(unsigned char _id)
{
    datagram[2] = _id;
}

void pQuery::setIDC(unsigned short _id)
{
    *(ushort*)(datagram + 12) = _id;
}

void pQuery::setType(unsigned char _st)
{
    datagram[11] = _st;
}

unsigned char pQuery::getID()
{
    return datagram[2];
}

unsigned short pQuery::getIDC()
{
    return *(ushort*)(datagram + 12);
}

long int pQuery::get_date()
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    for(int i = 3; i < 7; i++)
	tod.str[i - 3] = datagram[i];
    
    return tod.dig;
}

long int pQuery::get_time()
{
    union timeordate{
	long int dig;
	char str[4];
    };
    timeordate tod;
    
    for(int i = 7; i < 11; i++)
	tod.str[i - 7] = datagram[i];
    
    return tod.dig;
}

QDate pQuery::getDate()
{
    return sdt.inttodate(get_date());
}

QTime pQuery::getTime()
{
    return sdt.inttotime(get_time());
}

unsigned char pQuery::getType()
{
    return datagram[11];
}

void pQuery::calculateChecksum()
{
    union checksum{
	short int dig;
	char str[2];
    };
    checksum chksm;
    
    chksm.dig = qChecksum(datagram + 2, 12);
    for(int i = 0; i < 2; i++)
	datagram[i] = chksm.str[i];
}

bool pQuery::verifyChecksum()
{
    union checksum{
	short int dig;
	char str[2];
    };
    checksum chksm;
    
    short int tdig = qChecksum(datagram + 2, 12);
    for(int i = 0; i < 2; i++)
	chksm.str[i] = datagram[i];

    if(tdig == chksm.dig)
	return true;
    return false;
}

QString pQuery::print()
{
	return QString("%1 | %2 | %3 | %4")
				.arg(getID(), 3)
				.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"))
				.arg(getType(), 5)
				.arg(getIDC(), 5);
}

void pQuery::setDatagram(char* _d)
{
    for(int i = 0; i < 14; i++)
	datagram[i] = _d[i];
}

char* pQuery::getDatagram()
{
    calculateChecksum();
    return datagram;
}
