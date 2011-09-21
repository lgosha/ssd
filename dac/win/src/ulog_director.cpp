#include "ulog_director.h"
#include "cevent.h"

#include <qdebug.h>

ULogDirector::ULogDirector() {
}

ULogDirector::~ULogDirector() {

	foreach(ULog *pLog, m_mLogs.values())
		delete pLog;
}

ULog::EMsgType ULogDirector::CMsgTypeToULogType( int iMsgType ) {

	switch(iMsgType) {
		case   CMsg::eMT_Error:
		return ULog::eMT_Error;
		case   CMsg::eMT_Warning:
		return ULog::eMT_Warning;
		case   CMsg::eMT_Success:
		return ULog::eMT_Success;
		case   CMsg::eMT_Info1:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info2:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info3:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info4:
		return ULog::eMT_Info;
		case   CMsg::eMT_Info5:
		return ULog::eMT_Info;
	}

	return ULog::eMT_Info;
}

void ULogDirector::customEvent( QEvent *pEvent ) {

	CMsg *pMsg = reinterpret_cast<CMsg *>(pEvent);
	if( pMsg && m_mLogs[pMsg->id()] )
		m_mLogs[pMsg->id()]->addD(QStringList(pMsg->msg()), pMsg->date(), CMsgTypeToULogType(pMsg->type()));
}

void ULogDirector::add( ushort uiId, const QString &sFile, const QString &sCodepage, QListWidget *pViewWidget ) {

	m_mLogs[uiId] = new ULog( sFile, sCodepage, pViewWidget, true );
}
