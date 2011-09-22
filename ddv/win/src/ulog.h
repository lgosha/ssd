//<codepage = utf8
/*! 
	\file ulog.h
    \brief Описание класса ULog
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt версии 4 (протестировано для qt 4.3.1)
	\warning Платформной зависимости не замечено
*/

/*! не помню зачем это нужно :-) */
#ifndef __USERLOG_H__
#define __USERLOG_H__

/*! файлы библиотеки Qt */
#include <QDateTime>
#include <QStringList> 
#include <QFile>
#include <QTextStream>
//#include <QColor>

/*! предобъявление классов библиотеки Qt */
class QListWidget;
class QTextCodec;

/*! \class ULog
\brief Класс сохранения и отображения отладочных сообщений 

<p>Класс предназначен для сохранения и отображения отладочных сообщений. Сохранение сообщений производится в текстовый файл.
Отображение сообщений в объект класса Qt4 QListWidget. Сообщения могут быть однострочными и многострочными. Порядок отображения сторок
в файле и на экране различен.
</p>
	
<p><b>Создание объекта класса</b></p>
\code ULog pLog = new ULog("filename.txt", pListWidget);
\endcode
	
<p><b>Добавление однострочных сообщений</b></p>
\code pLog->add("message 1", date, EMsgType::eMT_Info);
 pLog->add("message 2", date, EMsgType::eMT_Infog);
\endcode
	
<p>На экране</p>
<pre> | 01.01.2008 22:12:04.154 | * | message 2
 | 01.01.2008 22:12:04.154 | * | message 1
</pre>

	<p>В файле</p> 
<pre> | 01.01.2008 22:12:04.154 | * | message 1
 | 01.01.2008 22:12:04.154 | * | message 2
</pre>

<p><b>Добавление многострочных сообщений</b></p>
\code QStringList lstr;
 // добавление первого сообщения
 lstr << "message header 1" << "message 1" << "message 2";
 pLog->add(lstr, date, EMsgType::eMT_Info);
 // очищаем список
 lstr.clear();
 // добавление второго сообщения
 lstr << "message header 2" << "message 1" << "message 2";
 pLog->add(lstr, date, EMsgType::eMT_Info);
\endcode

<p>На экране</p>
<pre> | 01.01.2008 22:12:04.154 | * | message header 2
 |			  | * | message 1
 |			  | * | message 2
 | 01.01.2008 22:12:04.154 | * | message header 1
 |			  | * | message 1
 |			  | * | message 2
</pre>

<p>В файле</p>
<pre> | 01.01.2008 22:12:04.154 | * | message header 1
 |			  | * | message 1
 |			  | * | message 2
 | 01.01.2008 22:12:04.154 | * | message header 2
 |			  | * | message 1
 |			  | * | message 2
</pre>

<p><b>Удаление объекта</b></p>
\code delete pLog;\endcode

*/
class ULog {
public:
	/*! \brief тип сообщения
		\details Тип отладочного сообщения
	*/
	enum EMsgType
	{ 
		/*!	информационное сообщение */
		eMT_Info,
		/*!	сообщение об успешном выполнении */
		eMT_Success,
		/*!	сообщение о предупреждении */
		eMT_Warning,
		/*!	сообщение об ошибке */
		eMT_Error 
	}; 

	/*! \brief конструктор по умолчанию
		\details Конструктор по умолчанию необходим для работы с контейнерами
	*/
	ULog();
	/*! \brief конструктор класса
	*/
	explicit ULog(const QString &, const QString &, QListWidget * = 0, bool bToFile = true);
	/*! \brief деструктор класса
	*/
	~ULog();
	/*! \brief добавление сообщения с текущей датой
		\details Добавление отладочного сообщения с текущей датой
	*/
	int add(
				const QStringList &lstr, // сообщениe 
				EMsgType type = eMT_Info, // тип (EMsgType::eMT_Info by default)
				const QDateTime &date = QDateTime::currentDateTime() // дата и метка времени
			);
	int add(
				const QString &str, // сообщениe 
				EMsgType type = eMT_Info, // тип (EMsgType::eMT_Info by default)
				const QDateTime &date = QDateTime::currentDateTime() // дата и метка времени
			);
	/*! \brief добавление сообщения с пользовательской датой
		\details Добавление отладочного сообщения с пользовательской датой (например необходимо для передачи сообщений из
		нитей для того, чтобы сообщение имело дату и метку времени нити, а не дату и метку времени получения сообщения главным окном)
	*/
	int addD(
				const QStringList &lstr, // сообщениe 
				const QDateTime &date = QDateTime::currentDateTime(), // дата и метка времени
				EMsgType type = eMT_Info // тип (EMsgType::eMT_Info by default)
			);
private:
	/*! \brief открывает файл для записи сообщений
	*/
	int open(const QString &fname);
	/*! \brief возвращает цвет сообщения
	*/
//	static QColor getColorByMsgType(EMsgType);
	/*! \brief возвращает символ сообщения
	*/
	static QString getSymbolByMsgType(EMsgType);
	/*! \brief переводит дату и метку времени в строковый формат
	*/
	static QString dateTimeToString(const QDateTime &, bool);
private:
	/*! \brief файл
	*/
	QFile m_File;
	/*! \brief файловый поток
	*/
	QTextStream m_FileStream;
	/*! \brief флаг записи в файл
	*/
	bool m_bToFile;
	/*! \brief указатель на объект QListWidget
	*/
	QListWidget *m_pListWidget;
	/*! \brief указатель на объект QTextCode - преобразование кодировок
	*/
	QTextCodec *m_pCodec;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

