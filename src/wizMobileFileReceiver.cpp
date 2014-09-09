#include "wizMobileFileReceiver.h"
#include <QUdpSocket>
#include <QTcpSocket>
#include <QXmlStreamReader>
#include <QImage>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

#include "share/wizmisc.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"


CWizMobileFileReceiver::CWizMobileFileReceiver(QObject *parent) :
    QThread(parent)
  , m_udpSocket(0)
  , m_tcpSocket(0)
  , m_xmlProcesser(new CWizMobileXmlProcesser(this))
  , m_stop(false)
{
    connect(m_xmlProcesser, SIGNAL(fileReceived(QString)), SIGNAL(fileReceived(QString)));
}

CWizMobileFileReceiver::~CWizMobileFileReceiver()
{
}

void CWizMobileFileReceiver::initSocket()
{
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::Any, 18695);
    m_tcpSocket = new QTcpSocket(this);

    connect(m_udpSocket, SIGNAL(readyRead()),this ,SLOT(readUdpPendingData()), Qt::DirectConnection);
    qDebug() << "connect tcp : " << connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readTcpPendingData()), Qt::DirectConnection);
    if (!isRunning())
    {
        start();
    }
}

void CWizMobileFileReceiver::waitForDone()
{
    stop();
    WizWaitForThread(this);
}

void CWizMobileFileReceiver::stop()
{
    m_stop = true;
}

void CWizMobileFileReceiver::readUdpPendingData()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        qint64 size = m_udpSocket->pendingDatagramSize();
        datagram.resize(size);
        QHostAddress sender;
        quint16 senderPort;

        m_udpSocket->readDatagram(datagram.data(), datagram.size(),
                                  &sender, &senderPort);

        //parse udp data. connect server is tcp is free
        if (m_tcpSocket->state() == QAbstractSocket::UnconnectedState)
        {
            QString userID;//
            getInfoFromUdpData(datagram, userID);
            if (isUdpSendToCurrentUser(userID))
            {
                qDebug() << sender << " port : " << senderPort;
                m_tcpSocket->connectToHost(sender, 19586, QTcpSocket::ReadOnly);
                m_tcpSocket->waitForConnected(60000);

                m_tcpSocket->waitForReadyRead(60000);
                QByteArray *tcpData = new QByteArray();
                while (m_tcpSocket->bytesAvailable())
                {
                    tcpData->append(m_tcpSocket->readAll());
                }
                m_xmlProcesser->addNewSegment(tcpData);
                m_tcpSocket->disconnectFromHost();

            }
        }
    }
}

void CWizMobileFileReceiver::readTcpPendingData()
{
    static QByteArray *strData = 0;
    while (m_tcpSocket->waitForReadyRead())
    {
        while (m_tcpSocket->bytesAvailable())
        {
            if (strData == 0)
            {
                 strData = new QByteArray();
            }
            strData->append(m_tcpSocket->readLine(3096));

            if (strData->right(1) == "\n")
            {
                strData->remove(strData->length() - 1, 1);
            }
            if (strData->right(17) == "<!--WizDataEnd-->")
            {
                m_xmlProcesser->addNewSegment(strData);
                strData = 0;
                continue;
            }
            if (strData->right(3) == "bye")
            {
                m_tcpSocket->disconnectFromHost();
                delete strData;
                strData = 0;
                qDebug() << "tcp disconnectFromHost";
                return;
            }
        }
    }
    if (strData)
        delete strData;
    strData = 0;
}

void CWizMobileFileReceiver::run()
{
    while (!m_stop)
    {
        if (m_xmlProcesser->hasUnprocessedData())
        {
            m_xmlProcesser->processData();
        }
        else
        {
            sleep(1);
        }
    }
}

void CWizMobileFileReceiver::getInfoFromUdpData(const QByteArray& udpData, QString& userID)
{
    userID = udpData;
}

void CWizMobileXmlProcesser::processXML(const QByteArray& datagram)
{
    QXmlStreamReader xml(datagram);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }

        if(token == QXmlStreamReader::StartElement)
        {
            if(xml.name() == "file")
            {
                processFileParam(xml);
            }
        }
    }
    if (xml.hasError()) {
        qDebug() << "process xml failed : " << xml.errorString();
    }
    xml.clear();
}

