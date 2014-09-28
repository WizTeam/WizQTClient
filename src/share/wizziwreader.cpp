#include "wizziwreader.h"

#include <QDataStream>
#include <QFile>

#include "wizmisc.h"
#include "wizenc.h"
#include "utils/logger.h"


CWizZiwReader::CWizZiwReader(QObject *parent)
    : QObject(parent)
    , m_bSaveUserCipher(false)
{
    memset(&m_header, 0, sizeof(m_header));
}

bool CWizZiwReader::setFile(const QString& strFileName)
{
    WIZZIWHEADER header;
    if (!loadZiwHeader(strFileName, header)) {
        TOLOG("Unable loading header");
        return false;
    }

    m_strZiwCipher.clear();
    m_strFileName = strFileName;
    memcpy(&m_header, &header, sizeof(WIZZIWHEADER));
    return true;
}

bool CWizZiwReader::loadZiwHeader(const QString& strFileName, WIZZIWHEADER& header)
{
    QFile file(strFileName);

    if (!file.open(QIODevice::ReadOnly)) {
        TOLOG("Can't open for reading while load header");
        return false;
    }

    QDataStream ds(&file);

    int szRead = ds.readRawData(reinterpret_cast<char *>(&header), sizeof(header));
    if (szRead != sizeof(header)) {
        TOLOG("file format is not correct");
        return false;
    }

    file.close();

    if (!QString("ZIWR").compare(header.szSign, Qt::CaseSensitive) \
            || !QString("ZIWA").compare(header.szSign, Qt::CaseSensitive)) {
        TOLOG("Unknown encrypt type");
        return false;
    }

    if (header.nVersion != 1) {
        TOLOG1("Unknow encrypt version: %1", WizIntToStr(header.nVersion));
        return false;
    }

    if (header.nKeyLength != WIZZIWFILE_KEY_LENGTH) {
        TOLOG("The announced key length is not correct");
        return false;
    }

    if (!QString(reinterpret_cast<char *>(header.szReserved)).isEmpty()) {
        TOLOG("The reserved field should be null, but actually it's not");
        return false;
    }

    //debugPrintHeader();
    return true;
}

bool CWizZiwReader::loadZiwData(const QString& strFileName, QByteArray& strData)
{
    QFile file(strFileName);

    if (!file.open(QIODevice::ReadOnly)) {
        TOLOG("Can't open file for reading while load data");
        return false;
    }

    file.seek(sizeof(WIZZIWHEADER));
    strData.clear();
    strData.append(file.readAll());
    file.close();

    return true;
}

ZiwEncryptType CWizZiwReader::encryptType()
{
    if (!QString("ZIWR").compare(m_header.szSign, Qt::CaseSensitive)) {
        return ZiwR;
    } else if (!QString("ZIWA").compare(m_header.szSign, Qt::CaseSensitive)) {
        return ZiwA;
    } else {
        return ZiwUnknown;
    }
}

bool CWizZiwReader::isFileAccessible(const QString& encryptedFile)
{
    if (!setFile(encryptedFile)) {
        return false;
    }

    if (!isZiwCipherAvailable() && !initZiwCipher())
        return false;

    QByteArray encryptedData, rawData;
    if (!loadZiwData(encryptedFile, encryptedData)) {
       return false;
    }

    if (!WizAESDecryptToString((const unsigned char *)(m_strZiwCipher.toUtf8().constData()), encryptedData, rawData)) {
        return false;
    }

    return true;
}

bool CWizZiwReader::isRSAKeysAvailable()
{
    return !m_N.isEmpty() && !m_e.isEmpty() && !m_d.isEmpty();
}

bool CWizZiwReader::isZiwCipherAvailable()
{
   return (!m_d.isEmpty() && !m_strZiwCipher.isEmpty());
}

