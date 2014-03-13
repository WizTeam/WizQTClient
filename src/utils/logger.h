#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <QtGlobal>
#include <QObject>
#include <QMutex>

class QBuffer;

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
