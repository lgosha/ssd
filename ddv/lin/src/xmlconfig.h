//<codepage = utf8
#ifndef __XMLCONFIG_H__
#define __XMLCONFIG_H__

// פאיכû qt
#include <QStringList>

// פאיכû xerces 
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE

class XMLConfigErrorHandler : public ErrorHandler
{
public:
    XMLConfigErrorHandler() : m_bSawErrors(false) {};
    ~XMLConfigErrorHandler() {};

    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    bool getSawErrors() const;
	QStringList getErrors() { return m_lstrErrors; }

private:
    bool m_bSawErrors;

	QStringList m_lstrErrors;
};

inline bool XMLConfigErrorHandler::getSawErrors() const
{
    return m_bSawErrors;
}

class StrX
{
public :
	  StrX(const XMLCh* const xmlchars) { charRep = XMLString::transcode(xmlchars); }
	 ~StrX() { XMLString::release(&charRep); }

     const char*  char_str() const { return charRep; }
 
private :
     char  *charRep;
};

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <memory>
#include <string>

using namespace std;

class XMLConfig {
private:

public:

	explicit XMLConfig(const QByteArray &);
	~XMLConfig();

	const DOMNode* root() const { return m_aptrXMLDOMParser->getDocument()->getDocumentElement(); }
	const DOMNode* first(const DOMNode * = 0);
	const DOMNode* next();

	static const QString toQString(const XMLCh* const toTranscode) { return StrX(toTranscode).char_str(); }
	static const QByteArray toByteArray(const XMLCh* const toTranscode) { return StrX(toTranscode).char_str(); }

	bool isAttr(const DOMNode *, const QString &) const;
	QString attr(const DOMNode *, const QString &) const;
	QString attr(const QString &, const QString & = "name", const QString & = "value");

	static const DOMNode* find(const DOMNode *, const QString &, const QString &);

private:
	XMLConfig();
	void open();

	static DOMNode* getNodeAttribute(const DOMNode *, const QString &);

private:
	QByteArray m_strFileName;
	auto_ptr<XercesDOMParser> m_aptrXMLDOMParser;
	DOMNode *m_pNode;
};

#endif

