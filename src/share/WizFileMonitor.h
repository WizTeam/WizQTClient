#ifndef WIZFILEMONITOR_H
#define WIZFILEMONITOR_H

#include <QThread>
#include <QDateTime>
#include <QList>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>

class WizFileMonitor : public QObject
{
    Q_OBJECT
public:
    explicit WizFileMonitor(QObject *parent = 0);
    static WizFileMonitor& instance();

    void addFile(const QString& strKbGUID, const QString& strGUID, const QString& strFileName,
                 const QString& strMD5);
signals:
    void fileModified(QString strKbGUID, QString strGUID,QString strFileName,
                      QString strMD5, QDateTime dtLastModified);

public slots:
    void on_timerOut();

private:
    void checkFiles();

private:
    struct FMData{
        QString strKbGUID;
        QString strGUID;
        QString strFileName;
        QString strMD5;
        QDateTime dtLastModified;
    };

    QList<FMData> m_fileList;
    QTimer m_timer;
};

#endif // WIZFILEMONITOR_H
