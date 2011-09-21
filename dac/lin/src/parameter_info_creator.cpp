#include "parameter_info_creator.h"

#include "exception.h"
#include "parameter_info.h"

// *** CParameterInfoCreator ***

CParameterDefinition * CParameterInfoCreator::create ( 
	ulong ulId, XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> & list, const QMap<QString, int> &map ) {

	return new CParameterDefinition( ulId, pConfig, pNode, list, map );

}
// ***

// *** CGroupInfoCreator ***

CParameterDefinition * CGroupInfoCreator::create ( 
	ulong ulId, XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> & list, const QMap<QString, int> &map ) {

	return new CGroupDefinition( ulId, pConfig, pNode, list, map );
}

// ***

// *** CParameterInfoBuilder ***
CParameterInfoBuilder::CParameterInfoBuilder() {

	m_qmapCreators.insert(ePT_Value,	new CParameterInfoCreator	);
	m_qmapCreators.insert(ePT_Delayed,	new CParameterInfoCreator	);
	m_qmapCreators.insert(ePT_Integrated,	new CParameterInfoCreator	);
	m_qmapCreators.insert(ePT_Group,	new CGroupInfoCreator		);
}

CParameterInfoBuilder::~CParameterInfoBuilder() {

	foreach( CParameterInfoAbstractCreator* creator, m_qmapCreators.values() )
		delete creator;
}

CParameterDefinition* CParameterInfoBuilder::build( 
	ePP_ParameterType ptype, ulong ulId, XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> & list, const QMap<QString, int> &map ) {

	if( !m_qmapCreators.contains(ptype) )
		throw ex_base	(
				ex_base::error,
				QString("not supported parameter info type: %1").arg(ptype),
				"CParameterInfoBuilder::build"
				);

	return m_qmapCreators[ptype]->create( ulId, pConfig, pNode, list, map );
}

// ***

