#include "WizZiwReader.h"

#include <QDataStream>
#include <QFile>

#include "WizMisc.h"
#include "WizEnc.h"
#include "utils/WizLogger.h"
#include "WizDatabase.h"


WizZiwReader::WizZiwReader(QObject *parent)
    : QObject(parent)
    , m_pDatabase(NULL)
{
}

QString WizZiwReader::certPassword()
{
    return m_pDatabase->getCertPassword();
}

bool WizZiwReader::loadZiwHeader(const QString& strFileName, WIZZIWHEADER& header)
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

    ZiwEncryptType type = ZiwUnknown;
    if (0 == strncmp("ZIWR", header.szSign, WIZZIWFILE_SIGN_LENGTH))
        type = ZiwR;
    else if (0 == strncmp("ZIWA", header.szSign, WIZZIWFILE_SIGN_LENGTH))
        type = ZiwA;
    else
        return false;

    if (header.nVersion != 1) {
        TOLOG1("Unknown encrypt version: %1", WizIntToStr(header.nVersion));
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

    return true;
}

bool WizZiwReader::loadZiwData(const QString& strFileName, QByteArray& strData)
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

ZiwEncryptType WizZiwReader::encryptType(const QString& strFileName)
{

    WIZZIWHEADER header;
    if (!loadZiwHeader(strFileName, header))
        return ZiwUnknown;
    //
    if (0 == strncmp("ZIWR", header.szSign, WIZZIWFILE_SIGN_LENGTH))
        return ZiwR;
    else if (0 == strncmp("ZIWA", header.szSign, WIZZIWFILE_SIGN_LENGTH))
        return ZiwA;
    else
        return ZiwUnknown;
}

bool WizZiwReader::isEncryptedFile(const QString& fileName)
{
    return encryptType(fileName) == ZiwR;
}


bool WizZiwReader::isFileAccessible(const QString& encryptedFile)
{
    QByteArray data;
    return decryptFileToData(encryptedFile, data);
}

bool WizZiwReader::isRSAKeysAvailable()
{
    return !m_N.isEmpty() && !m_e.isEmpty() && !m_d.isEmpty();
}

void WizZiwReader::setRSAKeys(const QByteArray& strN, \
                               const QByteArray& stre, \
                               const QByteArray& str_encrypted_d, \
                               const QString& strHint)
{
    m_N = strN;
    m_e = stre;
    m_encrypted_d = str_encrypted_d;
    m_strHint = strHint;
    m_d.clear();
}

bool WizZiwReader::decryptRSAdPart(QByteArray& d)
{
    Q_ASSERT(!m_encrypted_d.isEmpty());

    QByteArray rawEncrypted_d;
    WizBase64Decode(QString(m_encrypted_d), rawEncrypted_d);

    return decryptRSAdPart(rawEncrypted_d, d);
}

bool WizZiwReader::decryptRSAdPart(const QByteArray& encrypted_d, QByteArray& d)
{
    Q_ASSERT(!certPassword().isEmpty());

    return WizAESDecryptToString((const unsigned char *)certPassword().toUtf8().constData(), encrypted_d, d);
}

bool WizZiwReader::encryptRSAdPart(QByteArray& encrypted_d)
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

bool WizZiwReader::encryptRSAdPart(const QByteArray& d, QByteArray& encrypted_d)
{
    Q_ASSERT(!certPassword().isEmpty());

    return WizAESDecryptToString((const unsigned char *)certPassword().toUtf8().constData(), d, encrypted_d);
}

bool WizZiwReader::encryptZiwPassword(const QByteArray& ziwPassword, QByteArray& encryptedZiwPassword)
{
    Q_ASSERT(!m_N.isEmpty());
    Q_ASSERT(!m_e.isEmpty());

    if (!WizRSAEncryptToString(m_N.constData(), \
                               m_e.constData(), \
                               ziwPassword.constData(), \
                               encryptedZiwPassword)) {
        return false;
    }

    return true;
}


