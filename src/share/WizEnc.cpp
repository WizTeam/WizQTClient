#include "WizEnc.h"

#include <cassert>
#include <exception>

#include <QBuffer>
#include <QFile>
#include <QDebug>

#include "WizMd5.h"
#include "WizMisc.h"
#include "utils/WizLogger.h"


/**
init the enc/dec key.
@param key the enc/dec key
@param len the key length in bytes, this value can be 16, 24, 32 (128, 196, 256 bits) bytes
@param iv block size 16 bytes initializaiton vector.
*/
void WizAES::init(const char * key, int len, const unsigned char * iv)
{
    enc.SetKeyWithIV((const unsigned char *)key, len, (const unsigned char *)iv);
    dec.SetKeyWithIV((const unsigned char *)key, len, (const unsigned char *)iv);
    memcpy(this->iv, iv, 16);
}

/**
get the maximal cipher data length after encrypted.
@param len the plain data length.
@return the cipher data length.
*/
int WizAES::getCipherLen(int len)
{
    // for PKCS#1 v1.5 padding
    // max padding BLOCK_SIZE=16.
    int pad = len%16;
    if (0 == pad)
    {
        return len + 16;
    }
    return len - pad + 16;
}

bool WizAES::encrypt(QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    pStreamSrc->device()->seek(0);
    pStreamDest->device()->seek(0);

    // resynchronize with an IV
    enc.Resynchronize((const unsigned char *)iv);

    unsigned char* pBufferSrc = NULL;
    unsigned char* pBufferDest = NULL;

    bool bRet = false;

    try
    {
        int nSrcLen = pStreamSrc->device()->size();

        const unsigned int BLOCK_SIZE = 1024 * 16;
        assert(BLOCK_SIZE % 16 == 0);

        pBufferSrc = new unsigned char[BLOCK_SIZE];
        pBufferDest = new unsigned char[BLOCK_SIZE];

        if (!pBufferSrc || !pBufferDest) {
            //throw std::exception("Out of memory");
            TOLOG("Out of memory");
            return false;
        }

        int nBlockCount = nSrcLen / BLOCK_SIZE;

        for (int i = 0; i < nBlockCount; i++)
        {
            if (pStreamSrc->readRawData((char *)pBufferSrc, BLOCK_SIZE) == -1) {
                //throw std::exception("Failed to read data from stream!");
                TOLOG("Failed to read data from stream!");
                return false;
            }
            enc.ProcessData(pBufferDest, pBufferSrc, BLOCK_SIZE);
            if (pStreamDest->writeRawData((char *)pBufferDest, BLOCK_SIZE) == -1) {
                //throw std::exception("Failed to write data to stream!");
                TOLOG("Failed to write data to stream!");
                return false;
            }
        }

        int nLast = nSrcLen % BLOCK_SIZE;

        if (pStreamSrc->readRawData((char *)pBufferSrc, nLast) == -1) {
            //throw std::exception("Failed to read data from stream!");
            TOLOG("Failed to read data from stream!");
            return false;
        }

        int pad = nLast % 16;
        int prefix = nLast - pad;
        // process normal prefix blocks.
        if (prefix > 0)
        {
            enc.ProcessData(pBufferDest, pBufferSrc, prefix);
        }
        // process the last padding block.
        unsigned char padding[16];
        if (pad < 16)
        {
            memcpy(padding, pBufferSrc + prefix, pad);
        }
        memset(padding + pad, 16 - pad, 16 - pad);
        enc.ProcessLastBlock(pBufferDest + prefix, padding, 16);

        if (pStreamDest->writeRawData((char *)pBufferDest, prefix + 16) == -1) {
            //throw std::exception("Failed to write data to stream!");
            TOLOG("Failed to write data to stream!");
            return false;
        }

        bRet = true;
    }
    catch (const std::exception &err)
    {
        TOLOG(err.what());
    }

    delete [] pBufferSrc;
    delete [] pBufferDest;

    pStreamDest->device()->seek(0);
    return bRet;
}

