//<codepage = utf8
#include "ulog.h"
#include "exception.h"

//#include <QListWidget>
#include <QTextCodec>

const QString c_sDateTimeFormat = "dd.MM.yyyy hh:mm:ss.zzz";

ULog::ULog() : m_pListWidget(0) {
}

ULog::ULog(const QString &sFileName, const QString &sCodePage, QListWidget *pListWidget, bool bToFile) : 
	m_pListWidget(pListWidget), m_bToFile(bToFile)
{
	m_pCodec =  QTextCodec::codecForName(sCodePage.toAscii());

	if(open(sFileName))
		throw ex_base(ex_base::warning, "can't open log file: " + sFileName, "ULog::ULog");
}

ULog::~ULog()
{
	if(m_File.isOpen()) m_File.close();
}

int ULog::open(const QString &fname)
{
	m_File.setFileName(fname);
	if(!m_File.open(QIODevice::WriteOnly))
		return 1;

	m_FileStream.setDevice(&m_File);

	return 0;
}

//QColor ULog::getColorByMsgType(EMsgType type)
//{
//	switch(type) {
//
//		case eMT_Success: return QColor(000, 150, 000); break;
//		case eMT_Warning: return QColor(150, 150, 000); break;
//		case eMT_Error:   return QColor(255, 000, 000);	break;
//	}
//
//	return QColor(000, 000, 000);
//}

QString ULog::getSymbolByMsgType(EMsgType type)
{
	switch(type) {

		case eMT_Success: return "+"; break;
		case eMT_Warning: return "!"; break;
		case eMT_Error:   return "-"; break;
	}

	return "*";
}

QString ULog::dateTimeToString(const QDateTime &date, bool bEmpty)
{
	if(!bEmpty)
		return date.toString(c_sDateTimeFormat);

	QString str = "";
	for(QStringList::size_type i=0;i<c_sDateTimeFormat.length();i++)
		str += " ";

	return str;
}

int ULog::add(const QStringList& lstr, EMsgType type, const QDateTime &date)
{
	// prepare messages
	QStringList lMsg;
	for(QStringList::size_type i=0;i<lstr.size();i++)
		lMsg.push_back(QString("| %1 | %2 | %3").arg(dateTimeToString(date, i)).arg(getSymbolByMsgType(type)).arg(lstr[i]));

	// to file
	if(m_bToFile) {
		for(QStringList::size_type i=0;i<lMsg.size();i++)
			m_FileStream << m_pCodec->toUnicode(lMsg[i].toAscii()) << endl;
		m_File.flush();
	}

	// to listview
	//if(m_pListWidget) {
	//	for(QStringList::size_type i=lMsg.size();i>0;i--) {
	//		m_pListWidget->insertItem(0, m_pCodec->toUnicode(lMsg[i-1].toAscii()));
	//		QListWidgetItem *pItem = m_pListWidget->item(0);
	//		pItem->setForeground(getColorByMsgType(type));
	//	}
	//}
	
	return 0;
}

int ULog::add(const QString& str, EMsgType type, const QDateTime &date) {

	return add(QStringList(str), type, date);
}

int ULog::addD(const QStringList& lstr, const QDateTime &date, EMsgType type) {

	add(lstr, type, date);

	return 0;
}

