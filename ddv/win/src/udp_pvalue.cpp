//<codepage = utf8
#include <stdlib.h>
#include <string.h>
#include "udp_pvalue.h"

pValue::pValue()
{
    data = NULL;
    type = NODATA;
}

pValue::~pValue()
{
    if(type) {
	free(data);
	data = NULL;
	type = NODATA;
    }
}

pValue::pValue(const pValue & _pv)
{
    type = _pv.type;
    switch(type) {
	case INTEGER:
	    data = (char*)malloc(4);
	    for(int i = 0; i < 4; i++)
		data[i] = _pv.data[i];
	    break;
	case SHORTINT:
	    data = (char*)malloc(2);
	    for(int i = 0; i < 2; i++)
		data[i] = _pv.data[i];
	    break;
	case STRING:
	    data = (char*)malloc(strlen(_pv.data) + 1);
	    for(unsigned int i = 0; i <= strlen(_pv.data); i++)
		data[i] = _pv.data[i];
	    break;
	case DOUBLE:
	    data = (char*)malloc(8);
	    for(int i = 0; i < 8; i++)
		data[i] = _pv.data[i];
	    break;
    }
}

void pValue::set(int _i, char _type)
{
    if(type)
	free(data);
    if(_type == INTEGER) {
	data = (char*)malloc(4);
	*(int*)data = _i;
	type = INTEGER;
    }
    else if(_type == SHORTINT) {
	data = (char*)malloc(2);
	*(short*)data = _i;
	type = SHORTINT;
    }
}

void pValue::set(double _d)
{
    if(type)
	free(data);
    data = (char*)malloc(8);
    *(double*)data = _d;
    type = DOUBLE;
}

void pValue::set(const char* _c)
{
    if(type)
	free(data);
    data = (char*)malloc(strlen(_c) + 1);
    strcpy(data, _c);
    type = STRING;
}

char* pValue::get()
{
    return data;
}

void pValue::get(int* _i)
{
    if(type == INTEGER)
	*_i = *(int*)data;
    if(type == SHORTINT)
	*_i = *(short*)data;
}

void pValue::get(double* _d)
{
    if(type == DOUBLE)
	*_d = *(double*)data;
}

void pValue::get(char* _c)
{
    if(type == STRING)
	strcpy(_c, data);
}

char pValue::getType()
{
    return type;
}

char pValue::getLength()
{
    switch(type) {
	case NODATA:
	    return 0;
	case SHORTINT:
	    return 2;
	case INTEGER:
	    return 4;
	case DOUBLE:
	    return 8;
	case STRING:
	    return strlen(data) + 1;
	default:
	    //BUG
	    
	    return 0;
    };
}

void pValue::setData(char* _data, char _type)
{
    if(type)
	free(data);
    
    switch(_type) {
	case SHORTINT:
	    data = (char*)malloc(2);
	    for(int i = 0; i < 2; i++)
		data[i] = _data[i];
	    break;
	case INTEGER:
	    data = (char*)malloc(4);
	    for(int i = 0; i < 4; i++)
		data[i] = _data[i];
	    break;
	case DOUBLE:
	    data = (char*)malloc(8);
	    for(int i = 0; i < 8; i++)
		data[i] = _data[i];
	    break;
	case STRING:
	    data = (char*)malloc(strlen(_data) + 1);
	    strcpy(data, _data);
	    break;
	default:
	    data = NULL;
    };
    type = _type;
}

char* pValue::toStr()
{
    int i1, i2;
    char *tval = new char[16];
    switch(type) {
	case SHORTINT:
	    return fcvt(*(short*)data, 0, &i1, &i2);
	    break;
	case INTEGER:
	    return fcvt(*(int*)data, 0, &i1, &i2);
	    break;
	case DOUBLE:
	    gcvt(*(double*)data, 8, tval);
	    return tval;
	    break;
	case STRING:
	    return data;
	    break;
	default:
	    //BUG
	    
	    return NULL;
    };
    
}

pValue & pValue::operator=(const pValue & _pv)
{
    if(this == &_pv)
	return *this;

    if(type)
	free(data);
    type = _pv.type;
    switch(type) {
	case INTEGER:
	    data = (char*)malloc(4);
	    for(int i = 0; i < 4; i++)
		data[i] = _pv.data[i];
	    break;
	case SHORTINT:
	    data = (char*)malloc(2);
	    for(int i = 0; i < 2; i++)
		data[i] = _pv.data[i];
	    break;
	case STRING:
	    data = (char*)malloc(strlen(_pv.data) + 1);
	    for(unsigned int i = 0; i <= strlen(_pv.data); i++)
		data[i] = _pv.data[i];
	    break;
	case DOUBLE:
	    data = (char*)malloc(8);
	    for(int i = 0; i < 8; i++)
		data[i] = _pv.data[i];
	    break;
    }
    return *this;
}


