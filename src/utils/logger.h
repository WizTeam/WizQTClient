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

class QBuffer;
class CWizInfo;

namespace Utils {

class Logger : public QObject
{
    Q_OBJECT

protected:
    Logger();
    ~Logger();
public:

#if QT_VERSION < 0x050000
    static void messageHandler(QtMsgType type, const char* msg);
#else
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#endif
    static void writeLog(const QString& strMsg);
    static void getAllLogs(QString& text);
    static Logger* logger();

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
class CWizInfo
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
    inline CWizInfo(QIODevice *device) : stream(new Stream(device)) {}
    inline CWizInfo(QString *string) : stream(new Stream(string)) {}
    inline CWizInfo() : stream(new Stream(QtDebugMsg)) {}
    inline CWizInfo(const CWizInfo &o):stream(o.stream) { ++stream->ref; }
    inline CWizInfo &operator=(const CWizInfo &other);
    ~CWizInfo();
    inline void swap(CWizInfo &other) { qSwap(stream, other.stream); }

    CWizInfo &resetFormat();

    inline CWizInfo &space() { stream->space = true; stream->ts << ' '; return *this; }
    inline CWizInfo &nospace() { stream->space = false; return *this; }
    inline CWizInfo &maybeSpace() { if (stream->space) stream->ts << ' '; return *this; }

    bool autoInsertSpaces() const { return stream->space; }
    void setAutoInsertSpaces(bool b) { stream->space = b; }

    inline CWizInfo &quote() { stream->unsetFlag(Stream::NoQuotes); return *this; }
    inline CWizInfo &noquote() { stream->setFlag(Stream::NoQuotes); return *this; }
    inline CWizInfo &maybeQuote(char c = '"') { if (!(stream->testFlag(Stream::NoQuotes))) stream->ts << c; return *this; }

    inline CWizInfo &operator<<(QChar t) { maybeQuote('\''); stream->ts << t; maybeQuote('\''); return maybeSpace(); }
    inline CWizInfo &operator<<(bool t) { stream->ts << (t ? "true" : "false"); return maybeSpace(); }
    inline CWizInfo &operator<<(char t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(signed short t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(unsigned short t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(signed int t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(unsigned int t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(signed long t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(unsigned long t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(qint64 t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(quint64 t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(float t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(double t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(const char* t) { stream->ts << QString::fromUtf8(t); return maybeSpace(); }
    inline CWizInfo &operator<<(const QString & t) { maybeQuote(); stream->ts << t; maybeQuote(); return maybeSpace(); }
    inline CWizInfo &operator<<(const QStringRef & t) { return operator<<(t.toString()); }
    inline CWizInfo &operator<<(QLatin1String t) { maybeQuote(); stream->ts << t; maybeQuote(); return maybeSpace(); }
    inline CWizInfo &operator<<(const QByteArray & t) { maybeQuote(); stream->ts << t; maybeQuote(); return maybeSpace(); }
    inline CWizInfo &operator<<(const void * t) { stream->ts << t; return maybeSpace(); }
    inline CWizInfo &operator<<(QTextStreamFunction f) {
        stream->ts << f;
        return *this;
    }

    inline CWizInfo &operator<<(QTextStreamManipulator m)
    { stream->ts << m; return *this; }
};



        #define qInfo CWizInfo
    #else
        #define qInfo   qDebug
    #endif
#endif

// obsolete, should remove
#define TOLOG(x)                            Utils::Logger::writeLog(x)
#define TOLOG1(x, p1)                       Utils::Logger::writeLog(WizFormatString1(x, p1))
#define TOLOG2(x, p1, p2)                   Utils::Logger::writeLog(WizFormatString2(x, p1, p2))
#define TOLOG3(x, p1, p2, p3)               Utils::Logger::writeLog(WizFormatString3(x, p1, p2, p3))
#define TOLOG4(x, p1, p2, p3, p4)           Utils::Logger::writeLog(WizFormatString4(x, p1, p2, p3, p4))
#define DEBUG_TOLOG(x)                      Utils::Logger::writeLog(x)
#define DEBUG_TOLOG1(x, p1)                 Utils::Logger::writeLog(WizFormatString1(x, p1))
#define DEBUG_TOLOG2(x, p1, p2)             Utils::Logger::writeLog(WizFormatString2(x, p1, p2))
#define DEBUG_TOLOG3(x, p1, p2, p3)         Utils::Logger::writeLog(WizFormatString3(x, p1, p2, p3))
#define DEBUG_TOLOG4(x, p1, p2, p3, p4)     Utils::Logger::writeLog(WizFormatString4(x, p1, p2, p3, p4))


#endif // UTILS_LOGGER_H