bool CWizZiwReader::initZiwCipher()
{
    if (!decryptRSAdPart(m_d)) {
        return false;
    }

    QByteArray ziwCipher;
    if(!decryptZiwCipher(ziwCipher)) {
        return false;
    }

    return true;
}

bool CWizZiwReader::createZiwHeader()
{
    QString strZiwCipher = WizGenGUIDLowerCaseLetterOnly() + WizGenGUIDLowerCaseLetterOnly();
    return initZiwHeader(m_header, strZiwCipher);
}

void CWizZiwReader::setRSAKeys(const QByteArray& strN, \
                               const QByteArray& stre, \
                               const QByteArray& str_encrypted_d, \
                               const QString& strHint)
{
    m_N = strN;
    m_e = stre;
    m_encrypted_d = str_encrypted_d;
    m_strHint = strHint;
}

bool CWizZiwReader::decryptRSAdPart(QByteArray& d)
{
    Q_ASSERT(!m_encrypted_d.isEmpty());

    QByteArray rawEncrypted_d;
    WizBase64Decode(QString(m_encrypted_d), rawEncrypted_d);

    return decryptRSAdPart(rawEncrypted_d, d);
}

bool CWizZiwReader::decryptRSAdPart(const QByteArray& encrypted_d, QByteArray& d)
{
    Q_ASSERT(!m_userCipher.isEmpty());

    return WizAESDecryptToString((const unsigned char *)m_userCipher.toUtf8().constData(), encrypted_d, d);
}

bool CWizZiwReader::encryptRSAdPart(QByteArray& encrypted_d)
{
    Q_ASSERT(!m_d.isEmpty());

    QByteArray rawEncrypted_d;
    if (!encryptRSAdPart(m_d, rawEncrypted_d)) {
        return false;
    }

    QString strEncoded_d;
    WizBase64Encode(rawEncrypted_d, strEncoded_d);

    encrypted_d.clear();
    encrypted_d.append(strEncoded_d.toUtf8());

    return true;
}

bool CWizZiwReader::encryptRSAdPart(const QByteArray& d, QByteArray& encrypted_d)
{
    Q_ASSERT(!m_userCipher.isEmpty());

    return WizAESDecryptToString((const unsigned char *)m_userCipher.toUtf8().constData(), d, encrypted_d);
}

bool CWizZiwReader::encryptZiwCipher(QByteArray& encryptedZiwCipher)
{
    Q_ASSERT(!m_strZiwCipher.isEmpty());

    return encryptZiwCipher(m_strZiwCipher.toUtf8(), encryptedZiwCipher);
}

bool CWizZiwReader::encryptZiwCipher(const QByteArray& ziwCipher, QByteArray& encryptedZiwCipher)
{
    Q_ASSERT(!m_N.isEmpty());
    Q_ASSERT(!m_e.isEmpty());

    if (!WizRSAEncryptToString(m_N.constData(), \
                               m_e.constData(), \
                               ziwCipher.constData(), \
                               encryptedZiwCipher)) {
        return false;
    }

    return true;
}

bool CWizZiwReader::decryptZiwCipher(QByteArray& ziwCipher)
{
    QByteArray encryptedZiwCipher((const char *)m_header.szEncryptedKey, WIZZIWFILE_KEY_LENGTH);
    if (!decryptZiwCipher(encryptedZiwCipher, ziwCipher)) {
        return false;
    }

    m_strZiwCipher = ziwCipher;
    return true;
}

bool CWizZiwReader::decryptZiwCipher(const QByteArray& encryptedZiwCipher, QByteArray& ziwCipher)
{
    Q_ASSERT(!m_N.isEmpty());
    Q_ASSERT(!m_e.isEmpty());
    Q_ASSERT(!m_d.isEmpty());

    if (!WizRSADecryptToString(m_N.constData(), \
                               m_e.constData(), \
                               m_d.constData(), \
                               encryptedZiwCipher, ziwCipher)) {
        return false;
    }

    // FIXME: verify ziw cipher is correct match two GUID composed form

    return true;
}

