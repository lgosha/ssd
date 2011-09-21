//<codepage = utf8
#ifndef __MWND_H__
#define __MWND_H__

#include <QtGui/QDialog>
#include "ui_mwnd.h"

#include "xmlconfig.h"

class QTabWidget;
class QListWidget;

class CMainWnd : public QDialog {
	Q_OBJECT
public:
	CMainWnd(XMLConfig *pXMLConfig);
	~CMainWnd();

	QListWidget * getView( ushort usId ) { return m_mViews[usId]; }
private:
	Ui::CMainWnd ui;
private:
	void createView( XMLConfig *, const DOMNode * );
private:
	QTabWidget *m_pTabContainer;
	QMap<ushort, QListWidget *>	  m_mViews;
};

#endif


