//<codepage = utf8
#include "exception.h"

#include <QObject>

#include <sstream>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------
// class ex_base implementation

// default base class constructor (no message & ignore status)
ex_base::ex_base(const int status_arg) throw() : _status(status_arg) {

	_status = status_arg;
	_what  = "";
	_where = "";
}

// base class constructor for global exceptions
ex_base::ex_base(const int status_arg, const QString & what_arg) throw() : _what(what_arg) {

	_status = status_arg;
	_where = "";
}

// constructor
ex_base::ex_base(const int status_arg, const QString & what_arg, const QString & where_arg) throw()
		: _what(what_arg), _where(where_arg) {

	_status = status_arg;
}

// full constructor
ex_base::ex_base(const int status_arg, const QString & what_arg, const QString & where_arg, const QStringList &errors_arg) throw()
		: _what(what_arg), _where(where_arg), _errors(errors_arg) {

	_status = status_arg;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------
// class ex_file_parse implementation

// full constructor
ex_file_parse::ex_file_parse(int line_arg, int status_arg, const QString & what_arg, const QString & where_arg) throw() 
	: ex_base(status_arg, what_arg, where_arg) {

	m_iLineNumber = line_arg;
}

// get error reason
QString ex_file_parse::get_what() const throw() {
	
	QString str;
	str = ex_base::get_what() + QObject::tr(" in line ") + QString::number(m_iLineNumber);

	return str;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