bool WizZiwReader::decryptZiwPassword(const QByteArray& encryptedZiwPassword, QByteArray& ziwPassword)
{
    Q_ASSERT(!m_N.isEmpty());
    Q_ASSERT(!m_e.isEmpty());
    Q_ASSERT(!m_d.isEmpty());

    if (!WizRSADecryptToString(m_N.constData(), \
                               m_e.constData(), \
                               m_d.constData(), \
                               encryptedZiwPassword, ziwPassword)) {
        return false;
    }

    return true;
}


// encrypt note decrypted before
bool WizZiwReader::encryptDataToFile(const QByteArray& sourceData, \
                           const QString& destFileName)
{
    //random password
    QString ziwPassword = WizGenGUIDLowerCaseLetterOnly() + WizGenGUIDLowerCaseLetterOnly();
    //
    // encrypt data
    QByteArray outBytes;
    if (!WizAESEncryptToString((const unsigned char *)(ziwPassword.toUtf8().constData()), sourceData, outBytes)) {
        return false;
    }

    WIZZIWHEADER header;
    initZiwHeader(header, ziwPassword);

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

    return true;
}


bool WizZiwReader::encryptFileToFile(const QString& sourceFileName, \
                                          const QString& destFileName)
{
    QFile sourceFile(sourceFileName);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        TOLOG("Can't open source file while encrypt to temp file");
        return false;
    }

    // encrypt data
    QByteArray inBytes(sourceFile.readAll());
    //
    bool ret = encryptDataToFile(inBytes, destFileName);

    sourceFile.close();

    return ret;
}

//
bool WizZiwReader::decryptFileToData(const QString& strEncryptedFileName, QByteArray& destData)
{
    if (!decryptRSAdPart(m_d))
        return false;
    //
    WIZZIWHEADER header;
    if (!loadZiwHeader(strEncryptedFileName, header)) {
        TOLOG("Unable loading header");
        return false;
    }
    //
    QByteArray ziwPassword;
    QByteArray encryptedZiwPassword((const char *)header.szEncryptedKey, WIZZIWFILE_KEY_LENGTH);
    if (!decryptZiwPassword(encryptedZiwPassword, ziwPassword)) {
        return false;
    }
    //
    QByteArray encryptedData;
    if (!loadZiwData(strEncryptedFileName, encryptedData)) {
       return false;
    }
    //
    if (!WizAESDecryptToString((const unsigned char *)(ziwPassword.constData()), encryptedData, destData)) {
        return false;
    }

    return true;
}

bool WizZiwReader::decryptFileToFile(const QString& strEncryptedFileName, const QString& destFileName)
{
    QByteArray plainData;
    if (!decryptFileToData(strEncryptedFileName, plainData))
        return false;
    //
    QFile file(destFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        TOLOG("can't open dest file while decrypt to temp file");
        return false;
    }
    //

    QDataStream out(&file);
    if (plainData.length() != out.writeRawData(plainData.constData(), plainData.length())) {
        TOLOG("write data failed while decrypt to temp file");
        file.remove();
        return false;
    }

    file.close();

    return true;
}

bool WizZiwReader::initZiwHeader(WIZZIWHEADER& header, const QString& ziwPassword)
{
    // encrypt ziw cipher
    QByteArray encryptedZiwPassword;
    if (!encryptZiwPassword(ziwPassword.toUtf8(), encryptedZiwPassword)) {
        return false;
    }

    memset(&header, 0, sizeof(header));
    header.szSign[0] = 'Z';
    header.szSign[1] = 'I';
    header.szSign[2] = 'W';
    header.szSign[3] = 'R';
    header.nVersion = 1;
    header.nKeyLength = WIZZIWFILE_KEY_LENGTH;
    memcpy(header.szEncryptedKey, encryptedZiwPassword.constData(), sizeof(header.szEncryptedKey));

    return true;
}
