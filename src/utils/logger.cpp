#include "logger.h"

#include <QString>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QDebug>
#include <QBuffer>

#include "pathresolve.h"

#define LOG_LINES_MAX 10000

namespace Utils {

static Logger* m_instance = 0;

Logger::Logger()
    : m_buffer(new QBuffer())
{
    Q_ASSERT(!m_instance);

    m_instance = this;

    prepare();
}

Logger::~Logger()
{
    m_instance = 0;
}

void Logger::prepare()
{
    // older version: move log file to new location
    QFile fOld(PathResolve::dataStorePath() + "wiznote.log");
    if (fOld.exists()) {
        fOld.copy(logFile());
        fOld.remove();
    }
}

QBuffer* Logger::buffer()
{
    return m_instance->m_buffer;
}

void Logger::messageHandler(QtMsgType type, const char* msg)
{
    m_instance->save(QString::fromUtf8(msg));
    m_instance->redirect(QString::fromUtf8(msg));

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

QString Logger::logFile()
{
    QString strDate = QDate::currentDate().toString(Qt::ISODate);
    QString strFileName = "wiznote_" + strDate + "_%1.log";

    int id = 0;
    QString strFilePath = PathResolve::logPath() + strFileName;
    while (!logFileWritable(strFilePath.arg(id))) {
        id++;
    }

    return strFilePath.arg(id);
}

bool Logger::logFileWritable(const QString& strFilePath)
{
    if (QFile::exists(strFilePath) && lines(strFilePath) > LOG_LINES_MAX) {
        return false;
    }

    return true;
}

int Logger::lines(const QString& strFilePath)
{
    QFile f(strFilePath);
    if (!f.open(QIODevice::ReadOnly))
        return LOG_LINES_MAX + 1;

    int i = 0;
    while (!f.atEnd()) {
        f.readLine();
        i++;
    }

    return i;
}

QString Logger::msg2Log(const QString& strMsg)
{
    QString strTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    return strTime + ": " + strMsg + "\n";
}

void Logger::save(const QString& strMsg)
{
    QFile f(logFile());
    f.open(QIODevice::Append | QIODevice::Text);
    f.write(msg2Log(strMsg).toUtf8());
    f.close();
}

void Logger::redirect(const QString& strMsg)
{
    m_buffer->open(QIODevice::Append);
    m_buffer->write(msg2Log(strMsg).toUtf8());
    m_buffer->close();
}

void Logger::writeLog(const QString& strMsg)
{
    qDebug() << strMsg;
}


} // namespace Utils
