#include "wizLANFileReceiver.h"
#include <QUdpSocket>
#include <QXmlStreamReader>
#include <QImage>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>


CWizLANFileReceiver::CWizLANFileReceiver(QObject *parent) :
    QObject(parent)
  , m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), SLOT(processData()));
}

void CWizLANFileReceiver::initSocket()
{
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::Any, 18695);

    connect(m_udpSocket, SIGNAL(readyRead()),this ,SLOT(readPendingDatagrams()), Qt::AutoConnection);
}

void CWizLANFileReceiver::readPendingDatagrams()
{
    qDebug() << "readPendingDatagrams called " << m_dataList.count();
    //m_timer->stop();
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray *datagram = new QByteArray();
        qint64 size = m_udpSocket->pendingDatagramSize();
        datagram->resize(size);
        QHostAddress sender;
        quint16 senderPort;

        m_udpSocket->readDatagram(datagram->data(), datagram->size(),
                                  &sender, &senderPort);

        //processXML(datagram);
        m_dataList.append(datagram);
    }
        m_timer->start(5000);
}

void CWizLANFileReceiver::processData()
{
    while (!m_dataList.isEmpty())
    {
        QByteArray *datagram = m_dataList.first();
        processXML(*datagram);
        m_dataList.removeFirst();
        delete datagram;
    }
}

void CWizLANFileReceiver::processXML(const QByteArray& datagram)
{
    qDebug() << datagram;
    QXmlStreamReader xml(datagram);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        qDebug() << "current string : " << xml.text().toString();
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }

        if(token == QXmlStreamReader::StartElement)
        {

            if(xml.name() == "user-name")
            {
                QString strUserName = getElementText(xml);

                //
            }

            if(xml.name() == "file")
            {
                processFileParam(xml);
            }
        }
    }
    if (xml.hasError()) {
        QMessageBox::information(NULL, QString("parseXML"), xml.errorString());
    }
    xml.clear();
}

void CWizLANFileReceiver::processFileParam(QXmlStreamReader& xml)
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
                newSeg.data.insert(0, QByteArray::fromBase64(getElementText(xml).toUtf8()));
            }
        }

        xml.readNext();
    }
    addSegmentIntoList(guid, index, newSeg);

    if (isSegmentCompleted(guid))
    {
        if(combineSegmentToFile(guid))
        {
            qDebug() << "combine sucessed : " << newSeg.name;
        }
        else
        {
            qDebug() << "combine failed : " << newSeg.name;
        }
        m_segmentList.remove(guid);
    }
}

QString CWizLANFileReceiver::getElementText(QXmlStreamReader& xml)
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

void CWizLANFileReceiver::addSegmentIntoList(const QString& strGuid, int index, const UdpSegment& newSeg)
{
    if (m_segmentList.contains(strGuid))
    {
        UdpSegment& curSeg = m_segmentList[strGuid];
        if ((curSeg.name != newSeg.name) || (curSeg.length != newSeg.length) ||
                (curSeg.totalCount != newSeg.totalCount))
        {
            qDebug() << "newSeg confict with otherSeg, guid : " << strGuid;
            return;
        }

        if (curSeg.data.contains(index))
        {
            qDebug() << QString("segment %1 with index %2 already exists!").arg(strGuid).arg(index);
            return;
        }

        curSeg.data.insert(index, newSeg.data.value(0));
        qDebug() << "new segment accpet : " << index;
    }
    else
    {
        m_segmentList.insert(strGuid, newSeg);
        qDebug() <<"new segment created";
    }
}

bool CWizLANFileReceiver::isSegmentCompleted(const QString& strGuid)
{
    return m_segmentList.value(strGuid).data.count() == m_segmentList.value(strGuid).totalCount;
}

bool CWizLANFileReceiver::combineSegmentToFile(const QString& strGuid)
{
    UdpSegment newSeg = m_segmentList.value(strGuid);
    int segCount = newSeg.data.count();
    QByteArray data;
    for (int i = 0;i < segCount; i++)
    {
        if (!newSeg.data.contains(i))
            return false;

        data.append(newSeg.data.value(i));
    }

    if (newSeg.type == "image")
    {
        QImage image;
        if (image.loadFromData(data))
        {
            return image.save(newSeg.name);
        }
    }

    return false;
}



CWizLANXmlProcesser::CWizLANXmlProcesser(QObject* parent) : QThread(parent)
{

}

void CWizLANXmlProcesser::run()
{

}
