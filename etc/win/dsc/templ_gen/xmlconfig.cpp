//<codepage = utf8
#include "xmlconfig.h" 

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/dom/DOMError.hpp>

void XMLConfigErrorHandler::warning(const SAXParseException &) {
}

void XMLConfigErrorHandler::error(const SAXParseException &toCatch) {

	m_lstrErrors.push_back(
		QString("XML parsing error: %1, line %2, column %3").arg(StrX(toCatch.getMessage()).char_str())
															.arg(toCatch.getLineNumber())
															.arg(toCatch.getColumnNumber()));

    m_bSawErrors = true;
}

void XMLConfigErrorHandler::fatalError(const SAXParseException &toCatch) {

	m_lstrErrors.push_back(
		QString("XML parsing error: %1, line %2, column %3").arg(StrX(toCatch.getMessage()).char_str())
															.arg(toCatch.getLineNumber())
															.arg(toCatch.getColumnNumber()));

	m_bSawErrors = true;
}

void XMLConfigErrorHandler::resetErrors() {

    m_bSawErrors = false;
}

#include "exception.h"

XMLConfig::XMLConfig() : m_aptrXMLDOMParser(0) {
}

XMLConfig::XMLConfig(const QByteArray &strBaseFileName) : m_aptrXMLDOMParser(new XercesDOMParser), m_pNode(0) {
	
	m_strFileName = strBaseFileName;

	open();
}

XMLConfig::~XMLConfig() {
}

void XMLConfig::open() {

	auto_ptr<XMLConfigErrorHandler> aptrXMLDOMParserErrorHandler(new XMLConfigErrorHandler);

	m_aptrXMLDOMParser->setErrorHandler(aptrXMLDOMParserErrorHandler.get());
//	m_aptrXMLDOMParser->setValidationScheme(XercesDOMParser::Val_Always);
//	m_aptrXMLDOMParser->setDoNamespaces( true );
//	m_aptrXMLDOMParser->setDoSchema( true );

	try {
		m_aptrXMLDOMParser->parse(m_strFileName.data());
	}
	catch (const OutOfMemoryException &) {
		throw ex_base(ex_base::error, "XMLConfig::XMLConfig", "OutOfMemoryException");
	}
	catch (const XMLException &ex) {
		QStringList list; list.push_back(toQString(ex.getMessage()));
		throw ex_base(ex_base::error, "XMLConfig::XMLConfig", "XMLException", list);
	}
	catch (const DOMException &ex) {
        const unsigned int maxChars = 2047; XMLCh errText[maxChars + 1]; QStringList list;
		if(DOMImplementation::loadDOMExceptionMsg(ex.code, errText, maxChars)) {
			list.push_back(toQString(ex.getMessage()));
		}
		throw ex_base(ex_base::error, "XMLConfig::XMLConfig", "DOMException", list);
	}
 
	if(aptrXMLDOMParserErrorHandler->getSawErrors()) {
		throw ex_base(ex_base::error, "XMLConfig::XMLConfig", "parse errors", aptrXMLDOMParserErrorHandler->getErrors());
	}
}

const DOMNode* XMLConfig::first(const DOMNode *pParentNode) {

	pParentNode ? m_pNode = pParentNode->getFirstChild() : m_pNode = root()->getFirstChild();

	return m_pNode;
}

const DOMNode* XMLConfig::next() {

	if(m_pNode) m_pNode = m_pNode->getNextSibling();

	return m_pNode;
} 

DOMNode* XMLConfig::getNodeAttribute(const DOMNode* pNode, const QString &sName) {

	XMLCh pBuf[128]; XMLString::transcode(sName.toAscii(), pBuf, 128);

	return pNode->getAttributes()->getNamedItem(pBuf);
}

