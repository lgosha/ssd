#ifndef __PARAMETER_CREATOR_H__
#define __PARAMETER_CREATOR_H__

#include <QMap>

#include "parameter_enum.h"
#include "exception.h"

class CParameterDefinition;
class CParameter;

class CParameterAbstractCreator {
public:
	virtual CParameter *		createIntPar	( const CParameterDefinition * ) = 0;
	virtual CParameter *		createStrPar	( const CParameterDefinition * ) = 0;
	virtual CParameter *		createDoublePar	( const CParameterDefinition * ) = 0;
	virtual CParameter *		createShortPar	( const CParameterDefinition * ) = 0;

	virtual bool isValidParameterDefinition( const CParameterDefinition * ) { return true; }
};

class CParameterTemplateCreator : public CParameterAbstractCreator {
public:
	virtual CParameter *		createIntPar	( const CParameterDefinition * );
	virtual CParameter *		createStrPar	( const CParameterDefinition * );
	virtual CParameter *		createDoublePar	( const CParameterDefinition * );
	virtual CParameter *		createShortPar	( const CParameterDefinition * );
};

class CParameterDelayedCreator : public CParameterAbstractCreator {
public:
	virtual CParameter *		createIntPar	( const CParameterDefinition * );
	virtual CParameter *		createStrPar	( const CParameterDefinition * );
	virtual CParameter *		createDoublePar	( const CParameterDefinition * );
	virtual CParameter *		createShortPar	( const CParameterDefinition * );
};

class CParameterBuilder {
public:
	 CParameterBuilder();
	~CParameterBuilder();

	CParameter* build( ePP_ParameterType, ePP_ValueType, const CParameterDefinition * );
private:
	QMap<ePP_ParameterType, CParameterAbstractCreator *> m_qmapCreators;
};

#endif
