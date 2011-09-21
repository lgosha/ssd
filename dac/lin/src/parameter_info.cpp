//<codepage = utf8
#include "parameter_info.h"

ulong CParameterProperties::_ulDigitBase = 10;

ulong CParameterProperties::toULongWithClean(const QString &str) {

	QString sTmp = str.right(str.length() - 1); sTmp = sTmp.left(sTmp.indexOf(".."));

	return toULong(sTmp);
}

ulong CParameterProperties::toULong(const QString &str) {

	return str.left(c_uiIdPrecision).toInt()*_ulDigitBase*_ulDigitBase + str.mid(c_uiIdPrecision, c_uiIdPrecision).toInt()*_ulDigitBase + str.right(c_uiIdPrecision).toInt();
}

QString CParameterProperties::toStr(ulong number) {

	return QString("%1").arg(number, 3*c_uiIdPrecision, 10, QChar('0'));
}

QMap<QString, ePP_QueryType>		CParameterProperties::_qmapStrQueryTypeToEnum 		= QMap<QString, ePP_QueryType>();
QMap<QString, ePP_ValueType>		CParameterProperties::_qmapStrValueTypeToEnum 		= QMap<QString, ePP_ValueType>();
QMap<QString, ePP_ParameterType>	CParameterProperties::_qmapStrParameterTypeToEnum 	= QMap<QString, ePP_ParameterType>();

QMap<ePP_QueryType, QString>		CParameterProperties::_qmapEnumQueryTypeToStr 		= QMap<ePP_QueryType, QString>();
QMap<ePP_ValueType, QString>		CParameterProperties::_qmapEnumValueTypeToStr 		= QMap<ePP_ValueType, QString>();
QMap<ePP_ParameterType, QString>	CParameterProperties::_qmapEnumParameterTypeToStr	= QMap<ePP_ParameterType, QString>();

void CParameterProperties::Init() {

	for(uint i=0;i<c_uiIdPrecision-1;i++)
		_ulDigitBase *= 10;

	_qmapStrQueryTypeToEnum.insert("snmp",		eQT_SNMP);
	_qmapStrQueryTypeToEnum.insert("wmi",		eQT_WMI);
	_qmapStrQueryTypeToEnum.insert("calc",		eQT_CALC);

	foreach( QString str, _qmapStrQueryTypeToEnum.keys() )
		_qmapEnumQueryTypeToStr[_qmapStrQueryTypeToEnum[str]] = str;

	_qmapStrValueTypeToEnum.insert("int",		eVT_Int);
	_qmapStrValueTypeToEnum.insert("double",	eVT_Double);
	_qmapStrValueTypeToEnum.insert("string",	eVT_String);
	_qmapStrValueTypeToEnum.insert("short",		eVT_Short);

	foreach( QString str, _qmapStrValueTypeToEnum.keys() )
		_qmapEnumValueTypeToStr[_qmapStrValueTypeToEnum[str]] = str;

	_qmapStrParameterTypeToEnum.insert("value",		ePT_Value);
	_qmapStrParameterTypeToEnum.insert("delayed",		ePT_Delayed);
	_qmapStrParameterTypeToEnum.insert("integrated",	ePT_Integrated);
	_qmapStrParameterTypeToEnum.insert("group",		ePT_Group);

	foreach( QString str, _qmapStrParameterTypeToEnum.keys() )
		_qmapEnumParameterTypeToStr[_qmapStrParameterTypeToEnum[str]] = str;

}

ePP_QueryType CParameterProperties::StrQueryTypeToEnum( const QString &qtype) {

	return _qmapStrQueryTypeToEnum[qtype];
}

ePP_ValueType CParameterProperties::StrValueTypeToEnum( const QString &vtype) {

	return _qmapStrValueTypeToEnum[vtype];
}

ePP_ParameterType CParameterProperties::StrParameterTypeToEnum( const QString &ptype) {

	return _qmapStrParameterTypeToEnum[ptype];
}

QString CParameterProperties::EnumQueryTypeToStr( ePP_QueryType qtype) {

	return _qmapEnumQueryTypeToStr[qtype];
}
QString CParameterProperties::EnumValueTypeToStr( ePP_ValueType vtype) {

	return _qmapEnumValueTypeToStr[vtype];
}
QString CParameterProperties::EnumParameterTypeToStr( ePP_ParameterType ptype) {

	return _qmapEnumParameterTypeToStr[ptype];
}

