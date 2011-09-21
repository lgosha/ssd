//<codepage = utf8
#include "mwnd.h"
#include "udp_pvar.h"
#include "udp_client.h"

#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>

UTreeWidget::UTreeWidget( QWidget *parent ) :
	QTreeWidget	( parent ),
	m_pEmpty	( new QTreeWidgetItem( this ) ),
	m_uiCount	( 1 )
{
	setColumnCount( 5 );
	setSelectionMode( QAbstractItemView::NoSelection );
	setHeaderLabels( QStringList() << "Number" << "Identifier" << "Timestamp" << "Status" << "Value" );

	connect	(
		this,
		SIGNAL( currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * ) ),
		this,
		SLOT( itemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) )
		);
}

void UTreeWidget::createItem( uint uiRow ) {

	m_mItems[uiRow] = new QTreeWidgetItem( this );
}


uint UTreeWidget::getParId( uint uiCab, uint uiDev, uint uiPar ) {

	return QString("%1%2%3")
			.arg( uiCab, 3, 10, QChar('0') )
			.arg( uiDev, 3, 10, QChar('0') )
			.arg( uiPar, 3, 10, QChar('0') ).toUInt();
}

QString UTreeWidget::getParStrId( uint uiCab, uint uiDev, uint uiPar ) {

	return QString("%1%2%3")
			.arg( uiCab, 3, 10, QChar('0') )
			.arg( uiDev, 3, 10, QChar('0') )
			.arg( uiPar, 3, 10, QChar('0') );
}

void UTreeWidget::find( const QString &sId ) {

	if	(
	    m_mIds.contains( getParId( sId.left(3).toUInt(), sId.mid(3, 3).toUInt(), sId.right(3).toUInt() ) )
	    &&
	    m_mItems.contains( m_mIds[ getParId( sId.left(3).toUInt(), sId.mid(3, 3).toUInt(), sId.right(3).toUInt() ) ] )
		) {

		setCurrentItem( m_mItems[m_mIds[ getParId( sId.left(3).toUInt(), sId.mid(3, 3).toUInt(), sId.right(3).toUInt() ) ]]) ;
	}
	else QMessageBox::information( 0, "Error", "No such Id" );
}

QColor UTreeWidget::itemBackgroundColor( QTreeWidgetItem *pItem, uint uiStatus ) {

	QColor color = QColor( 255, 255, 255 );
	switch( uiStatus ) {
		case 0:
			pItem == currentItem() ? color = QColor( 255, 0, 0 ) : color = QColor( 255, 150, 150 );
		break;
		case 1:
			pItem == currentItem() ? color = QColor( 0, 255, 0 ) : color = QColor( 150, 255, 150 );
		break;
		case 2:
			pItem == currentItem() ? color = QColor( 100, 100, 100 ) : color = QColor( 200, 200, 200 );
		break;
	}

	return color;
}

void UTreeWidget::setItemBackgroundColor( QTreeWidgetItem *pItem, uint uiStatus ) {

	QColor color = itemBackgroundColor( pItem, uiStatus );
	for(int i=0;i<5;i++) pItem->setBackground( i, color );
}


void UTreeWidget::itemChanged( QTreeWidgetItem *c, QTreeWidgetItem *p ) {

	if( p && p != m_pEmpty )
		setItemBackgroundColor( p, p->text( 3 ).toUInt() );
	if( c != m_pEmpty )
		setItemBackgroundColor( c, c->text( 3 ).toUInt() );
}

void UTreeWidget::hideItems() {

	foreach( QTreeWidgetItem *pItem, m_mItems.values() ) {
		setItemBackgroundColor( pItem, 3 );
		pItem->setText( 2, QString::number(2) );
	}
}


