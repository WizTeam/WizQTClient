#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <QtGlobal>
#include <QObject>
#include <QMutex>
#include <QtCore/qalgorithms.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>
#include <QtCore/qset.h>
#include <QtCore/qcontiguouscache.h>

#include "share/WizMisc.h"


class QBuffer;
class WizInfo;

namespace Utils {

class WizLogger : public QObject
{
    Q_OBJECT

protected:
    WizLogger();
    ~WizLogger();
public:

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void writeLog(const QString& strMsg);
    static void getAllLogs(QString& text);
    static WizLogger* logger();

private Q_SLOTS:
    void onBuffer_readRead() { emit readyRead(); }
Q_SIGNALS:
    void readyRead();

private:
    QMutex m_mutex;
    QBuffer* m_buffer;

    void getAll(QString& text);

    QString logFileName();
    QString msg2LogMsg(const QString& strMsg);
    void saveToLogFile(const QString& strMsg);
    void addToBuffer(const QString& strMsg);
};

} // namespace Utils


#if QT_VERSION < 0x050500
    #if QT_VERSION > 0x050000
class WizInfo
{
    struct Stream {
        Stream(QIODevice *device) : ts(device), ref(1), type(QtDebugMsg), space(true), message_output(false), flags(0) {}
        Stream(QString *string) : ts(string, QIODevice::WriteOnly), ref(1), type(QtDebugMsg), space(true), message_output(false), flags(0) {}
        Stream(QtMsgType t) : ts(&buffer, QIODevice::WriteOnly), ref(1), type(t), space(true), message_output(true), flags(0) {}
        QTextStream ts;
        QString buffer;
        int ref;
        QtMsgType type;
        bool space;
        bool message_output;
        QMessageLogContext context;

        enum FormatFlag {
            NoQuotes = 0x1
        };

        // ### Qt 6: unify with space, introduce own version member
        bool testFlag(FormatFlag flag) const { return (context.version > 1) ? (flags & flag) : false; }
        void setFlag(FormatFlag flag) { if (context.version > 1) { flags |= flag; } }
        void unsetFlag(FormatFlag flag) { if (context.version > 1) { flags &= ~flag; } }

        // added in 5.4
        int flags;
    } *stream;
public:
    inline WizInfo(QIODevice *device) : stream(new Stream(device)) {}
    inline WizInfo(QString *string) : stream(new Stream(string)) {}
    inline WizInfo() : stream(new Stream(QtDebugMsg)) {}
    inline WizInfo(const WizInfo &o):stream(o.stream) { ++stream->ref; }
    inline WizInfo &operator=(const WizInfo &other);
    ~WizInfo();
    inline void swap(WizInfo &other) { qSwap(stream, other.stream); }

    WizInfo &resetFormat();

    inline WizInfo &space() { stream->space = true; stream->ts << ' '; return *this; }
    inline WizInfo &nospace() { stream->space = false; return *this; }
    inline WizInfo &maybeSpace() { if (stream->space) stream->ts << ' '; return *this; }

    bool autoInsertSpaces() const { return stream->space; }
    void setAutoInsertSpaces(bool b) { stream->space = b; }

    inline WizInfo &quote() { stream->unsetFlag(Stream::NoQuotes); return *this; }
    inline WizInfo &noquote() { stream->setFlag(Stream::NoQuotes); return *this; }
    inline WizInfo &maybeQuote(char c = '"') { if (!(stream->testFlag(Stream::NoQuotes))) stream->ts << c; return *this; }

    inline WizInfo &operator<<(QChar t) { maybeQuote('\''); stream->ts << t; maybeQuote('\''); return maybeSpace(); }
    inline WizInfo &operator<<(bool t) { stream->ts << (t ? "true" : "false"); return maybeSpace(); }
    inline WizInfo &operator<<(char t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(signed short t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(unsigned short t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(signed int t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(unsigned int t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(signed long t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(unsigned long t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(qint64 t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(quint64 t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(float t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(double t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(const char* t) { stream->ts << QString::fromUtf8(t); return maybeSpace(); }
    inline WizInfo &operator<<(const QString & t) { maybeQuote(); stream->ts << t; maybeQuote(); return maybeSpace(); }
    inline WizInfo &operator<<(const QStringRef & t) { return operator<<(t.toString()); }
    inline WizInfo &operator<<(QLatin1String t) { maybeQuote(); stream->ts << t; maybeQuote(); return maybeSpace(); }
    inline WizInfo &operator<<(const QByteArray & t) { maybeQuote(); stream->ts << t; maybeQuote(); return maybeSpace(); }
    inline WizInfo &operator<<(const void * t) { stream->ts << t; return maybeSpace(); }
    inline WizInfo &operator<<(QTextStreamFunction f) {
        stream->ts << f;
        return *this;
    }

    inline WizInfo &operator<<(QTextStreamManipulator m)
    { stream->ts << m; return *this; }
};



        #define qInfo WizInfo
    #else
        #define qInfo   qDebug
    #endif
#endif

// obsolete, should remove
#define TOLOG(x)                            Utils::WizLogger::writeLog(x)
#define TOLOG1(x, p1)                       Utils::WizLogger::writeLog(WizFormatString1(x, p1))
#define TOLOG2(x, p1, p2)                   Utils::WizLogger::writeLog(WizFormatString2(x, p1, p2))
#define TOLOG3(x, p1, p2, p3)               Utils::WizLogger::writeLog(WizFormatString3(x, p1, p2, p3))
#define TOLOG4(x, p1, p2, p3, p4)           Utils::WizLogger::writeLog(WizFormatString4(x, p1, p2, p3, p4))
#define DEBUG_TOLOG(x)                      Utils::WizLogger::writeLog(x)
#define DEBUG_TOLOG1(x, p1)                 Utils::WizLogger::writeLog(WizFormatString1(x, p1))
#define DEBUG_TOLOG2(x, p1, p2)             Utils::WizLogger::writeLog(WizFormatString2(x, p1, p2))
#define DEBUG_TOLOG3(x, p1, p2, p3)         Utils::WizLogger::writeLog(WizFormatString3(x, p1, p2, p3))
#define DEBUG_TOLOG4(x, p1, p2, p3, p4)     Utils::WizLogger::writeLog(WizFormatString4(x, p1, p2, p3, p4))


#endif // UTILS_LOGGER_H