bool CWizZiwReader::encryptDataToTempFile(const QString& sourceFileName, \
                                          const QString& destFileName)
{
    Q_ASSERT(!m_strZiwCipher.isEmpty());

    return encryptDataToTempFile(sourceFileName, destFileName, m_strZiwCipher);
}

bool CWizZiwReader::encryptDataToTempFile(const QString& sourceFileName, \
                                          const QString& destFileName, \
                                          const QString& strZiwCipher)
{
    QFile sourceFile(sourceFileName);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        TOLOG("Can't open source file while encrypt to temp file");
        return false;
    }

    // encrypt data
    QByteArray inBytes(sourceFile.readAll());
    QByteArray outBytes;
    if (!WizAESEncryptToString((const unsigned char *)(strZiwCipher.toUtf8().constData()), inBytes, outBytes)) {
        return false;
    }

    WIZZIWHEADER header;
    initZiwHeader(header, strZiwCipher);

    QFile destFile(destFileName);
    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        TOLOG("Can't open dest file while encrypt to temp file");
        return false;
    }

    QDataStream out(&destFile);
    if (sizeof(header) != out.writeRawData((const char *)&header, sizeof(header))) {
        TOLOG("Write data failed while encrypt to temp file");
        destFile.remove();
        return false;
    }

    if (outBytes.length() != out.writeRawData(outBytes.constData(), outBytes.length())) {
        TOLOG("Write data failed while encrypt to temp file");
        destFile.remove();
        return false;
    }

    destFile.close();
    sourceFile.close();

    return true;
}

bool CWizZiwReader::decryptDataToTempFile(const QString& tempFileName)
{
    QFile file(tempFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        TOLOG("can't open dest file while decrypt to temp file");
        return false;
    }

    if (!isZiwCipherAvailable() && !initZiwCipher())
        return false;

    QByteArray encryptedData, rawData;
    if (!loadZiwData(m_strFileName, encryptedData)) {
       return false;
    }

    if (!WizAESDecryptToString((const unsigned char *)(m_strZiwCipher.toUtf8().constData()), encryptedData, rawData)) {
        return false;
    }

    QDataStream out(&file);
    if (rawData.length() != out.writeRawData(rawData.constData(), rawData.length())) {
        TOLOG("write data failed while decrypt to temp file");
        file.remove();
        return false;
    }

    file.close();

    // clean user cipher when done
    if (!m_bSaveUserCipher) {
        m_userCipher.clear();
    }

    return true;
}


void CWizZiwReader::debugPrintHeader()
{
    TOLOG(m_header.szSign);
    TOLOG(QString::number(m_header.nVersion));
    TOLOG(QString::number(m_header.nKeyLength));

    //TOLOG(QString::fromStdString(base64_encode(m_header.szEncryptedKey, sizeof(m_header.szEncryptedKey))));

    TOLOG("---------");

    //TOLOG(QString::fromStdString(base64_encode(m_header.szReserved, sizeof(m_header.szReserved))));
}

bool CWizZiwReader::initZiwHeader(WIZZIWHEADER& header, const QString& strZiwCipher)
{
    // encrypt ziw cipher
    QByteArray encryptedZiwCipher;
    if (!encryptZiwCipher(strZiwCipher.toUtf8(), encryptedZiwCipher)) {
        return false;
    }

    // compose file
    // FIXME: hard coded here.

    memset(&header, 0, sizeof(header));
    header.szSign[0] = 'Z';
    header.szSign[1] = 'I';
    header.szSign[2] = 'W';
    header.szSign[3] = 'R';
    header.nVersion = 1;
    header.nKeyLength = WIZZIWFILE_KEY_LENGTH;
    memcpy(header.szEncryptedKey, encryptedZiwCipher.constData(), sizeof(header.szEncryptedKey));

    return true;
}
