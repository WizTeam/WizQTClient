#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <QtGlobal>
#include <QObject>

class QBuffer;

namespace Utils {

class Logger : public QObject
{
    Q_OBJECT

public:
    Logger();
    ~Logger();

#if QT_VERSION < 0x050000
    static void messageHandler(QtMsgType type, const char* msg);
#else
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#endif

    // who is interested in logs just need to connect readyRead signal
    static QBuffer* buffer();

    // obsolete method, will be removed
    static void writeLog(const QString& strMsg);

private:
    QBuffer* m_buffer;

    void prepare();
    void loadBuffer();
    int log2Buffer(const QString strFileName);

    QString lastLogFile(const QString strFileName);
    QString logFile();
    QStringList allLogFiles(int nDays);
    bool logFileWritable(const QString& strFilePath);
    int lines(const QString& strFilePath);
    QString msg2LogMsg(const QString& strMsg);
    void save(const QString& strMsg);
    void redirect(const QString& strMsg);
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
