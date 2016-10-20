#include "WizUdpClient.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QUdpSocket>
#include <QThread>

const int MAX_BUF_LEN = 255;

WizUdpClient::WizUdpClient(QObject* parent)
    : QObject(parent)
{    
}

WizUdpClient::~WizUdpClient()
{
    QMap<QString, QUdpSocket*>::iterator it;
    for (it = m_udpSocketMap.begin(); it != m_udpSocketMap.end(); it++)
    {
        it.value()->close();
        it.value()->deleteLater();
    }
}

bool WizUdpClient::getAllBoardcastAddresses(QMap<QString, QString>& addressMap)
{
    QList<QNetworkInterface> interface = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interface.size(); i++)
    {
        QNetworkInterface item = interface.at(i);
        if (item.isValid() &&  !(item.flags() & QNetworkInterface::CanBroadcast))
            continue;

        QList<QNetworkAddressEntry> entryList = item.addressEntries();
        for (int j = 0; j < entryList.size(); j++)
        {
            QString strIP = entryList.at(j).ip().toString();
            if (strIP.isEmpty() || strIP.startsWith("127"))
                continue;

            QString strBroad = entryList.at(j).broadcast().toString();
            if (!strBroad.isEmpty())
            {
                addressMap.insert(entryList.at(j).ip().toString(), strBroad);
            }
        }
    }
    return true;
}

void WizUdpClient::boardcast(int port, const QString& message)
{   
    QMap<QString, QString> addressMap;
    getAllBoardcastAddresses(addressMap);
    qDebug() << "get address map : " << addressMap << "form thread ; " << QThread::currentThread();
    //
    for (QMap<QString, QString>::const_iterator it = addressMap.begin();
        it != addressMap.end();
        it++)
    {
        QString address = it.key();
        QString broadcast = it.value();
        //
        QUdpSocket* udpSocket;
        if (m_udpSocketMap.find(address) == m_udpSocketMap.end())
        {
            udpSocket = new QUdpSocket();
            udpSocket->bind(QHostAddress(address), port);
            m_udpSocketMap.insert(address, udpSocket);
            connect(udpSocket, SIGNAL(readyRead()),this ,SLOT(readUdpPendingData()), Qt::DirectConnection);
        }
        udpSocket = m_udpSocketMap.find(address).value();


        //  broadcast message
        udpSocket->writeDatagram(message.toUtf8(), QHostAddress(broadcast), port);
        qDebug() << "send upd to " << broadcast << " with message ; " << message;
    }
}

void WizUdpClient::closeUdpConnections()
{
    QMap<QString, QUdpSocket*>::iterator it;
    for (it = m_udpSocketMap.begin(); it != m_udpSocketMap.end(); it++)
    {
        it.value()->close();
    }
    m_udpSocketMap.clear();
}

void WizUdpClient::readUdpPendingData()
{
    QUdpSocket* udpSocket = dynamic_cast<QUdpSocket*>(sender());
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        qint64 size = udpSocket->pendingDatagramSize();
        datagram.resize(size);
        QHostAddress host;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                  &host, &senderPort);

        qDebug() << "upd response from : " << host.toString() << " to local : " << udpSocket->localAddress().toString()
                 << "  with message : " << datagram;
        emit udpResponse(udpSocket->localAddress().toString(),
                         host.toString(), datagram);
    }
}



