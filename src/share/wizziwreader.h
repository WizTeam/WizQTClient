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

class CWizZiwReader : public QObject
{
    Q_OBJECT

public:
    explicit CWizZiwReader(QObject *parent = 0);

    const QString& userCipher() const { return m_userCipher; }
    void setUserCipher(const QString& strCipher) { m_userCipher = strCipher; }

    const QString& userCipherHint() const { return m_strHint; }
    void setSaveUserCipher(bool b) { m_bSaveUserCipher = b; }

    void setRSAKeys(const QByteArray& strN, \
                    const QByteArray& stre, \
                    const QByteArray& str_encrypted_d, \
                    const QString& strHint);

    // ziw file initialization
    bool setFile(const QString& strFileName);

    // encrypt new note
    bool encryptDataToTempFile(const QString& sourceFileName, \
                               const QString& destFileName, \
                               const QString& strZiwCipher);

    // encrypt note decrypted before
    bool encryptDataToTempFile(const QString& sourceFileName, \
                               const QString& destFileName);

    // call setUserCipher and setRSAKeys before use this
    bool decryptDataToTempFile(const QString& tempFileName);

    ZiwEncryptType encryptType();

    //call setUserCipher and setRSAKeys before use this
    bool isFileAccessible(const QString& encryptedFile);

    //
    bool isRSAKeysAvailable();
    bool isZiwCipherAvailable();
    bool initZiwCipher();
    bool createZiwHeader();

private:
    QString m_strFileName;
    WIZZIWHEADER m_header;
    QString m_userCipher;
    bool m_bSaveUserCipher;
    QString m_strZiwCipher;

    QByteArray m_N;
    QByteArray m_e;
    QByteArray m_d;
    QByteArray m_encrypted_d;
    QString m_strHint;

    bool loadZiwHeader(const QString& strFileName, WIZZIWHEADER& header);
    bool loadZiwData(const QString& strFileName, QByteArray& strData);

    bool encryptZiwCipher(QByteArray& encryptedZiwCipher);
    bool encryptZiwCipher(const QByteArray& ziwCipher, QByteArray& encryptedZiwCipher);

    bool decryptZiwCipher(QByteArray& ziwCipher);
    bool decryptZiwCipher(const QByteArray& encryptedZiwCipher, QByteArray& ziwCipher);

    bool encryptRSAdPart(QByteArray& encrypted_d);
    bool encryptRSAdPart(const QByteArray& d, QByteArray& encrypted_d);

    bool decryptRSAdPart(QByteArray& d);
    bool decryptRSAdPart(const QByteArray& encrypted_d, QByteArray& d);

    void debugPrintHeader();

    bool initZiwHeader(WIZZIWHEADER& header, const QString& strZiwCipher);
};

#endif // WIZZIWREADER_H