/**
the maximal plain data length after decrypted.
@param len the cipher data length that will be decrypted.
@return the maximal plain data length.
*/
int WizAES::getPlainLen(int len)
{
    // for PKCS#1 v1.5 padding
    // len always be times of BLOCK_SIZE=16.
    return len;
}


bool WizAES::decrypt(QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    pStreamSrc->device()->seek(0);
    pStreamDest->device()->seek(0);

    // resynchronize with an IV
    dec.Resynchronize((const unsigned char *)iv);

    unsigned char* pBufferSrc = NULL;
    unsigned char* pBufferDest = NULL;

    bool bRet = false;

    try
    {
        int nSrcLen = pStreamSrc->device()->size();
        if (nSrcLen % 16 != 0)
        {
            //throw std::exception("nSrcLen % 16 != 0");
            TOLOG("nSrcLen % 16 != 0");
            return false;
        }

        const unsigned int BLOCK_SIZE = 1024 * 16;
        assert(BLOCK_SIZE % 16 == 0);

        pBufferSrc = new unsigned char[BLOCK_SIZE];
        pBufferDest = new unsigned char[BLOCK_SIZE];

        if (!pBufferSrc || !pBufferDest)  {
            TOLOG("Out of memory");
            return false;
        }

        // process normal prefix blocks.
        int prefix = nSrcLen - 16;
        if (prefix > 0)
        {
            assert(prefix % 16 == 0);

            int nBlockCount = prefix / BLOCK_SIZE;

            for (int i = 0; i < nBlockCount; i++)
            {
                if (pStreamSrc->readRawData((char *)pBufferSrc, BLOCK_SIZE) == -1) {
                    //throw std::exception("Failed to read data from stream!");
                    TOLOG("Failed to read data from stream!");
                    return false;
                }

                dec.ProcessData(pBufferDest, pBufferSrc, BLOCK_SIZE);

                if (pStreamDest->writeRawData((char *)pBufferDest, BLOCK_SIZE) == -1) {
                    //throw std::exception("Failed to write data to stream!");
                    TOLOG("Failed to read data from stream!");
                    return false;
                }
            }

            int nLast = prefix % BLOCK_SIZE;
            if (nLast > 0)
            {
                assert(nLast % 16 == 0);

                if (pStreamSrc->readRawData((char *)pBufferSrc, nLast) == -1) {
                    //throw std::exception("Failed to read data from stream!");
                    TOLOG("Failed to read data from stream!");
                    return false;
                }

                dec.ProcessData(pBufferDest, pBufferSrc, nLast);

                if (pStreamDest->writeRawData((char *)pBufferDest, nLast) == -1) {
                    //throw std::exception("Failed to write data to stream!");
                    TOLOG("Failed to read data from stream!");
                    return false;
                }
            }
        }

        unsigned char padding[16];
        if (pStreamSrc->readRawData((char *)pBufferSrc, 16) == -1) {
            //throw std::exception("Failed to read data from stream!");
            TOLOG("Failed to read data from stream!");
            return false;
        }

        dec.ProcessLastBlock(padding, pBufferSrc, 16);
        int pad = padding[15];
        if (pad > 16) {
            //throw std::exception("Failed to process last block!\nInvalid password?");
            TOLOG("Failed to process last block! Invalid password?");
            return false;
        }

        for (int i = 0; i < pad; i++)
        {
            if (padding[15 - i] != pad) {
                //throw std::exception("Padding error!\nInvalid password?");
                TOLOG("Padding error!\nInvalid password?");
                return false;
            }
        }

        if (pStreamDest->writeRawData((char *)padding, 16 - pad) == -1) {
            //throw std::exception("Failed to write data to stream!");
            TOLOG("Failed to write data to stream!");
            return false;
        }

        bRet = true;
    }
    catch (const std::exception& err)
    {
        TOLOG(err.what());
    }

    delete [] pBufferSrc;
    delete [] pBufferDest;

    pStreamDest->device()->seek(0);
    return bRet;
}


bool processKeyAESCbc(const unsigned char* lpszKey, int nKeyLen, std::string& strKey)
{
    if (!lpszKey || nKeyLen <= 0)
        return false;

    strKey = WizMd5StringNoSpaceJava(lpszKey, nKeyLen).toStdString();

    if (strKey.length() != 32) {
        return false;
    }

    return true;
}

