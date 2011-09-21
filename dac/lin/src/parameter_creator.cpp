
#include "parameter_creator.h"
#include "parameter_type_delayed.h"
#include "parameter_type_integrated.h"
#include "parameter_type_group.h"

// *** CParameterTemplateCreator ***

CParameter* CParameterTemplateCreator::createIntPar( const CParameterDefinition *pInfo ) {

	return new CParameterTemplate<int>(pInfo);
}

CParameter* CParameterTemplateCreator::createStrPar( const CParameterDefinition *pInfo ) {

	return new CParameterTemplate<QString>(pInfo);
}

CParameter* CParameterTemplateCreator::createDoublePar( const CParameterDefinition *pInfo ) {

	return new CParameterTemplate<double>(pInfo);
}

CParameter* CParameterTemplateCreator::createShortPar( const CParameterDefinition *pInfo ) {

	return  new CParameterTemplate<short>(pInfo);
}

// ***

// *** CParameterDelayedCreator ***

CParameter* CParameterDelayedCreator::createIntPar( const CParameterDefinition *pInfo ) {

	return new CParameterDelayed<int>(pInfo);
}

CParameter* CParameterDelayedCreator::createStrPar( const CParameterDefinition *pInfo ) {

	throw ex_base	(
			ex_base::error, 
			"value type 'QString' not supported for CParameterDelayed parameter type",
			"CParameterDelayedCreator:createStrPar"
			);
	return 0;
}

CParameter* CParameterDelayedCreator::createDoublePar( const CParameterDefinition *pInfo ) {

	return new CParameterDelayed<double>(pInfo);
}

CParameter* CParameterDelayedCreator::createShortPar( const CParameterDefinition *pInfo ) {

	return new CParameterDelayed<short>(pInfo);
}

// ***

// *** CParameterIntegratedCreator ***

CParameter* CParameterIntegratedCreator::createIntPar( const CParameterDefinition *pInfo ) {

	return new CParameterIntegrated<int>(pInfo);
}

CParameter* CParameterIntegratedCreator::createStrPar( const CParameterDefinition *pInfo ) {

	return new CParameterIntegrated<QString>(pInfo);
}

CParameter* CParameterIntegratedCreator::createDoublePar( const CParameterDefinition *pInfo ) {

	return new CParameterIntegrated<double>(pInfo);
}

CParameter* CParameterIntegratedCreator::createShortPar( const CParameterDefinition *pInfo ) {

	return new CParameterIntegrated<short>(pInfo);
}

// ***

// *** CParameterGroupCreator ***

CParameter* CParameterGroupCreator::createIntPar( const CParameterDefinition *pInfo ) {

	return new CParameterGroup<int>(pInfo);
}

CParameter* CParameterGroupCreator::createStrPar( const CParameterDefinition *pInfo ) {

	return new CParameterGroup<QString>(pInfo);
}

CParameter* CParameterGroupCreator::createDoublePar( const CParameterDefinition *pInfo ) {

	return new CParameterGroup<double>(pInfo);
}

CParameter* CParameterGroupCreator::createShortPar( const CParameterDefinition *pInfo ) {

	return new CParameterGroup<short>(pInfo);
}

bool CParameterGroupCreator::isValidParameterDefinition( const CParameterDefinition *pinfo ) {

	return dynamic_cast<const CGroupDefinition *>(pinfo);
}

// ***

// *** CParameterBuilder ***
CParameterBuilder::CParameterBuilder() {

	m_qmapCreators.insert(ePT_Value,	new CParameterTemplateCreator	);
	m_qmapCreators.insert(ePT_Delayed,	new CParameterDelayedCreator	);
	m_qmapCreators.insert(ePT_Integrated,	new CParameterIntegratedCreator	);
	m_qmapCreators.insert(ePT_Group,	new CParameterGroupCreator	);
}

CParameterBuilder::~CParameterBuilder() {

	foreach( CParameterAbstractCreator* creator, m_qmapCreators.values() )
		delete creator;
}

CParameter* CParameterBuilder::build( ePP_ParameterType ptype, ePP_ValueType vtype, const CParameterDefinition *pInfo ) {

	if( !m_qmapCreators.contains(ptype) )
		throw ex_base	(
				ex_base::error,
				QString("not supported parameter type: %1").arg(ptype),
				"CParameterBuilder::build"
				);

	if( vtype == eVT_Int	)	return m_qmapCreators[ptype]->createIntPar	( pInfo );
	if( vtype == eVT_Double	)	return m_qmapCreators[ptype]->createDoublePar	( pInfo );
	if( vtype == eVT_String	)	return m_qmapCreators[ptype]->createStrPar	( pInfo );
	if( vtype == eVT_Short	)	return m_qmapCreators[ptype]->createShortPar	( pInfo );

	throw ex_base	(
			ex_base::error,
			QString("not supported value type: %1").arg(vtype),
			"CParameterBuilder::build"
			);

	return 0;
}

// ***

