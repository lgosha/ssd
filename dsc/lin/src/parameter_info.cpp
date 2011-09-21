//<codepage = utf8
#include "parameter_info.h"

ulong CParameterProperties::_ulDigitBase = 10;

ulong CParameterProperties::toULongWithClean(const QString &str) {

	QString sTmp = str; sTmp = sTmp.left(sTmp.indexOf(".."));

	return toULong( sTmp );
}

ulong CParameterProperties::toULong( ushort usCab, ushort usDev, ushort usPar ) {

	return QString("%1%2%3")
			.arg( usCab, 3, 10, QChar('0') )
			.arg( usDev, 3, 10, QChar('0') )
			.arg( usPar, 3, 10, QChar('0') ).toULong();
}

ulong CParameterProperties::toULong(const QString &str) {

	return str.left(c_uiIdPrecision).toInt()*_ulDigitBase*_ulDigitBase + str.mid(c_uiIdPrecision, c_uiIdPrecision).toInt()*_ulDigitBase + str.right(c_uiIdPrecision).toInt();
}

QString CParameterProperties::toStr(ulong number) {

	return QString("%1").arg(number, 3*c_uiIdPrecision, 10, QChar('0'));
}

QMap<QString, ePP_ValueType>		CParameterProperties::_qmapStrValueTypeToEnum 		= QMap<QString, ePP_ValueType>();

QMap<ePP_ValueType, QString>		CParameterProperties::_qmapEnumValueTypeToStr 		= QMap<ePP_ValueType, QString>();

void CParameterProperties::Init() {

	for(uint i=0;i<c_uiIdPrecision-1;i++)
		_ulDigitBase *= 10;

	_qmapStrValueTypeToEnum.insert("int",		eVT_Int);
	_qmapStrValueTypeToEnum.insert("double",	eVT_Double);
	_qmapStrValueTypeToEnum.insert("string",	eVT_String);
	_qmapStrValueTypeToEnum.insert("short",		eVT_Short);

	foreach( QString str, _qmapStrValueTypeToEnum.keys() )
		_qmapEnumValueTypeToStr[_qmapStrValueTypeToEnum[str]] = str;
}

ePP_ValueType CParameterProperties::StrValueTypeToEnum( const QString &vtype) {

	return _qmapStrValueTypeToEnum[vtype];
}

QString CParameterProperties::EnumValueTypeToStr( ePP_ValueType vtype) {

	return _qmapEnumValueTypeToStr[vtype];
}

CParameterProperties::CParameterProperties( XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> & list ) :
	m_eValueType		( _qmapStrValueTypeToEnum[pConfig->attr(pNode, "vtype")] ),
	m_sParType		( XMLConfig::toQString(pNode->getNodeName()) == "common" ? "common" : "value" ),
	m_bToSend		( true ),
	m_qlistClientsId	( list )
{
	if(pConfig->isAttr(pNode, "send")) {
		if( pConfig->attr(pNode, "send") == "no" ) m_bToSend = false;
	}
}

CParameterProperties::CParameterProperties( const CParameterProperties *other_ ) :
	m_eValueType		(other_->m_eValueType),
	m_sParType		(other_->m_sParType),
	m_bToSend		(other_->m_bToSend)
{
}


CParameterProperties::CParameterProperties( const QString &type, const QList<uint> &list, bool bSend ) :
	m_eValueType		( StrValueTypeToEnum(type) ),
	m_sParType		( "value" ),
	m_qlistClientsId	( list ),
	m_bToSend		( bSend )
{
}

/************************************************************************************************************************************************/

CParameterDefinition::CParameterDefinition(
				ulong			  ulId,
				XMLConfig		* pConfig,
				const DOMNode		* pNode,
				const QList<uint> &	  list	
					   ) : 
#define base CParameterProperties::getDigitBase()
	// устанавливаем обязательные поля
	m_ulId		( ulId ),
	m_usSHWSId	( m_ulId / (base*base) ),
	m_usDevId	( m_ulId / base - m_usSHWSId * base ),
	m_usParId	( m_ulId - (m_usSHWSId * base*base + m_usDevId*base) ),
	m_pProps	(new CParameterProperties(pConfig, pNode, list))
#undef base
{
}


CParameterDefinition::CParameterDefinition	(
				ulong ulId,
				const QString &type,
				const QList<uint> &list,
				bool bSend

			) :
#define base CParameterProperties::getDigitBase()
	// устанавливаем обязательные поля
	m_ulId		( ulId ),
	m_usSHWSId	( m_ulId / (base*base) ),
	m_usDevId	( m_ulId / base - m_usSHWSId * base ),
	m_usParId	( m_ulId - (m_usSHWSId * base*base + m_usDevId*base) ),
	m_pProps	( new CParameterProperties(type, list, bSend) )
#undef base
{
}

CParameterDefinition::~CParameterDefinition() {

	delete m_pProps;
}