bool encryptAES256CbcPkcs5(const unsigned char* lpszKey, int nKeyLen, \
                           const unsigned char* pIV, int nIVLen, \
                           QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    if (!pStreamSrc)
    {
        TOLOG("pStreamSrc is NULL");
        return false;
    }
    if (!pStreamDest)
    {
        TOLOG("pStreamDest is NULL");
        return false;
    }
    if (!pIV)
    {
        TOLOG("IV is NULL");
        return false;
    }
    if (nIVLen != 16)
    {
        TOLOG("Length of IV is not 16");
        return false;
    }

    std::string strKey;
    if (!processKeyAESCbc(lpszKey, nKeyLen, strKey))
    {
        TOLOG("Failed to get key");
        return false;
    }

    WizAES aes;
    aes.init(strKey.c_str(), strKey.length(), pIV);
    return aes.encrypt(pStreamSrc, pStreamDest);
}

bool decryptAES256CbcPkcs5(const unsigned char* lpszKey, int nKeyLen, \
                      const unsigned char* pIV, int nIVLen, \
                      QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    if (!pStreamSrc)
    {
        TOLOG("pStreamSrc is NULL");
        return false;
    }
    if (!pStreamDest)
    {
        TOLOG("pStreamDest is NULL");
        return false;
    }
    if (!pIV)
    {
        TOLOG("IV is NULL");
        return false;
    }
    if (nIVLen != 16)
    {
        TOLOG("Length of IV is not 16");
        return false;
    }

    std::string strKey;
    if (!processKeyAESCbc(lpszKey, nKeyLen, strKey))
    {
        TOLOG("Failed to get key");
        return false;
    }

    WizAES aes;
    aes.init(strKey.c_str(), strKey.length(), pIV);
    return aes.decrypt(pStreamSrc, pStreamDest);
}

bool simpleAESEncrypt(const unsigned char* lpszKey, QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    Q_ASSERT(pStreamSrc->device()->isOpen());
    Q_ASSERT(pStreamDest->device()->isOpen());

    const unsigned char* lpszIV = (const unsigned char*)"0123456789abcdef";
    return encryptAES256CbcPkcs5(lpszKey, (int)strlen((const char*)lpszKey), lpszIV, 16, pStreamSrc, pStreamDest);
}

bool simpleAESDecrypt(const unsigned char* lpszKey, QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    Q_ASSERT(pStreamSrc->device()->isOpen());
    Q_ASSERT(pStreamDest->device()->isOpen());

    const unsigned char* lpszIV = (const unsigned char*)"0123456789abcdef";
    return decryptAES256CbcPkcs5(lpszKey, (int)strlen((const char*)lpszKey), lpszIV, 16, pStreamSrc, pStreamDest);
}

bool WizAESEncryptToString(const unsigned char* cipher, \
                           const QByteArray& inStr, QByteArray& outStr)
{
    QBuffer ibuf, obuf;
    ibuf.setData(inStr);

    if (!ibuf.open(QIODevice::ReadOnly) || !obuf.open(QIODevice::ReadWrite)) {
        TOLOG("Can't open buffer");
        return false;
    }

    QDataStream is(&ibuf), os(&obuf);

    if (!simpleAESEncrypt(cipher, &is, &os)) {
        TOLOG("AES decrypt failed");
        return false;
    }

    outStr.clear();
    outStr.append(obuf.buffer());
    return true;
}

bool WizAESEncryptStringToBase64String(const QString& password, \
                           const QString& inStr, QString& outBase64Str)
{
    QByteArray ret;
    if (!WizAESEncryptToString((const unsigned char*)password.toUtf8().constData(), inStr.toUtf8().constData(), ret))
        return false;
    //
    WizBase64Encode(ret, outBase64Str);
    return true;
}

