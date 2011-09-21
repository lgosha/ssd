#include "parameter_info_creator.h"

#include "exception.h"
#include "parameter_info.h"

// *** CParameterInfoCreator ***

CParameterDefinition * CParameterInfoCreator::create ( 
	ulong ulId, XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> &list ) {

	return new CParameterDefinition( ulId, pConfig, pNode, list );

}


CParameterDefinition *	CParameterInfoCreator::create(
	ulong ulId, const QString &type, const QList<uint> &list, bool bSend ) {

	return new CParameterDefinition( ulId, type, list, bSend );
}

// ***

// *** CParameterInfoBuilder ***

CParameterInfoBuilder::CParameterInfoBuilder() {

	m_qmapCreators.insert(ePT_Value, new CParameterInfoCreator );
}

CParameterInfoBuilder::~CParameterInfoBuilder() {

	foreach( CParameterInfoAbstractCreator* creator, m_qmapCreators.values() )
		delete creator;
}

CParameterDefinition* CParameterInfoBuilder::build( 
	ePP_ParameterType ptype, ulong ulId, XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> &list ) {

	if( !m_qmapCreators.contains(ptype) )
		throw ex_base	(
				ex_base::error,
				QString("not supported parameter info type: %1").arg(ptype),
				"CParameterInfoBuilder::build"
				);

	return m_qmapCreators[ptype]->create( ulId, pConfig, pNode, list );
}


CParameterDefinition * CParameterInfoBuilder::build( ePP_ParameterType ptype, ulong ulId, const QString &vtype, const QList<uint> &list, bool bSend ) {

	if( !m_qmapCreators.contains(ptype) )
		throw ex_base	(
				ex_base::error,
				QString("not supported parameter info type: %1").arg(ptype),
				"CParameterInfoBuilder::build"
				);

	return m_qmapCreators[ptype]->create( ulId, vtype, list, bSend );
}


// ***