CParameterProperties::CParameterProperties(
	XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> &list, const QMap<QString, int> &map) :

	m_uiUpdateTime	( 0 ),
	m_iStart	( 0 ),
	m_uiDelay	( 0 ),
	m_eQueryType	( _qmapStrQueryTypeToEnum[XMLConfig::toQString(pNode->getNodeName())] ),
	m_eValueType	( _qmapStrValueTypeToEnum[pConfig->attr(pNode, "vtype")] ),
	m_bToSend	( true ),
	m_bInvert	( false ),
	m_bSync		( false ),
	m_qlistClientId( list )

{
	// устанавливаем не обязательные поля
	if(pConfig->isAttr(pNode, "update"))
		m_uiUpdateTime		= pConfig->attr(pNode, "update").toUInt();
	if(pConfig->isAttr(pNode, "start"))
		m_iStart		= pConfig->attr(pNode, "start").toInt();
	if(pConfig->isAttr(pNode, "addr"))
		m_sAddr			= pConfig->attr(pNode, "addr");
	if(pConfig->isAttr(pNode, "prop"))
		m_sProperty		= pConfig->attr(pNode, "prop");
	if(pConfig->isAttr(pNode, "rexp"))
		m_sRexp			= pConfig->attr(pNode, "rexp");
	if(pConfig->isAttr(pNode, "math"))
		m_sMath		= pConfig->attr(pNode, "math");
	if(pConfig->isAttr(pNode, "vdep"))
		m_sValDepParId	= pConfig->attr(pNode, "vdep");
	if(pConfig->isAttr(pNode, "fval"))
		m_sFailValue	= pConfig->attr(pNode, "fval");
	if(pConfig->isAttr(pNode, "gval"))
		m_sGoodValue	= pConfig->attr(pNode, "gval");
	if(pConfig->isAttr(pNode, "delay"))
		m_uiDelay	= pConfig->attr(pNode, "delay").toUInt();
	if(pConfig->isAttr(pNode, "send")) {
		if( pConfig->attr(pNode, "send") == "no" ) m_bToSend = false;
	}
	if(pConfig->isAttr(pNode, "invert")) {
		if( pConfig->attr(pNode, "invert") == "yes" ) m_bInvert = true;
	}
	if(pConfig->isAttr(pNode, "sync")) {
		if( pConfig->attr(pNode, "sync") == "yes" ) m_bSync = true;
	}

	// get start timeout
	if( m_iStart == -1 ) {
		// get rand start delay
		if( map.contains( XMLConfig::toQString( pNode->getNodeName() ) )  ) {
			// rand start
			m_iStart = (int)(m_uiUpdateTime*(float)(RAND_MAX - rand())/RAND_MAX);
			// normalize start
			int iThreadUpdate = map[ XMLConfig::toQString( pNode->getNodeName() ) ];
			int iTmp = iThreadUpdate;
			while( iTmp < m_iStart ) iTmp += iThreadUpdate;
			m_iStart = iTmp;
			while( m_iStart >= m_uiUpdateTime ) m_iStart -= iThreadUpdate;
		}
		else m_iStart = 0;
	}

	// сохраняем соответствия значения параметра, логические выражения и
	// списки разрешенных, запрещенных и игнорируемых значений для интегральных групповых параметров
	const DOMNode *pChildNode = pNode->getFirstChild();
	while(pChildNode) {
		if(pChildNode->getNodeType() == DOMNode::ELEMENT_NODE && XMLConfig::toQString(pChildNode->getNodeName()) == "rule")
			m_qmapValuesList[pConfig->attr(pChildNode, "key")] = pConfig->attr(pChildNode, "value");
		if(pChildNode->getNodeType() == DOMNode::ELEMENT_NODE && XMLConfig::toQString(pChildNode->getNodeName()) == "item") {
			if(pConfig->attr(pChildNode, "type") == "accept") m_qlistAccept.append(pConfig->attr(pChildNode, "name"));
			if(pConfig->attr(pChildNode, "type") == "denied") m_qlistDenied.append(pConfig->attr(pChildNode, "name"));
			if(pConfig->attr(pChildNode, "type") == "ignore") m_qlistIgnore.append(pConfig->attr(pChildNode, "name"));
		}
		if(pChildNode->getNodeType() == DOMNode::ELEMENT_NODE && XMLConfig::toQString(pChildNode->getNodeName()) == "logic") {
			m_vExpressions.append( qMakePair( pConfig->attr(pChildNode, "opCompare"), pConfig->attr(pChildNode, "val") ) );
			m_vLogicOperations.append( pConfig->attr(pChildNode, "opLogic") );
		}
		pChildNode = pChildNode->getNextSibling();
	}
}

