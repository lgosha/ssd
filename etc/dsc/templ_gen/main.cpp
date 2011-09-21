#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QString>

// файлы проекта
#include "xmlconfig.h"
#include "exception.h"

// файлы xerces
#include <xercesc/util/PlatformUtils.hpp>

int XMLInit() {

	try {
		XMLPlatformUtils::Initialize();
	}
	catch(const XMLException &ex) {

		qDebug() << QString( "can't initialize XML library: " + XMLConfig::toQString(ex.getMessage()) );
		return 2;
	}
	catch(...) {

		qDebug() << "can't initialize XML library: unknown exception";
		return 3;
	}

	return 0;
}

XMLConfig *pXMLConfig = 0;
int create_configuration( const QString &file ) {

	try {
		pXMLConfig = new XMLConfig( file.toAscii() );
	}
	catch(const ex_base &ex) {
		qDebug() << QStringList("failed") + ex.getErrors();
		return 4;
	}
	catch(...) {
		qDebug() << QStringList("failed") + QStringList("Unknown exception during parsing");
		return 5;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	QString sFile;
	for(int i=1;i<argc;i++) {
		if( QString(argv[i]) == "--file" ) {
			if( ++i < argc ) { sFile = argv[i]; continue; }
		}
	}

	XMLInit();
	int iRes = 0;
	
	iRes = create_configuration( sFile );
	
	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "", ""));
	QString sComment;
	
	QFile file( sFile + ".comment" );
	file.open( QIODevice::WriteOnly ); 

	QFile filer( sFile + ".reserve" );
	filer.open( QIODevice::WriteOnly ); 

	int iReverse = -1;
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::COMMENT_NODE) {
				
			sComment = XMLConfig::toQString( pNode->getNodeValue() );
			sComment.replace( QRegExp("\\d{9}\\s+"), "" );

			iReverse = -1;
			iReverse = sComment.indexOf("(r)");
		}
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
		
		    if( !pXMLConfig->isAttr( pNode, "send" ) || pXMLConfig->attr( pNode, "send" ) != "no" ) {
		    
				file.write( QString( "%1 - <!-- %2 -->\n").arg(pXMLConfig->attr(pNode, "id")).arg(sComment).toAscii() );

				QString str = pXMLConfig->attr(pNode, "id").right(3);
				if( str.toInt() > 1 && str.toInt() < 100 ) {
					filer.write( QString("%1 - %2\n").arg(pXMLConfig->attr(pNode, "id")).arg( iReverse ).toAscii() );
				}
		    
				if( pXMLConfig->attr( pNode, "vtype" ) != "int" )
					printf( "\t\t%s <!-- %s -->\n", QString("<through id=\"%1\" serv=\"%SERV\" vtype=\"%2\"\t/>").arg(pXMLConfig->attr(pNode, "id")).arg(pXMLConfig->attr(pNode, "vtype")).toAscii().constData(), sComment.toAscii().constData() );
				else
					printf( "\t\t%s <!-- %s -->\n", QString("<through id=\"%1\" serv=\"%SERV\" vtype=\"%2\"\t\t/>").arg(pXMLConfig->attr(pNode, "id")).arg(pXMLConfig->attr(pNode, "vtype")).toAscii().constData(), sComment.toAscii().constData() );
				}
		}

		pNode = pXMLConfig->next();
	}
	
	file.close();
	filer.close();

	return 0;
}