const DOMNode* XMLConfig::find(const DOMNode *pParentNode, const QString &sNodeFName, const QString &sAttrNameForSearch) {

	const DOMNode *pFindedNode = 0;
	DOMNode *pNode = pParentNode->getFirstChild();

	QString sCurName;
	if(sNodeFName.contains(":"))
		sCurName = sNodeFName.left(sNodeFName.indexOf(":"));
	else
		sCurName = sNodeFName;

	while(pNode) {

		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {

			bool bFinded = false;
			if(sAttrNameForSearch.isEmpty()) { 
				if(toQString(pNode->getNodeName()) == sCurName) bFinded = true;
			}
			else {
				DOMNode *pAttrNode = getNodeAttribute(pNode, sAttrNameForSearch);
				if(pAttrNode && toQString(pAttrNode->getNodeValue()) == sCurName) bFinded = true;
			}

			if(bFinded) {

				if(sNodeFName.contains(":")) {

					pFindedNode = find(pNode, sNodeFName.right(sNodeFName.length() - sNodeFName.indexOf(":") - 1), sAttrNameForSearch);
					if(pFindedNode) break;
				}
				else {
					pFindedNode = pNode;
					break;
				}
			}
		}

		pNode = pNode->getNextSibling();
	}

	return pFindedNode;
}

bool XMLConfig::isAttr(const DOMNode *pNode, const QString &sAttrName) const {

	if(!pNode) throw ex_base(ex_base::error, "empty node", "XMLConfig::getNodeAttributeValue");

	DOMNode *pAttrNode = getNodeAttribute(pNode, sAttrName);

	if(!pAttrNode) return false;

	return true;
}

QString XMLConfig::attr(const DOMNode *pNode, const QString &sAttrName) const {

	if(!pNode) throw ex_base(ex_base::error, QString("empty node").arg(sAttrName), "XMLConfig::attr(pNode, sAttrName)");

	DOMNode *pAttrNode = getNodeAttribute(pNode, sAttrName);

	if(!pAttrNode)
		throw ex_base(ex_base::error, 
					  QString("No such attribute '%1' in node '%2' in file '%3'")
							.arg(sAttrName)
							.arg(toQString(pNode->getNodeName()))
							.arg(m_strFileName.data()), 
					  "XMLConfig::getNodeAttributeValue");

	return toQString(pAttrNode->getNodeValue());
}

QString XMLConfig::gattr( const QString &sAttrPath ) const {

	return attr( find( root(), sAttrPath.left(sAttrPath.indexOf(":")), "" ), sAttrPath.right( sAttrPath.length() - sAttrPath.indexOf(":") - 1) );
}

QString XMLConfig::attr(const QString &sNodeFName, const QString &sAttrNameForSearch, const QString &sAttrName) {

	return attr( find( root(), sNodeFName, sAttrNameForSearch ), sAttrName );
}

QString XMLConfig::attr(const DOMNode *pParentNode, const QString &sNodeName, const QString &sAttrName, const QString &sAttrValue, const QString &sFAttrName ) const {

	if(!pParentNode) throw ex_base(ex_base::error, "empty node", "XMLConfig::attr");

	const DOMNode *pNode = pParentNode->getFirstChild();
	const DOMNode *pFindedNode = 0;
	while(pNode) {
		if( pNode->getNodeType() == DOMNode::ELEMENT_NODE && toQString(pNode->getNodeName()) == sNodeName ) {
			const DOMNode *pAttrNode = getNodeAttribute( pNode, sAttrName );
			if( pAttrNode && toQString(pAttrNode->getNodeValue()) == sAttrValue ) {
				pFindedNode = getNodeAttribute( pNode, sFAttrName );
				break;
			}
		}
		pNode = pNode->getNextSibling();
	}

	if( !pFindedNode )
		throw ex_base(ex_base::error,
					  QString("No such attribute '%1' in node '%2' with attr '%3 = %4' in file '%5'")
							.arg(sFAttrName)
							.arg(sNodeName)
							.arg(sAttrName)
							.arg(sAttrValue)
							.arg(m_strFileName.data()), 
					  "XMLConfig::attr");

	return toQString( pFindedNode->getNodeValue() );
}

QString XMLConfig::gattr(const QString &sNodeName, const QString &sAttrName, const QString &sAttrValue, const QString &sFAttrName ) const {

	return attr( find( root(), sNodeName.left(sNodeName.indexOf(":")), "" ), sNodeName.right( sNodeName.length() - sNodeName.indexOf(":") - 1), sAttrName, sAttrValue, sFAttrName );
}
