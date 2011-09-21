//<codepage = utf8
#ifndef __CEVENTS_H__
#define __CEVENTS_H__

#include <QEvent>
#include <QDateTime>
#include <QStringList>

class CMsg : public QEvent {
public:

	enum EMsgType { eMT_Error = 0, eMT_Warning = 1, eMT_Success = 3, eMT_Info1 = 4, eMT_Info2 = 5, eMT_Info3 = 6, eMT_Info4 = 7, eMT_Info5 = 8 }; 

	CMsg(unsigned int, const QDateTime &, const QStringList &, int = -1);
	virtual ~CMsg();

	inline QStringList  msg()  { return m_lMessages;  }
	inline QDateTime    date() { return m_oDateTime;  }
	inline unsigned int id()   { return m_uiThreadId; }
	inline int          type() { return m_iMsgType;   }

private:
	unsigned int m_uiThreadId;
	QDateTime m_oDateTime;
	QStringList m_lMessages;
	int m_iMsgType;
};

#endif


