#ifndef __PARAMETER_INFO_CREATOR__
#define __PARAMETER_INFO_CREATOR__

#include <QMap>

#include "parameter_enum.h"
#include "xmlconfig.h"

class CParameterDefinition;

class CParameterInfoAbstractCreator {
public:
	virtual CParameterDefinition *	create	( ulong, XMLConfig *, const DOMNode *, const QList<uint> & ) = 0;
	virtual CParameterDefinition *	create	( ulong, const QString &, const QList<uint> &, bool = true ) = 0;
};

class CParameterInfoCreator : public CParameterInfoAbstractCreator {
public:
	virtual CParameterDefinition *	create	( ulong, XMLConfig *, const DOMNode *, const QList<uint> & );
	virtual CParameterDefinition *	create	( ulong, const QString &, const QList<uint> &, bool = true );
};

class CParameterInfoBuilder {
public:
	 CParameterInfoBuilder();
	~CParameterInfoBuilder();

	CParameterDefinition* build( ePP_ParameterType, ulong, XMLConfig *, const DOMNode *, const QList<uint> & );
	CParameterDefinition* build( ePP_ParameterType, ulong, const QString &, const QList<uint> &, bool = true );
private:
	QMap<ePP_ParameterType, CParameterInfoAbstractCreator *> m_qmapCreators;
};


#endif

