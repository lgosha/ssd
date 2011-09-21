//<codepage = utf8
/*! 
	\file datathread_snmp.h
	\brief Описание класса CSNMPThread
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt версии 4 (оригинал тестирован для qt 4.3.1)
	\warning Для использования класса необходимо наличие библиотеки netsnmp (оригинал тестирован для netsnmp 5.4.1)
	\warning Платформной зависимости не замечено
*/

#ifndef __SNMPTHREAD_H__
#define __SNMPTHREAD_H__

/*! файлы библиотеки Qt */
#include <QThread>
#include <QMap>
#include <QDateTime>
#include <QStringList>

// файлы проекта
#include "parameter_enum.h"
#include "parameter_info.h"

// файлы STL
#include <map>

using namespace std;

/*! предобъявление классов проекта */
class XMLConfig;
class CParameter; 

/*! \class CSNMPThread
\brief Класс периодического определения значений параметров по протоколу SNMPv2c 

<p>Класс предназначен для периодического запроса у нескольких SNMP-агентов параметров по протоколу SNMPv2c.
Значения параметров запрашиваются у всех SNMP-агентов асинхронно ("одновременно"). В классе предусмотрены отладочные сообщения
различного уровня подробности.
</p>

<p>Алгоритм работы нити аналогичен алгоритму работы абстрактной нити получения данных CAbstractDataThread
</p>

<p>Специфика нити CSNMPThread связана с использованием кросс-платформенного netsnmp api
</p>

\warning Перед использованием нити внимательно прочитайте раздел "Создание объекта класса" - этот раздел специфичен для CSNMPThread
	
<p><b>Создание объекта класса</b></p>
\code
// устанавливается указатель на глобальный "массив" данных
CSNMPThread::setPtrToParameters(map<QString, CParameter*> object);
// устанавливается указатель на объект QObject
CSNMPThread::setParent(pWnd);
// устанавливается идентификатор нити
CSNMPThread::setId("001");
// устанавливается уровень отладки
CSNMPThread::setDebugLevel(8);
// создаем объект нити
QThread *pDataThread = new CSNMPThread(
			mapEtc, // указатель на хеш конфигурации
			1000,	// такт нити
			10000	// максимальное значение счетчика тактов
			);
\endcode

\warning В программе может быть только один объект данного класса

<p><b>Запуск нити</b></p>
\code pDataThread->start();
\endcode

<p><b>"Мягкая" остановка нити</b></p>
\code pDataThread->stop();\endcode

<p><b>"Жесткая" остановка нити</b></p>
\code pDataThread->terminate();
\endcode

<p><b>Уничтожение объекта класса</b></p>
\code delete pDataThread;
\endcode

*/
class CSNMPThread : public QThread {
private:
	/*! \class snmpoid
	\brief Класс для хранения SNMP Object Identifier (OID) во внутреннем представлении библиотеки netsnmp

	<p>Класс предназначения для хранения одного SNMP Object Identifier (OID) во внутреннем представлении библиотеки netsnmp. Возможно
	хранение как внутренних SNMP OID (в переменной intOID), так и уже созданных (копия) для применения к ним методов класса (operator ==() и toPrint()).
	</p>

	<p>Создание внутреннего SNMP OID
	</p>
	\code snmpoid snmpoidobj;
	read_objid(".1.3.6.1.2.1.1.2.0", snmpoidobj.intOID, &snmpoidobj.OIDlen)) // функция netsnmp api
	\endcode

	<p>Создание внешнего SNMP OID
	</p>
	\code snmpoid snmpoidobj(internalOID, internalOID); // массив internalOID должен быть создан и проинициализирован
	\endcode

	\warning Перед использованием методов класса для внутренних переменных необходимо вызывать update_ptr()  
	*/
	class snmpoid {
	public:
		//! конструктор внутренних SNMP OID
		snmpoid() { ptrOID = intOID; }
		//! конструктор внешних SNMP OID
		snmpoid(const unsigned long* SNMPOID, size_t SNMPOIDlen) : ptrOID(SNMPOID), OIDlen(SNMPOIDlen) {};
		/*! \brief Оператор сравнения на равенство
			\warning Для внутренних SNMP OID перед вызовом вызвать update_ptr()
		*/
		friend bool operator == (const snmpoid &s1, const snmpoid &s2) {
			if(s1.OIDlen != s2.OIDlen) return false;
			for(int i=s1.OIDlen-1;i>=0;i--)
				if(s1.ptrOID[i] != s2.ptrOID[i])
					return false;
			return true;
		}
		/*! \brief Перевод SNMP OID в строковое представление
			\warning Для внутренних SNMP OID перед вызовом вызвать update_ptr()
		*/
		QString toPrint() const {
			QString str = "";
			for(size_t i=0;i<OIDlen;i++)
				str += QString(".%1").arg(ptrOID[i]);
			return str;
		}
		//! обновление указателя для внутренних OID
		void update_ptr() { ptrOID = intOID; }
		//! (внутренний) массив представление SNMP OID (ex. .1.3.6.1.2.1.1.3.0 => int array 136121130 of size 9)
		unsigned long  intOID[128]; 
		//! (внешний) массив представления SNMP OID
		const unsigned long *ptrOID;
		//! "реальный" размер массива SNMP OID
		size_t OIDlen;
	};