bool WizAESDecryptToString(const unsigned char* cipher, \
                           const QByteArray& inStr, QByteArray& outStr)
{
    QBuffer ibuf, obuf;
    ibuf.setData(inStr);

    if (!ibuf.open(QIODevice::ReadOnly) || !obuf.open(QIODevice::ReadWrite)) {
        TOLOG("Can't open buffer");
        return false;
    }

    QDataStream is(&ibuf), os(&obuf);

    if (!simpleAESDecrypt(cipher, &is, &os)) {
        TOLOG("AES decrypt failed");
        return false;
    }

    outStr.clear();
    outStr.append(obuf.buffer());
    return true;
}

bool WizAESDecryptBase64StringToString(const QString& password, \
                           const QString& inBase64Str, QString& outStr)
{
    QByteArray src;
    WizBase64Decode(inBase64Str, src);
    //
    QByteArray ret;
    if (!WizAESDecryptToString((const unsigned char*)password.toUtf8().constData(), src, ret))
        return false;
    //
    outStr = QString::fromUtf8(ret);
    return true;
}

bool WizAESEncryptToFile(const unsigned char* cipher, \
                         QDataStream* inStream, const QString& strFileName)
{
    Q_ASSERT(inStream->device()->isOpen());

    QFile file(strFileName);

    if (file.exists()) {
        TOLOG("file already exist");
        return false;
    }

    if (!file.open(QIODevice::ReadWrite)) {
        TOLOG("Can't open file");
        return false;
    }

    QDataStream os(&file);

    if(!simpleAESEncrypt(cipher, inStream, &os)) {
        TOLOG("AES decrypt failed");
        return false;
    }

    file.close();
    return true;
}

bool WizAESDecryptToFile(const unsigned char* cipher, \
                         QDataStream* inStream, const QString& strFileName)
{
    Q_ASSERT(inStream->device()->isOpen());

    QFile file(strFileName);

    if (file.exists()) {
        TOLOG("file already exist");
        return false;
    }

    if (!file.open(QIODevice::ReadWrite)) {
        TOLOG("Can't open file");
        return false;
    }

    QDataStream os(&file);

    if(!simpleAESDecrypt(cipher, inStream, &os)) {
        TOLOG("AES decrypt failed");
        return false;
    }

    file.close();
    return true;
}

//================================================================================
// CRSA encryption and decryption
//================================================================================

WizRSA::~WizRSA()
{
    if (NULL != enc)
    {
        delete enc;
        enc = NULL;
    }
    if (NULL != dec)
    {
        delete dec;
        dec = NULL;
    }
}

void WizRSA::initPublicKey(const char* N, const char* e)
{
    CryptoPP::Integer big_N(N);
    CryptoPP::Integer big_e(e);

    pk.Initialize(big_N, big_e);
    if (NULL != enc) {
        delete enc;
    }

    enc = new CryptoPP::RSAES_PKCS1v15_Encryptor(pk);
}

void WizRSA::initPrivateKey(const char* N, const char* e, const char* d)
{
    CryptoPP::Integer big_N(N);
    CryptoPP::Integer big_e(e);
    CryptoPP::Integer big_d(d);

    sk.Initialize(big_N, big_e, big_d);
    if (NULL != dec)
    {
        delete dec;
    }

    dec = new CryptoPP::RSAES_PKCS1v15_Decryptor(sk);
}

int WizRSA::getCipherLen(int len)
{
    return (int)enc->CiphertextLength(len);
}

int WizRSA::encrypt(const unsigned char* indata, int len, unsigned char* outdata)
{
    enc->Encrypt(rng, (const unsigned char *)indata, len, (unsigned char *)outdata);
    return (int)enc->FixedCiphertextLength();
}

int WizRSA::getPlainLen(int len)
{
    return (int)dec->MaxPlaintextLength(len);
}

int WizRSA::decrypt(const unsigned char* indata, int len, unsigned char* outdata)
{
    const CryptoPP::DecodingResult & res = dec->Decrypt(rng, indata, len, outdata);
    return (int)res.messageLength;
}


