#include "logger.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QDebug>
#include <QBuffer>
#include <QTextStream>
#include <iostream>
#include <fstream>
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

    //FIXME: useless waning message from qt, ignore it
#ifndef QT_DEBUG
    if (msg.startsWith("libpng warning: iCCP:") || msg.startsWith("QSslSocket: cannot call unresolved"))
        return;
#endif

    bool saveLog = true;
#ifndef QT_DEBUG
    if (type == QtDebugMsg)
        saveLog = false;
#endif
    if (saveLog)
    {
        logger()->saveToLogFile(msg);
        logger()->addToBuffer(msg);
    }

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[DEBUG] %s\n", msg.toUtf8().constData());
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        fprintf(stderr, "[INFO] %s\n", msg.toUtf8().constData());
        break;
#endif
    case QtWarningMsg:
        fprintf(stderr, "[WARNING]: %s (%s:%u, %s)\n", msg.toUtf8().constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[CRITICAL]: %s (%s:%u, %s)\n", msg.toUtf8().constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "[FATAL]: %s (%s:%u, %s)\n", msg.toUtf8().constData(), context.file, context.line, context.function);
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
//    QMutexLocker locker(&m_mutex);
//    Q_UNUSED(locker);

    std::ofstream logFile;
    logFile.open(logFileName().toUtf8().constData(), std::ios::out | std::ios::app);
    logFile << msg2LogMsg(strMsg).toUtf8().constData() << std::endl;
    logFile.close();
}

void Logger::addToBuffer(const QString& strMsg)
{
//    QMutexLocker locker(&m_mutex);
//    Q_UNUSED(locker);
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
    logger()->saveToLogFile(strMsg);
    logger()->addToBuffer(strMsg);

    fprintf(stderr, "[INFO] %s\n", strMsg.toUtf8().constData());
}
void Logger::getAllLogs(QString& text)
{
    logger()->getAll(text);
}

} // namespace Utils

#if QT_VERSION < 0x050500 && QT_VERSION > 0x050000
CWizInfo::~CWizInfo()
{
    if (!--stream->ref) {
        if (stream->space && stream->buffer.endsWith(QLatin1Char(' ')))
            stream->buffer.chop(1);
        if (stream->message_output) {
            qt_message_output(stream->type,
                              stream->context,
                              stream->buffer);
//            Utils::Logger::logger()->writeLog(stream->buffer);
        }
        delete stream;
    }
}
#endif
