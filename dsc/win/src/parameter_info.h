//<codepage = utf8
#ifndef __PARAMETER_INFO_H__
#define __PARAMETER_INFO_H__

#include <QString>
#include <QMap>

#include "parameter_enum.h"
#include "xmlconfig.h"

const uint c_uiIdPrecision = 3;

#define QStringMap QMap<QString, QString>

class CParameterProperties {

public:
	CParameterProperties	(
				XMLConfig *,
				const DOMNode *,
				const QList<uint> &
				);
	CParameterProperties( const QString &, const QList<uint> &, bool = true );
	CParameterProperties( const CParameterProperties * );

public:
	static ulong getDigitBase() { return _ulDigitBase; }

	static ulong	toULong				( ushort, ushort, ushort );
	static ulong	toULong				( const QString & );
	static ulong 	toULongWithClean	( const QString & );
	static QString	toStr				( ulong );

	static void Init();

	static ePP_ValueType	StrValueTypeToEnum	( const QString & );
	static QString			EnumValueTypeToStr	( ePP_ValueType );

public:
	ePP_ValueType	m_eValueType;
	QString			m_sParType;
	bool			m_bToSend;
	QList<uint>		m_qlistClientsId;

private:
	static QMap<QString, ePP_ValueType>		_qmapStrValueTypeToEnum;

	static QMap<ePP_ValueType,	QString>	_qmapEnumValueTypeToStr;

	static ulong _ulDigitBase;
};

class CParameterDefinition {
public:
	CParameterDefinition	(
					ulong,
					XMLConfig *,
					const DOMNode *,
					const QList<uint> &
				);
	CParameterDefinition	(
					ulong,
					const QString &,
					const QList<uint> &,
					bool = true
				);
	virtual ~CParameterDefinition();
public:
	ulong				 m_ulId;
	ushort				 m_usSHWSId;
	ushort				 m_usDevId;
	ushort				 m_usParId;
	const CParameterProperties	*m_pProps;
};

#endif