	/*! \class snmphost
	\brief Класс для описания SNMP-агента 

	Класс для описания SNMP-агента устройства. Предназначен для совместного использования с CSNMPThread. Фактически хранит в одном объекте
	информацию о SNMP-агенте и интересующих нас параметрах этого агента.
	*/
	class snmphost {
	public:
		//! Создает и возвращает указатель на строку, как копию переданной строки
		static char* getCharPointer(const QString &str);
	public:
		//! Пустой конструктор
		snmphost() : pHostSession(0) {};
		//! Пустой деструктор
		~snmphost() {};
		//! Возвращает указатель на snmp сессию с устройством
		struct		snmp_session * session()				{ return pHostSession; }
		//! Устанавливает snmp сессию с устройством
		void		setSession(struct snmp_session *s)			{ pHostSession = s; }
		/*! \brief Добавляет SNMP OID для запроса значения 
			\param Id идентификатор параметра CParameter *
			\param SNMPOID netsnmp
		*/
		void		add(ulong ulId, const snmpoid &SNMPOID)	{ mOIDs[ulId] = SNMPOID; }
		/*! \brief Возвращает указатель на параметр агента, значение которого необходимо обновить на текущем такте
			\param bFirst - флаг запроса первого параметра
		*/
		snmpoid*	get(bool bFirst);
		//! Возвращает идентификатор запрашиваемого параметра
		ulong getCurrentOIDId();
		//! Возвращает идентификатор параметра соответствующего netsnmp_oid
		ulong getOIDId(const snmpoid &netsnmp_oid);
		//! Возвращает список идентификатор параметров данного snmphost для печати
		QStringList getParametersIds() {
			QStringList lstr;
			foreach(ulong ulId, mOIDs.keys())
				lstr << CParameterProperties::toStr(ulId);
			return lstr;
		}
	private:
		//! snmp сессия с устройством
		struct snmp_session *pHostSession;
		//! набор параметров (хеш соответствия внешенго идентификатора параметра внутреннем представлению SNMP OID netsnmp)
		QMap<ulong, snmpoid> mOIDs;	
		//! итератор на запрашиваемый параметр
		QMap<ulong, snmpoid>::iterator p;
	};
public:
	/*! \brief Конструктор
		\param pXMLConfig конфигурация программы
		\param eType тип нити
		\param uiUpdate такт нити
		\param uiMUpdate максимальное значение счетчика тактов
	*/
	CSNMPThread(XMLConfig *pXMLConfig, ePP_QueryType eType, unsigned int uiUpdate, uint uiMUpdate);
	/*! \brief Деструктор */
	~CSNMPThread();
	//! Перегруженная функция QThread - главная функция нити
	void run();
	//! "Мягкая остановка нити"
	void stop() { m_bStop = true; }
	//! запуск сокетов
	static void SOCK_START();
	//! остановка сокетов
	static void SOCK_STOP();
	//! устанавливает указатель на глобальный "массив" данных
	static void setPtrToParameters(QMap<ulong, CParameter*> &ptrmapParameters) { m_ptrmapParameters = &ptrmapParameters; }
	//! устанавливает указатель на объект QObject
	static void setParent(QObject *pParent)										{ m_pParent = pParent; }
	//! устанавливает идентификатор нити
	static void setId(const QString &sId)										{ m_uiId = sId.right(sId.length() - 1).toInt(); }	
	//! устанавливает уровень отладки
	static void setDebugLevel(int iDebugLevel)									{ m_iDebugLevel = iDebugLevel; }
private:
	/*! \brief Посылка отладочного сообщения
		\param lstr cообщение
		\param date дата и метка времени сообщения
		\param type тип сообщения
		\sa ULog для формирования однострочного или многострочного сообщения
		\sa CMsg для установки типа сообщения
	*/
	static void sendMessage(const QStringList &lstr, const QDateTime &date = QDateTime::currentDateTime(), int type = 1);
	/*! \brief Callback функция netsnmp

		Callback функция netsnmp. Вызывается каждый раз, при получении ответа от SNMP-агента
		\param type тип операции, вызвавшей функцию (принято сообщение от агента, таймаут)
		\param session указатель на сессию с SNMP-агентом
		\param reqid не знаю
		\param responce указатель на структуру, содержащую ответ агента
		\param magic пустой указатель - средство передачи информации в эту функцию
	*/
	static int snmp_asynch_response(int type, struct snmp_session *session, int reqid, struct snmp_pdu *responce, void *magic);
	/*! \brief Посылает запрос SNMP-агенту
		\param pHost описание SNMP-агента
		\param bFirst флаг запроса первого параметра
	*/
	static int snmp_sendto(snmphost *pHost, bool bFirst = false);
	//! \brief Обеспечивает ожидание запроса всех параметров от всех SNMP-агентов, или выхода по таймауту
	static int snmp_select();
	/*! \brief Обработка ответа SNMP-агента
		\param status состояние ответа (success, timeout, error)
		\param session указатель на сессию с SNMP-агентом
		\param responce указатель на структуру, содержащую ответ агента
		\param pHost описание SNMP-агента
	*/
	static int snmp_parse_recv(int status, struct snmp_session *session, struct snmp_pdu *responce, snmphost *pHost);
	/*!
	*/
	static void apply_parameter( CParameter *, const QDateTime & );
private:
	/*! \brief флаг "мягкой" остановки нити */
	bool m_bStop;
	/*! \brief тип нити */
	ePP_QueryType m_eType;
	/*! \brief временной такт нити */
	unsigned int m_uiUpdate;
	/*! \brief максимальное значение счетчика тактов */
	unsigned int m_uiMaxUpdate;
	/*! \brief SNMP-агенты */
	QMap<QString, snmphost*> qmapSNMPHosts;
	/*! \brief уровень отладки */
	static int m_iDebugLevel;
	/*! \brief указатель на объект QObject (приемник отладочных сообщений) */
	static QObject *m_pParent;
	/*! \brief уникальный идентификатор нити */
	static unsigned int m_uiId;
	/*! \brief указатель на глобальный "массив" данных (объектов CParameter*) */
	static QMap<ulong, CParameter*> *m_ptrmapParameters;
	/*! \brief количество активных SNMP-агентов */
	static unsigned int m_uiActiveHosts;
};

#endif
