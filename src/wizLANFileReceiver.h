#ifndef WIZLANFILERECEIVER_H
#define WIZLANFILERECEIVER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QThread>

struct UdpSegment
{
    QString name;
    QString type;
    long length;
    int     totalCount;
    QMap<int, QByteArray> data;
};

class QTimer;
class QUdpSocket;
class QXmlStreamReader;

class CWizLANXmlProcesser : public QThread
{
    Q_OBJECT
public:
    explicit CWizLANXmlProcesser(QObject *parent = 0);

protected:
    void run();
};

class CWizLANFileReceiver : public QObject
{
    Q_OBJECT
public:
    explicit CWizLANFileReceiver(QObject *parent = 0);

    void initSocket();

signals:

public slots:
    void readPendingDatagrams();
    void processData();

private:
    void processXML(const QByteArray& datagram);
    void processFileParam(QXmlStreamReader& xml);
    QString getElementText(QXmlStreamReader& xml);
    void addSegmentIntoList(const QString& strGuid, int index ,const UdpSegment& newSeg);
    bool isSegmentCompleted(const QString& strGuid);
    bool combineSegmentToFile(const QString& strGuid);

private:
    QUdpSocket *m_udpSocket;
    QMap<QString, UdpSegment> m_segmentList;
    QTimer *m_timer;
    QList<QByteArray* > m_dataList;
};

#endif // WIZLANFILERECEIVER_H
