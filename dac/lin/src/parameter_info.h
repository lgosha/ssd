//<codepage = utf8
#ifndef __PARAMETER_INFO_H__
#define __PARAMETER_INFO_H__

#include <QString>
#include <QMap>
#include <QPair>
#include <QVector>

#include "parameter_enum.h"
#include "xmlconfig.h"

const uint c_uiIdPrecision = 3;

#define QStringMap QMap<QString, QString>

class CParameterProperties {

public:
	CParameterProperties	(
				XMLConfig *,
				const DOMNode *,
				const QList<uint> &,
				const QMap<QString, int> &
				);
	CParameterProperties( const CParameterProperties *, bool = false );

public:
	static ulong getDigitBase() { return _ulDigitBase; }

	static ulong	toULong			(const QString &);
	static ulong 	toULongWithClean	(const QString &);
	static QString	toStr			(ulong);

	static void Init();

	static ePP_QueryType		StrQueryTypeToEnum	( const QString &);
	static ePP_ValueType		StrValueTypeToEnum	( const QString &);
	static ePP_ParameterType	StrParameterTypeToEnum	( const QString &);
	static QString			EnumQueryTypeToStr	( ePP_QueryType);
	static QString			EnumValueTypeToStr	( ePP_ValueType);
	static QString			EnumParameterTypeToStr	( ePP_ParameterType);

public:
	uint		m_uiUpdateTime;
	uint		m_iStart;
	uint		m_uiDelay;
	QString		m_sAddr;
	QString		m_sProperty;
	ePP_QueryType	m_eQueryType;
	ePP_ValueType	m_eValueType;
	QString		m_sRexp;
	QString		m_sMath;
	QString		m_sValDepParId;
	QString		m_sFailValue;
	QString		m_sGoodValue;
	bool		m_bToSend;
	bool		m_bSync;
	bool		m_bInvert;
	QStringMap	m_qmapValuesList;
	QStringList	m_qlistAccept;
	QStringList	m_qlistDenied;
	QStringList	m_qlistIgnore;
	QList<uint>	m_qlistClientId;

	QVector< QPair<QString, QString> > m_vExpressions;
	QVector< QString > m_vLogicOperations;

private:
	static QMap<QString, ePP_QueryType>		_qmapStrQueryTypeToEnum;
	static QMap<QString, ePP_ValueType>		_qmapStrValueTypeToEnum;
	static QMap<QString, ePP_ParameterType>		_qmapStrParameterTypeToEnum;

	static QMap<ePP_QueryType,	QString>	_qmapEnumQueryTypeToStr;
	static QMap<ePP_ValueType,	QString>	_qmapEnumValueTypeToStr;
	static QMap<ePP_ParameterType,	QString>	_qmapEnumParameterTypeToStr;

	static ulong _ulDigitBase;
};

class CParameterDefinition {
public:
	CParameterDefinition	(
					ulong,
					XMLConfig *,
					const DOMNode *,
					const QList<uint> &,
					const QMap<QString, int> &
				);
	CParameterDefinition( const CParameterDefinition *);
	CParameterDefinition( ulong, const CParameterProperties *);
	virtual ~CParameterDefinition();
public:
	ulong				 m_ulId;
	ushort				 m_usSHWSId;
	ushort				 m_usDevId;
	ushort				 m_usParId;
	const CParameterProperties	*m_pProps;
private:
	bool m_bDeleteProps;
};

class CGroupDefinition : public CParameterDefinition {
public:
	CGroupDefinition(ulong, XMLConfig *pConfig, const DOMNode *pNode, const QList<uint> &, const QMap<QString, int> &);
public:
	uint				  m_uiBaseParNumber;
	uint				  m_uiParMaxCount;
	ePP_ParameterType		  m_eParType;
	ulong				  m_ulIdentId;
	QList<CParameterDefinition *>	  m_listParDefinition;
	const CParameterDefinition	* m_pDefinition;

};

#endif


