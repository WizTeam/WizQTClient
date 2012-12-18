/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "DateTools.h"
#include "CLucene/util/Misc.h"

CL_NS_USE(util)
CL_NS_DEF(document)

TCHAR* DateTools::timeToString(const int64_t time, Resolution resolution /*= MILLISECOND_FORMAT*/) {
	TCHAR* buf = _CL_NEWARRAY(TCHAR, DATETOOLS_BUFFER_SIZE);
	timeToString(time, resolution, buf, DATETOOLS_BUFFER_SIZE);
	return buf;
}

void DateTools::timeToString(const int64_t time, Resolution resolution, TCHAR* buf, size_t bufLength)
{
    // Take into account TZ and DST differences which may appear when using gmtime below
    const int64_t diff_secs = getDifferenceFromGMT();
	time_t secs = time / 1000 + diff_secs;
	tm *ptm = gmtime(&secs);

	char abuf[DATETOOLS_BUFFER_SIZE];

	if (resolution == MILLISECOND_FORMAT) {
		size_t len = strftime(abuf, DATETOOLS_BUFFER_SIZE, "%Y%m%d%H%M%S", ptm);
		uint32_t ms = static_cast<uint32_t>(time % 1000);
		_snprintf(abuf + len, 4, "%03u", ms);
	} else if (resolution == SECOND_FORMAT) {
		strftime(abuf, DATETOOLS_BUFFER_SIZE, "%Y%m%d%H%M%S", ptm);
	} else if (resolution == MINUTE_FORMAT) {
		strftime(abuf, DATETOOLS_BUFFER_SIZE, "%Y%m%d%H%M", ptm);
	} else if (resolution == YEAR_FORMAT) {
		strftime(abuf, DATETOOLS_BUFFER_SIZE, "%Y", ptm);
	} else if (resolution == MONTH_FORMAT) {
		strftime(abuf, DATETOOLS_BUFFER_SIZE, "%Y%m", ptm);
	} else if (resolution == DAY_FORMAT) {
		strftime(abuf, DATETOOLS_BUFFER_SIZE, "%Y%m%d", ptm);
	} else if (resolution == HOUR_FORMAT) {
		strftime(abuf, DATETOOLS_BUFFER_SIZE, "%Y%m%d%H", ptm);
	}

	STRCPY_AtoT(buf,abuf, bufLength);
}

tm* DateTools::stringToDate(const TCHAR* dateString){
    const int64_t time = stringToTime(dateString);
    time_t secs = time / 1000;
	tm *ptm = gmtime(&secs);
    return ptm;
}

int64_t DateTools::stringToTime(const TCHAR* dateString) {
  tm s_time;
  memset(&s_time, 0, sizeof (s_time));
  s_time.tm_mday = 1;
  int32_t ms = 0;

  switch (_tcslen(dateString)) {
  case 4: // YEAR_FORMAT
  {
    s_time.tm_year = _ttoi(dateString) - 1900;
    if (s_time.tm_year == -1900)
      _CLTHROWA(CL_ERR_Parse, "Input is not valid date string");
    break;
  }
  case 6: // MONTH_FORMAT
  {
    TCHAR* tmpDate = STRDUP_TtoT(dateString);
    s_time.tm_mon = _ttoi(&tmpDate[4]) - 1;
    tmpDate[4] = 0;
    s_time.tm_year = _ttoi(tmpDate) - 1900;
    _CLDELETE_CARRAY(tmpDate);
    break;
  }
  case 8: // DAY_FORMAT
  {
    TCHAR* tmpDate = STRDUP_TtoT(dateString);
    s_time.tm_mday = _ttoi(&tmpDate[6]);
    tmpDate[6] = 0;
    s_time.tm_mon = _ttoi(&tmpDate[4]) - 1;
    tmpDate[4] = 0;
    s_time.tm_year = _ttoi(tmpDate) - 1900;
    _CLDELETE_CARRAY(tmpDate);
    break;
  }
  case 10: // HOUR_FORMAT
  {
    TCHAR* tmpDate = STRDUP_TtoT(dateString);
    s_time.tm_hour = _ttoi(&tmpDate[8]);
    tmpDate[8] = 0;
    s_time.tm_mday = _ttoi(&tmpDate[6]);
    tmpDate[6] = 0;
    s_time.tm_mon = _ttoi(&tmpDate[4]) - 1;
    tmpDate[4] = 0;
    s_time.tm_year = _ttoi(tmpDate) - 1900;
    _CLDELETE_CARRAY(tmpDate);
    break;
  }
  case 12: // MINUTE_FORMAT
  {
    TCHAR* tmpDate = STRDUP_TtoT(dateString);
    s_time.tm_min = _ttoi(&tmpDate[10]);
    tmpDate[10] = 0;
    s_time.tm_hour = _ttoi(&tmpDate[8]);
    tmpDate[8] = 0;
    s_time.tm_mday = _ttoi(&tmpDate[6]);
    tmpDate[6] = 0;
    s_time.tm_mon = _ttoi(&tmpDate[4]) - 1;
    tmpDate[4] = 0;
    s_time.tm_year = _ttoi(tmpDate) - 1900;
    _CLDELETE_CARRAY(tmpDate);
    break;
  }
  case 14: // SECOND_FORMAT
  {
    TCHAR* tmpDate = STRDUP_TtoT(dateString);
    s_time.tm_sec = _ttoi(&tmpDate[12]);
    tmpDate[12] = 0;
    s_time.tm_min = _ttoi(&tmpDate[10]);
    tmpDate[10] = 0;
    s_time.tm_hour = _ttoi(&tmpDate[8]);
    tmpDate[8] = 0;
    s_time.tm_mday = _ttoi(&tmpDate[6]);
    tmpDate[6] = 0;
    s_time.tm_mon = _ttoi(&tmpDate[4]) - 1;
    tmpDate[4] = 0;
    s_time.tm_year = _ttoi(tmpDate) - 1900;
    _CLDELETE_CARRAY(tmpDate);
    break;
  }
  case 17: // MILLISECOND_FORMAT
  {
    TCHAR* tmpDate = STRDUP_TtoT(dateString);
    ms = _ttoi(&tmpDate[14]);
    tmpDate[14] = 0;
    s_time.tm_sec = _ttoi(&tmpDate[12]);
    tmpDate[12] = 0;
    s_time.tm_min = _ttoi(&tmpDate[10]);
    tmpDate[10] = 0;
    s_time.tm_hour = _ttoi(&tmpDate[8]);
    tmpDate[8] = 0;
    s_time.tm_mday = _ttoi(&tmpDate[6]);
    tmpDate[6] = 0;
    s_time.tm_mon = _ttoi(&tmpDate[4]) - 1;
    tmpDate[4] = 0;
    s_time.tm_year = _ttoi(tmpDate) - 1900;
    _CLDELETE_CARRAY(tmpDate);
    break;
  }
  default:
  {
    _CLTHROWA(CL_ERR_Parse, "Input is not valid date string");
    break;
  }
  }

  time_t t = mktime(&s_time);
  if (t == -1)
    _CLTHROWA(CL_ERR_Parse, "Input is not valid date string");

  // Get TZ difference in seconds, and calc it in
  const int64_t diff_secs = getDifferenceFromGMT();

  return (static_cast<int64_t>(t + diff_secs) * 1000) + ms;
}