bool simpleRSAEncrypt(const char* N, const char* e, \
                      QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    Q_ASSERT(pStreamSrc->device()->isOpen());
    Q_ASSERT(pStreamDest->device()->isOpen());

    pStreamSrc->device()->seek(0);
    pStreamDest->device()->seek(0);

    int nSrcLen = pStreamSrc->device()->size();
    if (nSrcLen > 117) {
        TOLOG("nSrcLen > 117");
        return false;
    }

    unsigned char src[128];
    memset(src, 0, sizeof(src));

    if (pStreamSrc->readRawData((char *)src, nSrcLen) != nSrcLen) {
        TOLOG("Input stream is not correct");
        return false;
    }

    unsigned char ret[256];
    memset(ret, 0, sizeof(ret));

    WizRSA enc;
    enc.initPublicKey(N, e);

    if (enc.getCipherLen(nSrcLen) >= 256) {
        TOLOG("enc.getCipherLen(nSrcLen) >= 256");
        return false;
    }

    int nRetOlen = enc.encrypt(src, nSrcLen, ret);

    assert(nRetOlen <= 256);

    if (pStreamDest->writeRawData((const char *)ret, nRetOlen) == -1) {
        TOLOG("WizStreamWrite(pStreamDest, ret, nRetOlen) failed");
        return false;
    }

    pStreamDest->device()->seek(0);

    return true;
}

bool simpleRSADecrypt(const char* N, const char* e, const char* d, \
                      QDataStream* pStreamSrc, QDataStream* pStreamDest)
{
    Q_ASSERT(pStreamSrc->device()->isOpen());
    Q_ASSERT(pStreamDest->device()->isOpen());

    pStreamSrc->device()->seek(0);
    pStreamDest->device()->seek(0);

    int nSrcLen = pStreamSrc->device()->size();
    if (nSrcLen > 256)
    {
        TOLOG("nSrcLen > 256");
        return false;
    }

    unsigned char src[256];
    memset(src, 0, sizeof(src));

    if (pStreamSrc->readRawData((char *)src, nSrcLen) != nSrcLen) {
        TOLOG("Input stream is not correct");
        return false;
    }

    unsigned char ret[128];
    memset(ret, 0, sizeof(ret));

    WizRSA enc;
    try
    {
        enc.initPrivateKey(N, e, d);
    }
    catch (std::exception& e)
    {
        TOLOG(e.what());
        return false;
    }

    if (enc.getPlainLen(nSrcLen) > 117)
    {
        TOLOG("enc.getPlainLen(nSrcLen) > 117");
        return false;
    }

    int nRetOlen = enc.decrypt(src, nSrcLen, ret);

    assert(nRetOlen < 117);

    if (pStreamDest->writeRawData((const char *)ret, nRetOlen) == -1) {
        TOLOG("WizStreamWrite(pStreamDest, ret, nRetOlen) failed");
        return false;
    }

    pStreamDest->device()->seek(0);

    return true;
}

bool WizRSADecryptToString(const char* N, const char* e, const char* d, \
                           const QByteArray& inStr, QByteArray& outStr)
{
    QBuffer ibuf, obuf;
    ibuf.setData(inStr);

    if (!ibuf.open(QIODevice::ReadOnly) || !obuf.open(QIODevice::ReadWrite)) {
        TOLOG("Can't open buffer");
        return false;
    }

    QDataStream is(&ibuf), os(&obuf);

    if (!simpleRSADecrypt(N, e, d, &is, &os)) {
        TOLOG("RSA decrypt failed");
        return false;
    }

    outStr.clear();
    outStr.append(obuf.buffer());
    return true;
}

bool WizRSAEncryptToString(const char* N, const char* e, \
                      const QByteArray& inStr, QByteArray& outStr)
{
    QBuffer ibuf, obuf;
    ibuf.setData(inStr);

    if (!ibuf.open(QIODevice::ReadOnly) || !obuf.open(QIODevice::ReadWrite)) {
        TOLOG("Can't open buffer");
        return false;
    }

    QDataStream is(&ibuf), os(&obuf);

    if(!simpleRSAEncrypt(N, e, &is, &os)) {
        TOLOG("RSA encrypt failed");
        return false;
    }

    outStr.clear();
    outStr.append(obuf.buffer());
    return true;
}
