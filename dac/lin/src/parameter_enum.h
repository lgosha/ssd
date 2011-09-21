#ifndef __PARAMETER_ENUM_H__
#define __PARAMETER_ENUM_H__

enum ePP_QueryType {

	eQT_SNMP,
	eQT_WMI,
	eQT_CALC,
};

enum ePP_ParameterType {

	ePT_Value,
	ePT_Delayed,
	ePT_Integrated,
	ePT_Group
};

enum ePP_ValueType {

	eVT_Int,
	eVT_Double,
	eVT_String,
	eVT_Short
};

#endif