void UTreeWidget::setData( pAnswer &pa ) {

	pVariable pv;
	for(int i = 0; i < pa.getVariableCount(); i++) {

		pv = pa.getVariable(i);
		uint uiId = getParId( pv.getPtsID(), pv.getDeviceID(), pv.getN() );

		if( !m_mIds.contains( uiId ) ) {
			m_mIds[ uiId ] = m_uiCount++;
			m_mStrIds[ uiId ] = getParStrId( pv.getPtsID(), pv.getDeviceID(), pv.getN() );
			createItem( m_mIds[ uiId ] );
		}

		m_mItems[m_mIds[ uiId ]]->setText( 0, QString::number(m_mIds[ uiId ] ) );
		m_mItems[m_mIds[ uiId ]]->setText( 1, m_mStrIds[ uiId ] );
		m_mItems[m_mIds[ uiId ]]->setText( 2, pv.getDateTime().toString("hh:mm:ss.zzz") );
		m_mItems[m_mIds[ uiId ]]->setText( 3, QString::number(pv.getStatus()) );
		m_mItems[m_mIds[ uiId ]]->setText( 4, pv.getValue().toStr() );

		setItemBackgroundColor( m_mItems[m_mIds[ uiId ]], pv.getStatus() );
	}
}

#include <QTabWidget>
#include <QAction>

CMainWnd::CMainWnd( XMLConfig *pXMLConfig ) :
	m_pClient( new UDPClient( this, pXMLConfig ) )
{
	setGeometry	(
			pXMLConfig->attr("view:x").toInt(), pXMLConfig->attr("view:y").toInt(),
			pXMLConfig->attr("view:width").toInt(), pXMLConfig->attr("view:height").toInt()
			);
	setFixedSize(pXMLConfig->attr("view:width").toInt(), pXMLConfig->attr("view:height").toInt());

	m_pTabContainer = new QTabWidget(this);
	m_pTabContainer->setGeometry(QRect(10, 10, pXMLConfig->attr("view:width").toInt() - 17, pXMLConfig->attr("view:height").toInt() - 20));

	const DOMNode *pNode = pXMLConfig->first(XMLConfig::find(pXMLConfig->root(), "servers", ""));
	while(pNode) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE)
			createView( pXMLConfig, pNode );
		pNode = pXMLConfig->next();
	}
}

CMainWnd::~CMainWnd()
{
}

#include <QKeyEvent>
#include <QInputDialog>

void CMainWnd::keyPressEvent( QKeyEvent *pEvent ) {

	if( pEvent->matches( QKeySequence::Find )  ) {
		QString str = QInputDialog::getText( this, "Find", "Input signal identifier" );
		if( !str.isNull() && !str.isEmpty() )
			m_mItemsByIndex[m_pTabContainer->currentIndex()]->find( str );
	}
}


void CMainWnd::createView( XMLConfig *pXMLConfig, const DOMNode *pNode ) {

	QWidget *pTab; UTreeWidget *pTreeWidget = 0;
	pTab = new QWidget( m_pTabContainer );
	pTreeWidget = new UTreeWidget( pTab );
	pTreeWidget->setGeometry( QRect(5, 5, pXMLConfig->attr("view:width").toInt() - 32, pXMLConfig->attr("view:height").toInt() - 56) );
	m_mTabs[pXMLConfig->attr(pNode, "id").toUInt()] = m_pTabContainer->addTab( pTab, QIcon( "../img/16x16/red.png" ), pXMLConfig->attr(pNode, "caption") );
	m_mViews[pXMLConfig->attr(pNode, "id").toUInt()] = pTreeWidget;
	m_mItemsByIndex[ m_mTabs[pXMLConfig->attr(pNode, "id").toUInt()] ] = pTreeWidget;
}


void CMainWnd::setData( uint uiId, pAnswer &answer ) {

	if( m_mViews.contains( uiId ) )
		m_mViews[uiId]->setData( answer );
}

void CMainWnd::setConnectStatus( uint uiId, uint uiStatus) {

	if( m_mViews.contains( uiId ) && m_mTabs.contains( uiId ) ) {
		switch( uiStatus ) {
		case 0:
			m_pTabContainer->setTabIcon( m_mTabs[uiId], QIcon( "../img/16x16/red.png" ) );
			m_mViews[uiId]->hideItems();
		break;
		case 1:
			m_pTabContainer->setTabIcon( m_mTabs[uiId], QIcon( "../img/16x16/green.png" ) );
		break;
		}
	}
}

