#include "logger.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QDebug>
#include <QBuffer>
#include <QTextStream>
#include "misc.h"
#include "pathresolve.h"

#define LOG_LINES_MAX 30000
#define LOG_DAYS_MAX 10
#define LOG_LINES_BUFFER_MAX 3000

namespace Utils {

Logger::Logger()
    : m_buffer(new QBuffer())
    , m_mutex(QMutex::Recursive)
{
    connect(m_buffer, SIGNAL(readyRead()), SLOT(onBuffer_readRead()));
}

Logger::~Logger()
{
}


#if QT_VERSION < 0x050000
void Logger::messageHandler(QtMsgType type, const char* msg)
{
    QString text = QString::fromUtf8(msg);
    logger()->saveToLogFile(text);
    logger()->addToBuffer(text);

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[DEBUG] %s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "[WARNING]: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[CRITICAL]: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "[FATAL]: %s\n", msg);
        abort();
    }
}
#else
void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString &msg)
{
    Q_UNUSED(context);

    //QTBug: ignore these
    if (msg.startsWith("libpng warning: iCCP"))
        return;

    logger()->saveToLogFile(msg);
    logger()->addToBuffer(msg);

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[DEBUG] %s\n", msg.toUtf8().constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "[WARNING]: %s\n", msg.toUtf8().constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[CRITICAL]: %s\n", msg.toUtf8().constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "[FATAL]: %s\n", msg.toUtf8().constData());
        abort();
    }
}
#endif

QString Logger::logFileName()
{
    QString strFileName = PathResolve::logFile();

    if (Misc::getFileSize(strFileName) > 10 * 1024 * 1024)
    {
        Misc::deleteFile(strFileName);
    }
    //
    return strFileName;
}

QString Logger::msg2LogMsg(const QString& strMsg)
{
    QString strTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    return strTime + ": " + strMsg + "\n";
}

void Logger::saveToLogFile(const QString& strMsg)
{
    QFile f(logFileName());
    f.open(QIODevice::Append | QIODevice::Text);
    f.write(msg2LogMsg(strMsg).toUtf8());
    f.close();
}

void Logger::addToBuffer(const QString& strMsg)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_buffer->open(QIODevice::Append);
    m_buffer->write(msg2LogMsg(strMsg).toUtf8());
    m_buffer->close();
}

void Logger::getAll(QString &text)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_buffer->open(QIODevice::ReadOnly);
    QByteArray data = m_buffer->readAll();
    text = QString::fromUtf8(data.data());
    m_buffer->close();
    m_buffer->setBuffer(NULL);
}

Logger* Logger::logger()
{
    static Logger logger;
    return &logger;
}

void Logger::writeLog(const QString& strMsg)
{
    qDebug() << strMsg;
}
void Logger::getAllLogs(QString& text)
{
    logger()->getAll(text);
}

} // namespace Utils