void CWizMobileXmlProcesser::processFileParam(QXmlStreamReader& xml)
{
    xml.readNext();
    UdpSegment newSeg;
    QString guid;
    int index;

    while(!(xml.tokenType() == QXmlStreamReader::EndElement &&
            xml.name() == "file"))
    {
        if(xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if(xml.name() == "guid")
            {
                guid = getElementText(xml);
            }
            else if(xml.name() == "name")
            {
                newSeg.name = getElementText(xml);
            }
            else if(xml.name() == "type")
            {
                newSeg.type = getElementText(xml);
            }
            else if(xml.name() == "length")
            {
                newSeg.length = getElementText(xml).toLong();
            }
            else if(xml.name() == "index")
            {
                index = getElementText(xml).toInt();
            }
            else if(xml.name() == "count")
            {
                newSeg.totalCount = getElementText(xml).toInt();
            }
            else if(xml.name() == "data")
            {
                newSeg.data = QByteArray::fromBase64(getElementText(xml).toUtf8());
            }
        }

        xml.readNext();
    }
    addSegmentIntoDataList(guid, index, newSeg);

    if (isSegmentCompleted(guid))
    {
        QString strFileName;
        if(combineSegmentToFile(guid, strFileName))
        {
            qDebug() << "combine sucessed : " << strFileName;
            emit fileReceived(strFileName);
            m_dataMap.remove(guid);
        }
        else
        {
            qDebug() << "combine failed : " << newSeg.name;
        }
    }
}

QString CWizMobileXmlProcesser::getElementText(QXmlStreamReader& xml)
{
    if(xml.tokenType() == QXmlStreamReader::StartElement)
    {
        xml.readNext();
        if(xml.tokenType() == QXmlStreamReader::Characters)
        {
            return xml.text().toString();
        }
    }
    return "";
}

void CWizMobileXmlProcesser::addSegmentIntoDataList(const QString& strGuid, int index, const UdpSegment& newSeg)
{
    if (m_dataMap.contains(strGuid))
    {
        MobileFileData& curData = m_dataMap[strGuid];
        if ((curData.name != newSeg.name) || (curData.length != newSeg.length) ||
                (curData.totalCount != newSeg.totalCount))
        {
            qDebug() << "newSeg confict with otherSeg, guid : " << strGuid;
            return;
        }

        if (curData.data.contains(index))
        {
            qDebug() << QString("segment %1 with index %2 already exists!").arg(strGuid).arg(index);
            return;
        }

        curData.data.insert(index, newSeg.data);
        qDebug() << "new segment accpet : " << index;
    }
    else
    {
        MobileFileData newData;
        newData.data.insert(index, newSeg.data);
        newData.length = newSeg.length;
        newData.name = newSeg.name;
        newData.totalCount = newSeg.totalCount;
        newData.type = newSeg.type;
        m_dataMap.insert(strGuid, newData);
        qDebug() <<"new segment created";
    }
}

bool CWizMobileXmlProcesser::isSegmentCompleted(const QString& strGuid)
{
    return m_dataMap.value(strGuid).data.count() == m_dataMap.value(strGuid).totalCount;
}

bool CWizMobileXmlProcesser::combineSegmentToFile(const QString& strGuid, QString& strFile)
{
    MobileFileData fileData = m_dataMap.value(strGuid);
    if (fileData.totalCount != fileData.data.count())
        return false;

    int segCount = fileData.data.count();
    QByteArray data;
    for (int i = 0;i < segCount; i++)
    {
        if (!fileData.data.contains(i))
            return false;

        data.append(fileData.data.value(i));
    }

    if (fileData.type == "image")
    {
        QImage image;
        if (image.loadFromData(data))
        {
            QString strWizPath = QDir::homePath()  + "/Downloads/WizNote/";
            WizEnsurePathExists(strWizPath);
            strFile = strWizPath + fileData.name;
            return image.save(strFile);
        }
    }

    return false;
}

void CWizMobileFileReceiver::addDataToProcesser(QByteArray* ba)
{
    m_xmlProcesser->addNewSegment(ba);
}

bool CWizMobileFileReceiver::isUdpSendToCurrentUser(const QString& userID)
{
    CWizDatabaseManager *dbMgr = CWizDatabaseManager::instance();
    if (dbMgr)
    {
        CWizDatabase& db = dbMgr->db();
        return db.GetUserGUID() == userID;
    }

    return false;
}



CWizMobileXmlProcesser::CWizMobileXmlProcesser(QObject* parent) : QObject(parent)
{

}

void CWizMobileXmlProcesser::addNewSegment(QByteArray* ba)
{
    if (ba->isEmpty())
        return;
    m_mutex.lock();
    m_segmentList.append(ba);
    m_mutex.unlock();
}

bool CWizMobileXmlProcesser::hasUnprocessedData()
{
    bool hasData;
    m_mutex.lock();
    hasData = m_segmentList.count() > 0;
    m_mutex.unlock();
    return hasData;
}

void CWizMobileXmlProcesser::processData()
{
    QByteArray *ba = peekData();
    if (ba)
    {
        processXML(*ba);
        delete ba;
    }
}


QByteArray *CWizMobileXmlProcesser::peekData()
{
    QByteArray *ba = 0;
    m_mutex.lock();
    if (m_segmentList.count() > 0)
    {
        ba = m_segmentList.first();
        m_segmentList.removeFirst();
    }
    m_mutex.unlock();
    return ba;
}


