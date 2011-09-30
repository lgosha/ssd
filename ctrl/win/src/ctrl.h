#ifndef CTRL_H
#define CTRL_H

#include <QtGui/QMainWindow>
#include "ui_ctrl.h"

#include "xmlconfig.h"

class QProcess;

class CCommand {
private:
		typedef struct tagArgv {

			tagArgv() : sCmd( QString::null ) {};

			tagArgv( const tagArgv & _other ) {
				sCmd = _other.sCmd;
				sArgs = _other.sArgs;
				sIgnore = _other.sIgnore;
			}

			tagArgv & operator = ( const tagArgv & _other ) {

				sCmd = _other.sCmd;
				sArgs = _other.sArgs;
				sIgnore = _other.sIgnore;
				
				return *this;
			}

			QString sCmd;
			QStringList sArgs;
			QStringList sIgnore;

		} _Argv;
public:
		CCommand( XMLConfig *, QObject * );
		~CCommand();
		int exec( const QString & , const QString &, const QMap<QString, QString> &vars = QMap<QString, QString>(), bool = false );
		const QString & last() { return m_sLastCommand; }
		inline QByteArray getStdout() { return m_baOutput; }
private:
		void update( const QString & , const QString &, const QMap<QString, QString> & );
		void restore( const QString & , const QString & );
private:
		QProcess * m_pProcess;
		QMap<QString, QMap<QString, _Argv> > m_mCommands;
		QString m_sLastCommand;
		_Argv m_toRestore;
		QByteArray m_baOutput;
};

class ctrl : public QMainWindow
{
	Q_OBJECT

public:
	ctrl(XMLConfig *, QWidget *parent = 0, Qt::WFlags flags = 0);
	~ctrl();

private:
	Ui::ctrlClass ui;

	static QString stateToText( uint );

	uint m_uiState;

	QMap<QString, QString> m_Vars;

	QTimer * m_pTimer;

	CCommand * m_pCmdExecutor;

	QMap<QString, QMap<QString, QString> > m_mEnv;

private:

	void reportExecError( int );
	void updateEnable( );

private slots:
	void on_m_pClose_button_clicked();
	void on_m_pVersion_combo_currentIndexChanged(int);
	void on_m_pApply_button_clicked();
	void on_m_pApp_combo_currentIndexChanged(int);
	void on_m_pRestart_button_clicked();
	void on_m_pStop_button_clicked();
	void on_m_pStart_button_clicked();
	void on_m_pChange_button_toggled(bool);

	void update_state_slot();
};

#endif // CTRL_H
