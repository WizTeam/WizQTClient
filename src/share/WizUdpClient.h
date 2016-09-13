#ifndef CWIZUDPCLIENT_H
#define CWIZUDPCLIENT_H

#include <QString>
#include <QMap>
#include <QList>
#include <QObject>

class QUdpSocket;


class WizUdpClient : public QObject
{
    Q_OBJECT
public:
    WizUdpClient(QObject *parent = 0);
    ~WizUdpClient();

signals:
    void udpResponse(const QString& boardAddress, const QString& serverAddress,
                       const QString& udpMessage);

public slots:
    void boardcast(int port, const QString& message);
    void closeUdpConnections();

private slots:
    void readUdpPendingData();

private:
    static bool getAllBoardcastAddresses(QMap<QString, QString>& addressMap);

private:
//    QList<QUdpSocket*> m_udpSocketList;
    QMap<QString, QUdpSocket*> m_udpSocketMap;
};

#endif // CWIZUDPCLIENT_H
