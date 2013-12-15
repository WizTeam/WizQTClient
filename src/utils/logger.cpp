#include "logger.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QDebug>
#include <QBuffer>
#include <QTextStream>

#include "pathresolve.h"

#define LOG_LINES_MAX 30000
#define LOG_DAYS_MAX 10
#define LOG_LINES_BUFFER_MAX 3000

namespace Utils {

static Logger* m_instance = 0;

Logger::Logger()
    : m_buffer(new QBuffer())
{
    Q_ASSERT(!m_instance);

    m_instance = this;

    prepare();
    loadBuffer();
}

Logger::~Logger()
{
    m_instance = 0;
}

void Logger::prepare()
{
    QString strLogPath = PathResolve::logPath();
    // older version: move log file to new location
    QFile fOld(PathResolve::dataStorePath() + "wiznote.log");
    if (fOld.exists()) {
        fOld.copy(strLogPath + logFile() + ".old");
        fOld.remove();
    }

    QDir dirLog(strLogPath);
    QStringList liAll = dirLog.entryList(QDir::Files);
    QStringList liNew = allLogFiles(LOG_DAYS_MAX);
    qDebug() << liNew;
    for (int i = 0; i < liAll.size(); i++) {
        if (!liNew.contains(liAll.at(i))) {
            dirLog.remove(liAll.at(i));
            qDebug() << "[Logger]Remove old log file:" << liAll.at(i);
        }
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

QStringList Logger::allLogFiles(int nDays)
{
    QStringList liFilter;

    if (nDays == -1) {
        liFilter << "wiznote_*";
    } else {
        int d = 0;
        while (d < nDays) {
            QString strDate = QDate::currentDate().addDays(-d).toString(Qt::ISODate);
            QString strName = "wiznote_" + strDate + "*";
            liFilter << strName;
            d++;
        }
    }

    QDir dirLog(PathResolve::logPath());
    return dirLog.entryList(liFilter, QDir::Files, QDir::Time);
}

QString Logger::lastLogFile(const QString strFileName)
{
    QDir dirLog(PathResolve::logPath());
    QStringList liAll = dirLog.entryList(QDir::Files, QDir::Time);
    int i = liAll.indexOf(strFileName);

    if (i == -1)
        return 0;

    return liAll.at(i+1);
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

    return strFileName.arg(id);
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

QString Logger::msg2LogMsg(const QString& strMsg)
{
    QString strTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    return strTime + ": " + strMsg + "\n";
}

void Logger::save(const QString& strMsg)
{
    QFile f(PathResolve::logPath() + logFile());
    f.open(QIODevice::Append | QIODevice::Text);
    f.write(msg2LogMsg(strMsg).toUtf8());
    f.close();
}

void Logger::loadBuffer()
{
    QString strCurrentLog = logFile();

    int nLines = log2Buffer(strCurrentLog);
    while (nLines < LOG_LINES_BUFFER_MAX) {
        QString strLast = lastLogFile(strCurrentLog);
        if (strLast.isEmpty()) {
            break;
        } else {
            nLines += log2Buffer(strLast);
        }
    }
}

int Logger::log2Buffer(const QString strFileName)
{
    if (strFileName.isEmpty())
        return 0;

    QString strFilePath = PathResolve::logPath() + strFileName;
    if (!QFile::exists(strFilePath))
        return 0;

    QFile f(strFilePath);
    if (!f.open(QIODevice::ReadOnly)) {
        return 0;
    }

    QTextStream ts(&f);
    int nLines = 0;

    QByteArray bytes = m_buffer->data();
    m_buffer->open(QIODevice::Truncate| QIODevice::Text);

    do {
        m_buffer->write(ts.readLine().toUtf8());
        m_buffer->write("\n");
        nLines++;
    } while (!ts.atEnd());

    f.close();

    m_buffer->write(bytes);
    m_buffer->close();

    return nLines;
}

void Logger::redirect(const QString& strMsg)
{
    // FIXME: truncate old entries
    m_buffer->open(QIODevice::Append);
    m_buffer->write(msg2LogMsg(strMsg).toUtf8());
    m_buffer->close();
}

void Logger::writeLog(const QString& strMsg)
{
    qDebug() << strMsg;
}


} // namespace Utils
