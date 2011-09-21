#ifndef __MWND_H__
#define __MWND_H__

#include <QTreeWidget>

#include "udp_panswer.h"

class UTreeWidget : public QTreeWidget {
	Q_OBJECT
public:
	UTreeWidget( QWidget * );

	void setData( pAnswer & );
	void find( const QString & );
	void hideItems();
private:
	void createItem( uint );
private:
	static uint getParId( uint, uint, uint );
	static QString getParStrId( uint, uint, uint );
	QColor itemBackgroundColor( QTreeWidgetItem *, uint );
	void setItemBackgroundColor( QTreeWidgetItem *, uint );
private slots:
	void itemChanged( QTreeWidgetItem *, QTreeWidgetItem * );
private:
	QMap<uint, QTreeWidgetItem *>	  m_mItems;
	QMap<uint, uint>		  m_mIds;
	QMap<uint, QString>		  m_mStrIds;
	uint				  m_uiCount;
	QTreeWidgetItem			* m_pEmpty;
};

#include <QtGui/QDialog>

#include "xmlconfig.h"

class QTabWidget;
class UDPClient;

class CMainWnd : public QDialog {
	Q_OBJECT
public:
	CMainWnd( XMLConfig *pXMLConfig );
	~CMainWnd();

	void setData( uint, pAnswer & );
	void setConnectStatus( uint, uint );
protected slots:
	void keyPressEvent ( QKeyEvent * );
private:
	void createView( XMLConfig *, const DOMNode * );
private:
	QTabWidget			* m_pTabContainer;
	QMap<uint, UTreeWidget*>	  m_mViews;
	QMap<uint, UTreeWidget *>	  m_mItemsByIndex;
	QMap<uint, int>			  m_mTabs;
	UDPClient			* m_pClient;
};


#endif

