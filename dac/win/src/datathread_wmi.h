//<codepage = utf8
/*! 
	\file datathread_wmi.h
    \brief Описание класса CWMIThread
	\author Лепехин Игорь Юрьевич
	\date 2008
	\warning Для использования класса необходимо наличие библиотеки Qt версии 4 (оригинал тестирован для qt 4.3.1)
	\warning Платформная зависимость - Win32
*/

#ifndef __WMITHREAD_H__
#define __WMITHREAD_H__

/*! файлы библиотеки Qt */
#include <QThread>
#include <QMap>
#include <QStringList>
#include <QDateTime>

/*! файлы проекта */
#include "parameter_enum.h"
#include "parameter_info.h"

/*! предобъявление классов проекта */
class XMLConfig;
class CParameter; 

/*! объявления для WMI */
#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")

class CWMIThread : public QThread {
private:
	class CWMIQuery : public IWbemObjectSink
	{
		LONG m_lRef;
		bool bDone;
		CRITICAL_SECTION threadLock; // for thread safety

	public:
		CWMIQuery(CParameter *, QMap<ulong, QStringList> *, QMap<ulong, bool> *);
		~CWMIQuery();

		virtual ULONG STDMETHODCALLTYPE AddRef();
		
		virtual ULONG STDMETHODCALLTYPE Release();        
		
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void** ppv );

		virtual HRESULT STDMETHODCALLTYPE Indicate( LONG lObjectCount, IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray );
	        
		virtual HRESULT STDMETHODCALLTYPE SetStatus( 
				/* [in] */ LONG lFlags,
				/* [in] */ HRESULT hResult,
				/* [in] */ BSTR strParam,
				/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
				);

		bool IsDone();
	private:
		int		valueFromVariant(VARIANT *pVARIANTVar, QString &sStringVar);
		void	toErrorMap(const QString &);
	private:
		/*! \brief  */
		CParameter					* m_pParameter;
		/*! \brief  */
		QMap<ulong, QStringList>	* m_ptrmapErrors;
		/*! \brief  */
		QMap<ulong, bool>			* m_ptrmapQueriesStatus;
		/*! \brief  */
		int							  m_uiCounter;
	};
public:
	/*! \brief Конструктор
		\param pXMLConfig конфигурация программы
		\param sType тип нити
		\param uiUpdate такт нити
	*/
	CWMIThread( XMLConfig *pXMLConfig, ePP_QueryType eType, uint uiUpdate, uint uiMUpdate );
	/*! \brief Деструктор */
	~CWMIThread();
	//! Перегруженная функция QThread - главная функция нити
	void run();
	//! "Мягкая остановка нити"
	void stop() { m_bStop = true; }
	//! устанавливает указатель на глобальный "массив" данных
	static void setPtrToParameters(QMap<ulong, CParameter*> &ptrmapParameters)  { m_ptrmapParameters = &ptrmapParameters; }
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
	/*! \brief инициализация и подключение к WMI
	*/
	bool WMI_init();
	/*! \brief WMI запрос
		\param pPar параметр для запроса
		\param mapErrors хеш ошибок во время запросов
		\param mapQueryesStatus хеш состояния запросов
	*/
	bool WMI_query(CParameter *pPar, QMap<ulong, QStringList> &mapErrors, QMap<ulong, bool> &mapQueryesStatus);
	/*! \brief ожидание выполнения запросов к WMI
		\param mapQueryesStatus хеш состояния запросов
	*/
	bool WMI_wait(const QMap<ulong, bool> &mapQueryesStatus) const;
	/*! \brief проверка ошибок
		\param mapErrors хеш ошибок во время запросов
	*/
	void WMI_check(const QMap<ulong, QStringList> &mapErrors) const;
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
	/*! \brief уровень отладки */
	static int m_iDebugLevel;
	/*! \brief указатель на объект QObject (приемник отладочных сообщений) */
	static QObject *m_pParent;
	/*! \brief уникальный идентификатор нити */
	static unsigned int m_uiId;
	/*! \brief указатель на глобальный "массив" данных (объектов CParameter*) */
	static QMap<ulong, CParameter*> *m_ptrmapParameters;
	/*! \brief указатель на объект локатора WMI */
	IWbemLocator *m_pLoc;
	/*! \brief указатель на объект сервиса WMI */
	IWbemServices *m_pSvc;
};
#endif
