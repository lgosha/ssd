//<codepage = utf8
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>
#include <string>
#include <sstream> 

#include <QString>
#include <QStringList>

class ex_base : public std::exception {
public:

	enum { ignore = 0, warning = 1, error = 2 };
	
	ex_base(const int) throw();
	ex_base(const int, const QString &) throw();
	ex_base(const int, const QString &, const QString &) throw();
	ex_base(const int, const QString &, const QString &, const QStringList &) throw();

	virtual ~ex_base() throw() {};

	virtual QString     get_what()  const throw() { return _what; }
	virtual QString     get_where() const throw() { return _where; }
	virtual QStringList getErrors() const throw() { return _errors; }

	void addError(const QString &str) { _errors.push_back(str); }
	
	int status() const { return _status; } 

private:	
	QString _what;		
	QString _where;		

	QStringList _errors;

	int _status;
};

class ex_file_parse : public ex_base {
private:
	int m_iLineNumber;
public:
	ex_file_parse(const int, const int, const QString &, const QString & = "") throw();
	virtual QString get_what() const throw();
};


#endif

