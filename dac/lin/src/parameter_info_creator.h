#ifndef __PARAMETER_INFO_CREATOR__
#define __PARAMETER_INFO_CREATOR__

#include <QMap>
//#include <QList>

#include "parameter_enum.h"
#include "xmlconfig.h"

class CParameterDefinition;
class CGroupDefinition;

class CParameterInfoAbstractCreator {
public:
	virtual CParameterDefinition *	create	( ulong, XMLConfig *, const DOMNode *, 	const QList<uint> &, const QMap<QString, int> & ) = 0;
};

class CParameterInfoCreator : public CParameterInfoAbstractCreator {
public:
	virtual CParameterDefinition *	create	( ulong, XMLConfig *, const DOMNode *, const QList<uint> &, const QMap<QString, int> & );
};

class CGroupInfoCreator : public CParameterInfoAbstractCreator {
public:
	virtual CParameterDefinition *	create	( ulong, XMLConfig *, const DOMNode *, const QList<uint> &, const QMap<QString, int> & );
};

class CParameterInfoBuilder {
public:
	 CParameterInfoBuilder();
	~CParameterInfoBuilder();

	CParameterDefinition* build( ePP_ParameterType, ulong, XMLConfig *, const DOMNode *, const QList<uint> &, const QMap<QString, int> & );
private:
	QMap<ePP_ParameterType, CParameterInfoAbstractCreator *> m_qmapCreators;
};


#endif
