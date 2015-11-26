#ifndef WIZQTHELPER_H
#define WIZQTHELPER_H

#include <QtGlobal>

#include <map>
#include <deque>
#include <set>
#include <functional>

#include <assert.h>


#include <QString>
#include <QDateTime>
#include <QSharedPointer>

#ifndef BOOL
    #define BOOL bool
#endif

#ifndef TRUE
    #define TRUE    true
#endif

#ifndef FALSE
    #define FALSE  false
#endif

#define COLORREF    int
#define _T(x)       x

#ifdef Q_OS_LINUX
typedef long long __int64;
#endif

#ifdef Q_OS_MAC
typedef long long __int64;
#endif

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int   DWORD;
typedef unsigned int    UINT;
typedef long HRESULT;

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define S_OK                                   ((HRESULT)0L)
#define S_FALSE                                ((HRESULT)1L)
#define E_FAIL                                  0x80004005L
#define E_NOTIMPL                               0x80004001L
#define E_OUTOFMEMORY                           0x8007000EL
#define E_INVALIDARG                            0x80070057L
#define E_NOINTERFACE                           0x80004002L
#define E_POINTER                               0x80004003L



#define LOBYTE(w)           ((BYTE)(((unsigned long)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((unsigned long)(w)) >> 8) & 0xff))

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))

#define ATLASSERT(x)        assert(x)

class CString: public QString
{
public:
    inline CString() : QString() {}
    CString(const QChar *unicode, int size) : QString(unicode, size) {}
    explicit CString(const QChar *unicode) : QString(unicode) {}
    CString(QChar c) :QString(c) {}
    CString(int size, QChar c) :QString(size, c) {}
    inline CString(const QLatin1String &latin1) : QString(latin1) {}
    inline CString(const QString &src) : QString(src) {}
    //inline CString(const CString &src) : QString(src) {}
#ifndef QT_NO_CAST_FROM_ASCII
    CString(const char *ch) :QString(ch) {}
#endif //QT_NO_CAST_FROM_ASCII
    //
    CString(const unsigned short* pUtf16) { *this = fromUtf16(pUtf16); }
    CString(const unsigned short* pUtf16, int size) { *this = fromUtf16(pUtf16, size); }
    //
    bool IsEmpty() const { return isEmpty(); }
    operator const unsigned short*() const { return utf16(); }
    //
    //QString &Format(const char *format, ...) { va_list p; return vsprintf(format, p); }
    //
    void Delete(int index, int len) { remove(index, len); }
    void Trim() { *this = trimmed(); }
    void Trim(char ch);
    void TrimLeft();
    void TrimRight();
    CString MakeLower() { *this = toLower(); return *this; }
    CString MakeUpper() { *this = toUpper(); return *this; }
    int GetLength() const { return length(); }
    int CompareNoCase(const CString& strOther) const { return compare(strOther, Qt::CaseInsensitive); }
    int Compare(const CString& strOther) const { return compare(strOther, Qt::CaseSensitive); }
    void Format(const CString& strFormat, ... );
    void Empty() { clear(); }
    int Find(char ch) const { return indexOf(ch); }
    int Find(char ch, int start) const { return indexOf(ch, start); }
    int Find(const CString& str) const { return indexOf(str); }
    CString Left(int count) const { return left(count); }
    CString Right(int count) const { return right(count); }
    void Append(const CString& str) { append(str); }
    void AppendChar(char ch) { append(ch); }
    void Insert(int index, const CString& str);
    void Insert(int index, QChar ch);
    void SetAt(int index, QChar ch);
    CString Mid(int iFirst, int nCount) const { return mid(iFirst, nCount); }
    CString Mid(int iFirst) const { return mid(iFirst); }
    void Remove(QChar ch) { remove(ch); }
    void Replace(QChar chFind, QChar chReplace) { replace(chFind, chReplace); }
    void Replace(const CString& strFind, const CString& strReplace) { replace(strFind, strReplace); }
    int FindOneOf(const CString& strFind) const;
};


class COleDateTime: public QDateTime
{
public:
    COleDateTime() :QDateTime() { *this = QDateTime::currentDateTime(); }
    COleDateTime(const QDateTime& other) { *this = other; }
    COleDateTime(int year, int month, int day, int hour, int minute, int second) : QDateTime(QDate(year, month, day), QTime(hour, minute, second))  {}
    //
    COleDateTime(time_t t) { this->setTime_t(t);}
    //
    int GetYear() const { return date().year(); }
    int GetMonth() const { return date().month(); }
    int GetDay() const { return date().day(); }
    int GetHour() const { return time().hour(); }
    int GetMinute() const { return time().minute(); }
    int GetSecond() const { return time().second(); }
    int GetDayOfYear() const { return date().dayOfYear(); }
    int GetDayOfWeek() const { return date().dayOfWeek(); }

    QString toHumanFriendlyString() const;
    //
    COleDateTime &operator=(const QDateTime &other);
    COleDateTime &operator=(const COleDateTime &other);
};

int GetTickCount();
bool PathFileExists(const CString& strPath);
bool DeleteFile(const CString& strFileName);

int _tcsicmp(const CString& str1, const CString& str2);
int _tcsnicmp(const CString& str1, const CString& str2, int count);
int _ttoi(const CString& str);
__int64 _ttoi64(const CString& str);


#define UNUSED_ALWAYS(x)        Q_UNUSED(x)

COLORREF WizGetSysColor(int id);

unsigned short* wiz_strinc(const unsigned short* current);
int wiz_isdigit(int c);
int wiz_isupper(int c);
int wiz_isupper(QChar c);
int wiz_isalpha(int c);
int wiz_isalpha(QChar c);
int wiz_isxdigit(int c);
int wiz_isxdigit(QChar c);
size_t wiz_strlen(const unsigned short* str);
long wiz_strtoul( const unsigned short *nptr, unsigned short **endptr, int base );
long wiz_strtoul( const unsigned short *nptr, QChar endchar, int base );
int wiz_atoi(const unsigned short *nptr);
int wiz_isspace(int c);
int wiz_isalnum(int c);
unsigned short * wiz_strstr( const unsigned short *string, const CString& strFind);
unsigned short * wiz_strstr( const unsigned short *wcs1, const unsigned short* wcs2);
unsigned short * wiz_strchr( const unsigned short *string, int ch);
unsigned short * wiz_strchr( const unsigned short *string, QChar ch);
int wiz_strncmp( const unsigned short* string1, const unsigned short* string2, size_t count );
int wiz_strncmp( const unsigned short* string1, const char* string2, size_t count );
int wiz_strnicmp(const unsigned short* first, const unsigned short* last, size_t count );
int wiz_strnicmp(const unsigned short* first, const char* last, size_t count );


template <class T>
bool WizMapLookup(const T& m, const typename T::key_type& key, typename T::mapped_type& value)
{
    typename T::const_iterator it = m.find(key);
    if (it == m.end())
        return false;
    //
    value = it->second;
    //
    return true;
}

class WizScopeGuard
{
public:
    explicit WizScopeGuard(std::function<void()> onExitScope)
        : onExitScope_(onExitScope)
    { }
    ~WizScopeGuard()
    {
            onExitScope_();
    }
private:
    std::function<void()> onExitScope_;
private: // noncopyable
    WizScopeGuard(WizScopeGuard const&);
    WizScopeGuard& operator=(WizScopeGuard const&);
};

typedef std::deque<CString> CWizStdStringArray;

#endif // WIZQTHELPER_H
