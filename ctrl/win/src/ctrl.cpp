#include "ctrl.h"

#include <QMessageBox>
#include <QTimer>
#include <QProcess>
#include <QDir>

CCommand::CCommand( XMLConfig *pXMLConfig, QObject *pParent ) :
	m_pProcess		( new QProcess( pParent ) ),
	m_sLastCommand	( QString::null )
{
	const DOMNode *pNode = pXMLConfig->first();
	// proc node
	while( pNode ) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			QMap<QString, _Argv> m;
			const DOMNode *pNode1 = pXMLConfig->first(pXMLConfig->find( pNode, "cmds", "" ));
			// cmd node
			while( pNode1 ) {
				if(pNode1->getNodeType() == DOMNode::ELEMENT_NODE) {
					// arg node
					_Argv args;
					args.sCmd = pXMLConfig->attr( pNode1, "app" );
					if( pXMLConfig->isAttr( pNode1, "ignore" ) )
						args.sIgnore = pXMLConfig->attr( pNode1, "ignore" ).split(",");
					const DOMNode *pNode2 = pXMLConfig->first(pNode1);
					while( pNode2 ) {
						if(pNode2->getNodeType() == DOMNode::ELEMENT_NODE) {					
							args.sArgs << pXMLConfig->attr( pNode2, "name" );
						}
						pNode2 = pXMLConfig->next( pNode2 );
					} // end arg node
					m[pXMLConfig->attr( pNode1, "name" )] = args;
				}
				pNode1 = pXMLConfig->next( pNode1 );
			} // end cmd node
			m_mCommands[pXMLConfig->attr( pNode, "name" )] = m;
		}
		pNode = pXMLConfig->next( pNode );
	} // end proc node
}

CCommand::~CCommand() {

	delete m_pProcess;
}

int CCommand::exec( const QString &proc, const QString &cmd, const QMap<QString, QString> &vars, bool bSimulate ) {

	m_sLastCommand = proc + ' ' + cmd;
	if( !m_mCommands.contains( proc ) || !m_mCommands[proc].contains(cmd) ) return 1;

	update(proc, cmd, vars);

	if( !bSimulate ) m_pProcess->start( m_mCommands[proc][cmd].sCmd, m_mCommands[proc][cmd].sArgs );
	m_sLastCommand = m_mCommands[proc][cmd].sCmd + ' ' + m_mCommands[proc][cmd].sArgs.join( " " );

	restore(proc, cmd);

	if( !bSimulate && !m_pProcess->waitForStarted() )
		return 2;

	if( !bSimulate && !m_pProcess->waitForFinished( 5000 ) ) {
		m_pProcess->kill();
		if( !m_mCommands[proc][cmd].sIgnore.contains("3") )
			return 3;
	}

	m_baOutput = m_pProcess->readAll();

	QStringList lstr = QString( m_baOutput ).split("\n");
	foreach(QString str, lstr) {
		if( str.indexOf("FAILED") != -1 ) return 4;
	}

	return 0;
}

void CCommand::update( const QString &proc, const QString &cmd, const QMap<QString, QString> &vars ) {

	m_toRestore = m_mCommands[proc][cmd];

	foreach( QString str, vars.keys() )
		m_mCommands[proc][cmd].sCmd.replace( str, vars[str] );

	QStringList::iterator p, e = m_mCommands[proc][cmd].sArgs.end();
	for(p=m_mCommands[proc][cmd].sArgs.begin();p!=e;++p)
		foreach( QString str, vars.keys() )
			p->replace( str, vars[str] );
}

void CCommand::restore( const QString &proc, const QString &cmd ) {

	m_mCommands[proc][cmd] = m_toRestore;
}

QString ctrl::stateToText( uint state ) {

	switch( state ) {
		case 1: return "Stopped";
		case 2:	return "Running";
	}

	return "Unknown";
}

