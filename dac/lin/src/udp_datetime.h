//<codepage = utf8
#ifndef DATETIMESPECIALDEFENITION
#define DATETIMESPECIALDEFENITION

#include <QDateTime>

class DateTime {
    
public:
	
	static long int timetoint(QTime t) {
	  long int ti = t.msec();
	  ti += 1000 * t.second();
	  ti += 60000 * t.minute();
	  ti += 3600000 * t.hour();
	  return ti;
	}

	static QTime inttotime(long int it) {
	  QTime ti;
	  int ih;
	  int im;
	  int is;
	  int ims = it % 1000;
	  it -= ims;
	  it /= 1000;
	  is = it % 60;
	  it -= is;
	  it /= 60;
	  im = it % 60;
	  it -= im;
	  it /= 60;
	  ih = it;
	  if(ti.setHMS(ih, im, is, ims))
		return ti;
	  ti.setHMS(0, 0, 0, 0);
	  return ti;
	}

	static long int datetoint(QDate d) {
	  long int ti = d.day();
	  ti += 40 * d.month();
	  ti += 800 * d.year();
	  return ti;
	}

	static QDate inttodate(long int it) {
	  QDate ti;
	  int iy;
	  int im;
	  int id = it % 40;
	  it -= id;
	  it /= 40;
	  im = it % 20;
	  it -= im;
	  it /= 20;
	  iy = it;
	  if(ti.setDate(iy, im, id))
		return ti;
	  ti.setDate(0, 0, 0);
	  return ti;
	}

};

#endif


