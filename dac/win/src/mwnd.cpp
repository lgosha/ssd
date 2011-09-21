//<codepage = utf8
#include "mwnd.h"

#include "cevent.h"

#include <QTabWidget>
#include <QListWidget>

CMainWnd::CMainWnd(XMLConfig *pXMLConfig) : QDialog( 0 ) {

	ui.setupUi(this);

	setGeometry	(
			pXMLConfig->gattr( "view:x"  ).toUInt(), pXMLConfig->gattr( "view:y" ).toUInt(),
			pXMLConfig->gattr( "view:width" ).toUInt(), pXMLConfig->gattr( "view:height" ).toUInt()
			);
	setFixedSize(pXMLConfig->gattr( "view:width" ).toUInt(), pXMLConfig->gattr( "view:height" ).toUInt());

	m_pTabContainer = new QTabWidget(this);
	m_pTabContainer->setGeometry(QRect(10, 10, pXMLConfig->gattr( "view:width" ).toUInt() - 17, pXMLConfig->gattr( "view:height" ).toUInt() - 20));

	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "logs", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			createView( pXMLConfig, pNode );
		}
		pNode = pXMLConfig->next();
	}
}

CMainWnd::~CMainWnd() {
}

void CMainWnd::createView(XMLConfig *pXMLConfig, const DOMNode *pNode) {

	QWidget *pTab; QListWidget *pListWidget = 0;
	pTab = new QWidget(m_pTabContainer);
	pListWidget = new QListWidget(pTab);
	pListWidget->setGeometry(QRect(5, 5, pXMLConfig->gattr("view:width").toUInt() - 32, pXMLConfig->gattr("view:height").toUInt() - 56));
	pListWidget->setSelectionMode(QAbstractItemView::NoSelection);
	pListWidget->setFont(QFont("courier new", 9));
	m_pTabContainer->insertTab(0, pTab, pXMLConfig->attr(pNode, "caption"));
	QString sTmp = pXMLConfig->attr(pNode, "id");
	m_mViews[sTmp.right(sTmp.length() - 1).toUShort()] = pListWidget;
}

