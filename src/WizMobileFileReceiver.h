#ifndef WIZMOBILEFILERECEIVER_H
#define WIZMOBILEFILERECEIVER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTcpSocket>

struct UdpSegment
{
    QString name;
    QString type;
    long    length;
    int       totalCount;
    QByteArray data;
};

struct MobileFileData
{
    QString name;
    QString type;
    long      length;
    int         totalCount;
    QMap<int, QByteArray> data;
};

class QMutex;
class QTimer;
class QUdpSocket;
class QTcpSocket;
class QXmlStreamReader;

class WizMobileXmlProcesser : public QThread
{
    Q_OBJECT
public:
    explicit WizMobileXmlProcesser(QObject *parent = 0);

    void addNewSegment(QByteArray *ba);

    void processData();

    void waitForDone();
    void stop();

signals:
    void fileReceived(QString strFileName);

protected:
    void run();

private:
    QByteArray * peekData();
    //
    void processXML(const QByteArray& datagram);
    void processFileParam(QXmlStreamReader& xml);
    QString getElementText(QXmlStreamReader& xml);
    void addSegmentIntoDataList(const QString& strGuid, int index ,const UdpSegment& newSeg);
    bool isSegmentCompleted(const QString& strGuid);
    bool combineSegmentToFile(const QString& strGuid, QString& strFile);

    void deleteAllSegments();
private:
    QMutex m_mutex;
    QWaitCondition m_wait;
    QList<QByteArray *> m_segmentList;
    QMap<QString, MobileFileData> m_dataMap;
    bool m_stop;
};


class WizMobileTcpContainer : public QThread
{
    Q_OBJECT
public:
    explicit WizMobileTcpContainer(WizMobileXmlProcesser *xmlProcesser, QObject *parent = 0);
    ~WizMobileTcpContainer();

    QAbstractSocket::SocketState tcpState();

public slots:
    void readTcpPendingData();
    void connectToHost(const QString& address, quint16 port);

protected:
    void run();

private:
    WizMobileXmlProcesser *m_xmlProcesser;
    QTcpSocket *m_tcpSocket;
    QString m_strHost;

    void initTcpSocket();
};

class WizMobileFileReceiver : public QThread
{
    Q_OBJECT
public:
    explicit WizMobileFileReceiver(QObject *parent = 0);
    ~WizMobileFileReceiver();

    void initSocket();

    void waitForDone();

signals:
    void fileReceived(QString strFileName);
    void connectToHost(QString address, quint16 port);

public slots:
    void readUdpPendingData();

protected:
    void run();

private:
    void getInfoFromUdpData(const QByteArray& udpData, QString& userID);

    //
    void addDataToProcesser(QByteArray *ba);
    bool isUdpSendToCurrentUser(const QString& userID);
private:
    QUdpSocket *m_udpSocket;
    WizMobileXmlProcesser *m_xmlProcesser;
    WizMobileTcpContainer *m_tcpContainer;
};

#endif // WIZMOBILEFILERECEIVER_H
