#ifndef WIZENC_H
#define WIZENC_H

#include "cryptopp/aes.h"
#include "cryptopp/rsa.h"
#include "cryptopp/randpool.h"
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"

#include <QDataStream>


class WizAES
{
public:
    WizAES() {}
    ~WizAES() {}

    void init(const char* key, int len, const unsigned char* iv);

    int getCipherLen(int len);
    bool encrypt(QDataStream* pStreamSrc, QDataStream* pStreamDest);

    int getPlainLen(int len);
    bool decrypt(QDataStream* pStreamSrc, QDataStream* pStreamDest);

private:
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;   // cryptopp implement aes CBC encryptor.
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;   // cryptopp implement aes CBC decryptor.
    char iv[16];   // initialization vector.

};

bool processKeyAESCbc(const unsigned char* lpszKey, int nKeyLen, std::string& strKey);

bool encryptAES256CbcPkcs5(const unsigned char* lpszKey, int nKeyLen, \
                           const unsigned char* pIV, int nIVLen, \
                           QDataStream* pStreamSrc, QDataStream* pStreamDest);

bool decryptAES256CbcPkcs5(const unsigned char* lpszKey, int nKeyLen, \
                           const unsigned char* pIV, int nIVLen, \
                           QDataStream* pStreamSrc, QDataStream* pStreamDest);

bool simpleAESEncrypt(const unsigned char* lpszKey, QDataStream* pStreamSrc, QDataStream* pStreamDest);
bool simpleAESDecrypt(const unsigned char* lpszKey, QDataStream* pStreamSrc, QDataStream* pStreamDest);

// AES decryption call
bool WizAESEncryptToString(const unsigned char* cipher, \
                           const QByteArray& inStr, QByteArray& outStr);

bool WizAESEncryptStringToBase64String(const QString& password, \
                           const QString& inStr, QString& outBase64Str);

bool WizAESDecryptToString(const unsigned char* cipher, \
                           const QByteArray& inStr, QByteArray& outStr);

bool WizAESDecryptBase64StringToString(const QString& password, \
                           const QString& inBase64Str, QString& outStr);

bool WizAESEncryptToFile(const unsigned char* cipher, \
                         QDataStream* inStream, const QString& strFileName);

bool WizAESDecryptToFile(const unsigned char* cipher, \
                         QDataStream* inStream, const QString& strFileName);

class WizRSA
{
public:
    WizRSA() { enc = 0; dec = 0; }
    ~WizRSA();

    void initPublicKey(const char * N, const char * e);
    void initPrivateKey(const char * N, const char * e, const char * d);

    int getCipherLen(int len);
    int encrypt(const unsigned char * indata, int len, unsigned char * outdata);

    int getPlainLen(int len);
    int decrypt(const unsigned char* indata, int len, unsigned char* outdata);

private:
    CryptoPP::RSAFunction pk;  // public key.
    CryptoPP::InvertibleRSAFunction sk;    // private key.
    CryptoPP::RSAES_PKCS1v15_Encryptor * enc;    // encryptor.
    CryptoPP::RSAES_PKCS1v15_Decryptor * dec;    // decryptor.
    CryptoPP::AutoSeededRandomPool rng;        // auto seeded randomor.
};

bool simpleRSAEncrypt(const char* N, const char* e, \
                      QDataStream* pStreamSrc, QDataStream* pStreamDest);

bool simpleRSADecrypt(const char* N, const char* e, const char* d, \
                      QDataStream* pStreamSrc, QDataStream* pStreamDest);

// RSA encryption call
bool WizRSAEncryptToString(const char* N, const char* e, \
                      const QByteArray& inStr, QByteArray& outStr);

// RSA decryption call
bool WizRSADecryptToString(const char* N, const char* e, const char* d, \
                           const QByteArray& inStr, QByteArray& outStr);


#endif // WIZENC_H
