//<codepage = utf8
#include "cevent.h"

//
CMsg::CMsg(unsigned int uiThreadId, const QDateTime &oDateTime, const QStringList &sList, int iType) :
	QEvent(QEvent::User), m_uiThreadId(uiThreadId), m_oDateTime(oDateTime), m_lMessages(sList), m_iMsgType(iType) {

}

//
CMsg::~CMsg() {
}


