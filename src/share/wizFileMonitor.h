#ifndef WIZFILEMONITOR_H
#define WIZFILEMONITOR_H

#include <QThread>
#include <QDateTime>
#include <QList>

class CWizFileMonitor : public QThread
{
    Q_OBJECT
public:
    explicit CWizFileMonitor(QObject *parent = 0);
    ~CWizFileMonitor();
    static CWizFileMonitor& instance();

    void addFile(const QString strKbGUID, const QString& strGUID, const QString& strFileName,
                 const QString& strMD5, const QDateTime& dtLastModified);
    void stop();
signals:
    void fileModified(QString strKbGUID, QString strGUID,QString strFileName,
                      QString strMD5, QDateTime dtLastModified);

public slots:

protected:
    virtual void run();

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
    bool m_stop;
};

#endif // WIZFILEMONITOR_H