CParameterProperties::CParameterProperties(const CParameterProperties *other_, bool bResetValueRule) :

	m_uiUpdateTime		(other_->m_uiUpdateTime),
	m_uiDelay		(other_->m_uiDelay),
	m_sAddr			(other_->m_sAddr),
	m_sProperty		(other_->m_sProperty),
	m_eQueryType		(other_->m_eQueryType),
	m_eValueType		(other_->m_eValueType),
	m_sRexp			(other_->m_sRexp),
	m_sMath			(other_->m_sMath),
	m_sValDepParId		(other_->m_sValDepParId),
	m_sFailValue		(other_->m_sFailValue),
	m_sGoodValue		(other_->m_sGoodValue),
	m_bToSend		(other_->m_bToSend),
	m_bSync			(other_->m_bSync),
	m_qmapValuesList	(other_->m_qmapValuesList),
	m_qlistClientId		(other_->m_qlistClientId)
{
	bResetValueRule ? m_sMath = "" : m_sMath = other_->m_sMath;
	bResetValueRule ? m_sRexp = "" : m_sRexp = other_->m_sRexp;
}

/************************************************************************************************************************************************/

CParameterDefinition::CParameterDefinition(
						ulong			  ulId,
						XMLConfig		* pConfig,
						const	DOMNode		* pNode,
						const	QList<uint>	& listClientsId,
						const	QMap<QString, int> &map
					   ) : 
#define base CParameterProperties::getDigitBase()
	// устанавливаем обязательные поля
	m_ulId		( ulId ),
	m_usSHWSId	( m_ulId / (base*base) ),
	m_usDevId	( m_ulId / base - m_usSHWSId * base ),
	m_usParId	( m_ulId - (m_usSHWSId * base*base + m_usDevId*base) ),
	m_pProps	(new CParameterProperties(pConfig, pNode, listClientsId, map)),
	m_bDeleteProps	(true)
#undef base
{
}

CParameterDefinition::CParameterDefinition(const CParameterDefinition *other_) :
	m_ulId			(other_->m_ulId),
	m_usSHWSId		(other_->m_usSHWSId),
	m_usDevId		(other_->m_usDevId),
	m_usParId		(other_->m_usParId),
	m_pProps		(new CParameterProperties(other_->m_pProps, true)),
	m_bDeleteProps		(true)
{
}

CParameterDefinition::CParameterDefinition(ulong ulId, const CParameterProperties * pProps) : 
#define base CParameterProperties::getDigitBase()
	m_ulId		( ulId ),
	m_usSHWSId	( m_ulId / (base*base) ),
	m_usDevId	( m_ulId / base - m_usSHWSId * base ),
	m_usParId	( m_ulId - (m_usSHWSId * base*base + m_usDevId*base) ),
	m_pProps	( pProps ),
	m_bDeleteProps (false)
#undef base
{
}

CParameterDefinition::~CParameterDefinition() {

	if(m_bDeleteProps) delete m_pProps;
}

CGroupDefinition::CGroupDefinition(ulong ulId, XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> &list, const	QMap<QString, int> &map) :
	CParameterDefinition	( ulId, pConfig, pNode, list, map ),
	m_eParType		( CParameterProperties::StrParameterTypeToEnum(pConfig->attr(pNode, "ptype")) )
 {

	QString sTmp = pConfig->attr(pNode, "id").right(pConfig->attr(pNode, "id").length() - 1);
	m_uiBaseParNumber = sTmp.mid(2*c_uiIdPrecision, c_uiIdPrecision).toUInt();
	m_uiParMaxCount = sTmp.right(c_uiIdPrecision).toUInt() - m_uiBaseParNumber + 1;

	CParameterProperties *pProperties = new CParameterProperties(pConfig, pNode, list, map);

	CParameterDefinition *pParDefinition;
	for(uint i=m_uiBaseParNumber;i<m_uiBaseParNumber+m_uiParMaxCount;++i) {

		pParDefinition = new CParameterDefinition	(
					QString("%1%2").arg(sTmp.left(2*c_uiIdPrecision)).arg(i, c_uiIdPrecision, 10, QChar('0')).toULong(),
					pProperties
								);

		m_listParDefinition.append(pParDefinition);
	}

	m_pDefinition = new CParameterDefinition(CParameterProperties::toULong(sTmp.left(3*c_uiIdPrecision)), pProperties);

	if(pConfig->isAttr(pNode, "ident"))
		m_ulIdentId	= CParameterProperties::toULong(pConfig->attr(pNode, "ident"));
}