int64_t DateTools::getDifferenceFromGMT()
{
    struct tm *tptr;
    time_t secs, local_secs, gmt_secs;
    time( &secs );  // Current time in GMT
    tptr = localtime( &secs );
    local_secs = mktime( tptr );
    tptr = gmtime( &secs );
    gmt_secs = mktime( tptr );
    return int64_t(local_secs - gmt_secs);
}

int64_t DateTools::timeMakeInclusive(const int64_t time)
{
    time_t secs = time / 1000;
    tm *ptm = localtime(&secs); // use localtime since mktime below will convert the time to GMT before returning
    ptm->tm_hour = 23;
    ptm->tm_min = 59;
    ptm->tm_sec = 59;

    time_t t = mktime(ptm);
    if (t == -1)
        _CLTHROWA(CL_ERR_Parse, "Input is not a valid date");

    return (static_cast<int64_t>(t) * 1000) + 999;
}

int64_t DateTools::getTime(unsigned short year, uint8_t month, uint8_t mday, uint8_t hours,
                uint8_t minutes, uint8_t seconds, unsigned short ms)
{
	struct tm* s_time;

    // get current time, and then change it according to the parameters
    time_t rawtime;
    time ( &rawtime );
    s_time = localtime ( &rawtime ); // use localtime, since mktime will take into account TZ differences
    s_time->tm_isdst = 0; // since we are using gmtime all around, make sure DST is off

    s_time->tm_year = year - 1900;
    s_time->tm_mon = month - 1;
    s_time->tm_mday = mday;
    s_time->tm_hour = hours;
    s_time->tm_min = minutes;
    s_time->tm_sec = seconds;
    
    time_t t = mktime(s_time);
    if (t == -1)
        _CLTHROWA(CL_ERR_Parse, "Input is not a valid date");

    return (static_cast<int64_t>(t) * 1000) + ms;
}

TCHAR* DateTools::getISOFormat(const int64_t time){
    const time_t secs = time / 1000;
    const int64_t ms = abs((int32_t)((secs * 1000) - time));
    tm *ptm = gmtime(&secs);
    return getISOFormat(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
        ptm->tm_sec, ms);
}

void DateTools::strCatDate(TCHAR* buf, int zeroes, int value){
  TCHAR str[10];
  _i64tot(value, str, 10);
  size_t l = _tcslen(str);

  TCHAR* p = buf;
  for ( size_t i=0;i<(zeroes-l);i++ ){
    *p = _T('0');
    p++;
  }
  _tcscat(p, str);
  p+=l;
  *p = _T('\0');
}
TCHAR* DateTools::getISOFormat(unsigned short year, uint8_t month, uint8_t mday, uint8_t hours,
        uint8_t minutes, uint8_t seconds, unsigned short ms)
{
    TCHAR* ISOString = _CL_NEWARRAY(TCHAR, 24);
    TCHAR* p = ISOString;
    strCatDate(p, 4, year); p+=4;
    _tcscat(p, _T("-")); p++;
    strCatDate(p, 2, month); p+=2;
    _tcscat(p, _T("-")); p++;
    strCatDate(p, 2, mday); p+=2;
    _tcscat(p, _T(" ")); p++;
    strCatDate(p, 2, hours); p+=2;
    _tcscat(p, _T(":")); p++;
    strCatDate(p, 2, minutes); p+=2;
    _tcscat(p, _T(":")); p++;
    strCatDate(p, 2, seconds); p+=2;
    _tcscat(p, _T(":")); p++;
    strCatDate(p, 3, ms); p+=3;

    return ISOString;
}

DateTools::~DateTools(){
}

CL_NS_END