ctrl::ctrl( XMLConfig *pXMLConfig, QWidget *parent, Qt::WFlags flags ) :
	QMainWindow		( parent, Qt::Dialog ),
	m_pCmdExecutor	( new CCommand( pXMLConfig, this ) )
{
	m_Vars["%DIR%"] = QString::null;
	m_Vars["%CFG%"] = QString::null;

	const DOMNode *pNode = pXMLConfig->first();
	// proc node
	while( pNode ) {
		if(pNode->getNodeType() == DOMNode::ELEMENT_NODE) {
			QMap<QString, QString> m;
			if( pXMLConfig->isAttr( pNode, "path" ) ) {
				m["%DIR%" ] = pXMLConfig->attr( pNode, "path" );
				m_mEnv[ pXMLConfig->attr( pNode, "name" ) ] = m;
			}
		}
		pNode = pXMLConfig->next( pNode );
	} // end proc node

	ui.setupUi(this);

	setFixedSize( 520, 280 );
	setWindowTitle( tr(" controller") );

	ui.m_pApp_combo->addItem( "dac" );
	ui.m_pApp_combo->addItem( "dsc" );

	m_pTimer = new QTimer( this );
	connect( m_pTimer, SIGNAL(timeout()), this, SLOT(update_state_slot()) );
	m_pTimer->start( 2000 );

	ui.m_pChange_button->toggle();
	ui.m_pChange_button->toggle();

	m_uiState = 0;
	update_state_slot();
}

ctrl::~ctrl()
{
	delete m_pCmdExecutor;
}

void ctrl::update_state_slot() {

	int r;
	// get status
	r = m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "status", m_Vars );
	if( r ) { 
		reportExecError( r );
		m_uiState = 3;
		updateEnable();
		return;
	}

	int iStart = -1; int n = 0;
	while( (iStart = m_pCmdExecutor->getStdout().indexOf( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), iStart + 1 )) != -1 )
		n++;

	if( n < 1 ) m_uiState = 1;
	else m_uiState = 2;

	// get config
	r = m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "config", m_Vars );
	if( r ) {
		reportExecError( r );
		m_uiState = 3;
		updateEnable();
		return;
	}

	updateEnable();

	QStringList lstr = QString( m_pCmdExecutor->getStdout() ).split("\n");
	m_Vars["%DIR%"] =  lstr.at( lstr.indexOf( QRegExp( ".*--directory.*" ) ) );
	m_Vars["%DIR%"].remove( QRegExp("^.*--directory\\s+") ); m_Vars["%DIR%"].remove( QRegExp("\\s+--config.*") );
	m_Vars["%CFG%"] =  lstr.at( lstr.indexOf( QRegExp( ".*--directory.*" ) ) );
	m_Vars["%CFG%"].remove( QRegExp("^.*--config\\s+") ); m_Vars["%CFG%"].remove( QRegExp(".*etc\\\\") );

	ui.m_pAppStatus_label->setText( QString("state: %1, dir: %2, cfg: %3").arg( stateToText( m_uiState ) ).arg( m_Vars["%DIR%"] ).arg( m_Vars["%CFG%"] ) );
}

void ctrl::updateEnable() {

	switch( m_uiState ) {
		case 1:
			ui.m_pChange_button->setEnabled( true );
			if( !ui.m_pChange_button->isChecked() ) ui.m_pStart_button->setEnabled( true );
			ui.m_pStop_button->setEnabled( false );
			ui.m_pRestart_button->setEnabled( false );
		break;
		case 2:
			ui.m_pChange_button->setEnabled( true );
			ui.m_pStart_button->setEnabled( false );
			if( !ui.m_pChange_button->isChecked() ) ui.m_pStop_button->setEnabled( true );
			if( !ui.m_pChange_button->isChecked() ) ui.m_pRestart_button->setEnabled( true );
		break;
		case 3:
			ui.m_pStart_button->setEnabled( false );
			ui.m_pStop_button->setEnabled( false );
			ui.m_pRestart_button->setEnabled( false );
		break;
	}
}

void ctrl::on_m_pStart_button_clicked()
{
	ui.m_pStatus_label->setText( "" );
	reportExecError( m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "start", m_Vars ) );
}

void ctrl::on_m_pStop_button_clicked()
{
	ui.m_pStatus_label->setText( "" );
	reportExecError( m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "stop", m_Vars ) );
}

void ctrl::on_m_pRestart_button_clicked()
{
	ui.m_pStatus_label->setText( "" );
	on_m_pStop_button_clicked();
	ui.m_pStatus_label->setText( "" );
	on_m_pStart_button_clicked();
}

