#include "WizLogger.h"

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
#include "WizMisc.h"
#include "WizPathResolve.h"

#define LOG_LINES_MAX 30000
#define LOG_DAYS_MAX 10
#define LOG_LINES_BUFFER_MAX 3000

namespace Utils {

WizLogger::WizLogger()
    : m_buffer(new QBuffer())
    , m_mutex(QMutex::Recursive)
{
    connect(m_buffer, SIGNAL(readyRead()), SLOT(onBuffer_readRead()));
}

WizLogger::~WizLogger()
{
}


void WizLogger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString &msg)
{
    Q_UNUSED(context);

    //FIXME: useless waning message from qt, ignore it
#ifndef QT_DEBUG
    if (msg.startsWith("libpng warning: iCCP:") || msg.startsWith("QSslSocket: cannot call unresolved"))
        return;
#endif

    if (type == QtWarningMsg && msg.startsWith("Property") && msg.indexOf("has no notify signal") != -1)
        return;

    logger()->saveToLogFile(msg);
    logger()->addToBuffer(msg);

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
        break;
    }
}

QString WizLogger::logFileName()
{
    QString strFileName = WizPathResolve::logFile();

    if (WizMisc::getFileSize(strFileName) > 10 * 1024 * 1024)
    {
        WizMisc::deleteFile(strFileName);
    }
    //
    return strFileName;
}

QString WizLogger::msg2LogMsg(const QString& strMsg)
{
    QString strTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    return strTime + ": " + strMsg + "\n";
}

void WizLogger::saveToLogFile(const QString& strMsg)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    std::ofstream logFile;
    logFile.open(logFileName().toUtf8().constData(), std::ios::out | std::ios::app);
    logFile << msg2LogMsg(strMsg).toUtf8().constData() << std::endl;
    logFile.close();
}

void WizLogger::addToBuffer(const QString& strMsg)
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);
    //
    m_buffer->open(QIODevice::Append);
    m_buffer->write(msg2LogMsg(strMsg).toUtf8());
    m_buffer->close();
}

void WizLogger::getAll(QString &text)
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

WizLogger* WizLogger::logger()
{
    static WizLogger logger;
    return &logger;
}

void WizLogger::writeLog(const QString& strMsg)
{
    if (strMsg.startsWith("[WARNING]: Property "))
        return;
    logger()->saveToLogFile(strMsg);
    logger()->addToBuffer(strMsg);

    fprintf(stderr, "[INFO] %s\n", strMsg.toUtf8().constData());
}
void WizLogger::getAllLogs(QString& text)
{
    logger()->getAll(text);
}

} // namespace Utils

#if QT_VERSION < 0x050500 && QT_VERSION > 0x050000
WizInfo::~WizInfo()
{
    if (!--stream->ref) {
        if (stream->space && stream->buffer.endsWith(QLatin1Char(' ')))
            stream->buffer.chop(1);
        if (stream->message_output) {
//            qt_message_output(stream->type,
//                              stream->context,
//                              stream->buffer);
            Utils::WizLogger::logger()->writeLog(stream->buffer);
        }
        delete stream;
    }
}
#endif
