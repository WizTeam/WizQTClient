#ifndef WIZZIWREADER_H
#define WIZZIWREADER_H

#include <QPointer>
#include <QBuffer>

#define WIZZIWFILE_SIGN_LENGTH      4
#define WIZZIWFILE_KEY_LENGTH       128
#define WIZZIWFILE_RESERVED_LENGTH  16

// ZiwR: RSA and AES mixed encrypt method
// ZiwA: not supported currently
enum ZiwEncryptType { ZiwR, ZiwA, ZiwUnknown };

struct WIZZIWHEADER
{
    char szSign[WIZZIWFILE_SIGN_LENGTH];
    quint32 nVersion;
    quint32 nKeyLength;
    unsigned char szEncryptedKey[WIZZIWFILE_KEY_LENGTH];
    unsigned char szReserved[WIZZIWFILE_RESERVED_LENGTH];
};

class WizDatabase;

class WizZiwReader : public QObject
{
    Q_OBJECT

public:
    explicit WizZiwReader(QObject *parent = 0);
    //
    void setDatabase(WizDatabase* pDatabase) { m_pDatabase = pDatabase; }

    QString certPassword();
    QString certPasswordHint() const { return m_strHint; }

    void setRSAKeys(const QByteArray& strN, \
                    const QByteArray& stre, \
                    const QByteArray& str_encrypted_d, \
                    const QString& strHint);

    // encrypt note decrypted before
    bool encryptFileToFile(const QString& sourceFileName, const QString& destFileName);
    bool encryptDataToFile(const QByteArray& sourceData, const QString& destFileName);
    //
    // call setUserCipher and setRSAKeys before use this
    bool decryptFileToFile(const QString& strEncryptedFileName, const QString& destFileName);
    bool decryptFileToData(const QString& strEncryptedFileName, QByteArray& destData);


    //call setUserCipher and setRSAKeys before use this
    bool isFileAccessible(const QString& encryptedFile);
    //
    bool isRSAKeysAvailable();
    bool createZiwHeader();
    //
    static ZiwEncryptType encryptType(const QString& strFileName);
    static bool isEncryptedFile(const QString& fileName);

private:
    WizDatabase* m_pDatabase;

    QByteArray m_N;
    QByteArray m_e;
    QByteArray m_d;
    QByteArray m_encrypted_d;
    QString m_strHint;

    static bool loadZiwHeader(const QString& strFileName, WIZZIWHEADER& header);
    static bool loadZiwData(const QString& strFileName, QByteArray& strData);

    bool encryptZiwPassword(const QByteArray& ziwPassword, QByteArray& encryptedZiwPassword);
    bool decryptZiwPassword(const QByteArray& encryptedZiwPassword, QByteArray& ziwPassword);

    bool encryptRSAdPart(QByteArray& encrypted_d);
    bool encryptRSAdPart(const QByteArray& d, QByteArray& encrypted_d);

    bool decryptRSAdPart(QByteArray& d);
    bool decryptRSAdPart(const QByteArray& encrypted_d, QByteArray& d);

    bool initZiwHeader(WIZZIWHEADER& header, const QString& strZiwCipher);
};

#endif // WIZZIWREADER_H