void ctrl::reportExecError( int err ) {

	QStringList lstr;
	switch( err ) {
	case 1: {
		ui.m_pStatus_label->setText( QString("Unknown command: %1").arg(m_pCmdExecutor->last()) );
	}
	break;
	case 2: {
		ui.m_pStatus_label->setText( QString("Failed to start command: %1").arg(m_pCmdExecutor->last()) );
	}
	break;
	case 3: {
		ui.m_pStatus_label->setText( QString("Command not finished: %1").arg(m_pCmdExecutor->last()) );
	}
	break;		
	case 4: {
		ui.m_pStatus_label->setText	( 
			QString("Command: %1 finished with errors:\n%2")
				.arg(m_pCmdExecutor->last())
				.arg(showMessage(QString( m_pCmdExecutor->getStdout() ).split("\n")))
									);
	}
	break;		
	}
}

QString ctrl::showMessage( const QStringList &lstr ) {

	QString sRes = QString::null;
	foreach( QString str, lstr )
		if( !str.isEmpty() ) sRes += str;

	return sRes;
}

void ctrl::on_m_pChange_button_toggled( bool state )
{
	ui.m_pVersion_combo->setEnabled( state );
	ui.m_pFile_combo->setEnabled( state );
	ui.m_pApply_button->setEnabled( state );

	ui.m_pApp_combo->setEnabled( !state );
	ui.groupBox->setEnabled( !state );

	if( state ) {

		ui.m_pVersion_combo->clear();

		QDir dir = QDir::root();
		
		dir.cd( m_Vars["%DIR%"] + "\\etc"  );

		ui.m_pVersion_combo->addItems( dir.entryList( QDir::Dirs ).filter( QRegExp("\\d+\\.\\d+\\.\\d+") ) );

		dir.cd( ui.m_pVersion_combo->itemText( ui.m_pVersion_combo->currentIndex() ) );

		ui.m_pFile_combo->addItems( dir.entryList( QStringList(QString("*.xml")) , QDir::Files ) );
	}
}

void ctrl::on_m_pVersion_combo_currentIndexChanged(int)
{
	ui.m_pFile_combo->clear();

	QDir dir = QDir::root();
		
	dir.cd( m_Vars["%DIR%"] + "\\etc"  );

	dir.cd( ui.m_pVersion_combo->itemText( ui.m_pVersion_combo->currentIndex() ) );

	ui.m_pFile_combo->addItems( dir.entryList( QStringList(QString("*.xml")) , QDir::Files ) );
}

void ctrl::on_m_pApp_combo_currentIndexChanged(int)
{
	ui.m_pAppStatus_label->setText( "" );
	ui.m_pStatus_label->setText( "" );

	if( m_mEnv[ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() )].contains("%DIR%") )
		m_Vars["%DIR%"] = m_mEnv[ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() )]["%DIR%"];

	update_state_slot();
}

#include <QtTest>

void ctrl::on_m_pApply_button_clicked()
{
	ui.m_pStatus_label->setText( "" );

	m_pTimer->stop();

	int r;
	// stop app
	if( m_uiState == 2 ) {
		r = m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "stop", m_Vars );
		if( r ) { reportExecError( r ); m_pTimer->start( 2000 ); return; }
	}

	// delete app
	r = m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "delete", m_Vars );
	if( r ) { reportExecError( r ); m_pTimer->start( 2000 ); return; }

	// create app
	m_Vars["%CFG%"] = ui.m_pVersion_combo->itemText( ui.m_pVersion_combo->currentIndex() ) + "\\" + ui.m_pFile_combo->itemText( ui.m_pFile_combo->currentIndex() );
	r = m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "create", m_Vars );
	if( r ) { reportExecError( r ); m_pTimer->start( 2000 ); return; }

	QTest::qSleep( 1000 );

	// start app
	r = m_pCmdExecutor->exec( ui.m_pApp_combo->itemText( ui.m_pApp_combo->currentIndex() ), "start", m_Vars );
	if( r ) { reportExecError( r ); m_pTimer->start( 2000 ); return; }

	m_pTimer->start( 2000 );

	ui.m_pChange_button->toggle();
}


void ctrl::on_m_pClose_button_clicked()
{
	QApplication::quit();
}