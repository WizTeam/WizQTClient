#include "WizZip.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QSaveFile>
#include <QBuffer>
#include <QTextStream>
#include <QDebug>

#include "utils/WizMisc.h"
#include "utils/WizLogger.h"

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "quazip/quazipfileinfo.h"


class JlCompress {
public:
    static QuaZip* openWriteonlyZip(QString fileName);
    static QuaZip* openReadonlyZip(QString fileName);
    static QuaZip* openReadonlyZip(QIODevice* device);
    static bool closeZip(QuaZip* pzip);

    static bool compressFile(QuaZip* zip, QString fileName, QString fileDest);
    static bool compressFile(QuaZip* zip, const QByteArray& data, QString fileDest);
    static bool compressSubDir(QuaZip* parentZip, QString dir, QString parentDir, bool recursive = true);
    static bool extractFile(QuaZip* zip, QString fileName, QString fileDest);
    //
    static bool extractText(QuaZip* zip, QString fileName, QString& text);
    static bool extractFile(QuaZip* zip, QString fileName, QByteArray& data);

    static bool removeFile(QStringList listFile);

public:
    static bool compressFile(QString fileCompressed, QString file);
    static bool compressFiles(QString fileCompressed, QStringList files);
    static bool compressDir(QString fileCompressed, QString dir = QString(), bool recursive = true);

public:
    static QString extractFile(QString fileCompressed, QString file, QString fileDest = QString());
    static QStringList extractFiles(QString fileCompressed, QStringList files, QString dir = QString());
    static QStringList extractDir(QString fileCompressed, QString dir = QString());
    static QStringList getFileList(QString fileCompressed);
};



static bool copyData(QIODevice &inFile, QIODevice &outFile)
{
    char* buf = new char[4096];
    while (!inFile.atEnd()) {
        memset(buf, 0, 4096);
        qint64 readLen = inFile.read(buf, 4096);
        if (readLen <= 0) {
            TOLOG1("Failed to read file: %1", inFile.errorString());
            delete [] buf;
            return false;
        }
        if (outFile.write(buf, readLen) != readLen) {
            TOLOG1("Failed to write file: %1", outFile.errorString());
            delete [] buf;
            return false;
        }
    }
    delete [] buf;
    return true;
}

/**OK
 * Comprime il file fileName, nell'oggetto zip, con il nome fileDest.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip e stato aperto in una modalita non compatibile con l'aggiunta di file;
 * * non e possibile aprire il file d'origine;
 * * non e possibile creare il file all'interno dell'oggetto zip;
 * * si e rilevato un errore nella copia dei dati;
 * * non e stato possibile chiudere il file all'interno dell'oggetto zip;
 */
bool JlCompress::compressFile(QuaZip* zip, QString fileName, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // fileName: nome del file reale
    // fileDest: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdCreate &&
        zip->getMode()!=QuaZip::mdAppend &&
        zip->getMode()!=QuaZip::mdAdd) return false;

    // Apro il file originale
    QFile inFile;
    inFile.setFileName(fileName);
    if(!inFile.open(QIODevice::ReadOnly)) return false;

    // Apro il file risulato
    QuaZipFile outFile(zip);
    if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest, inFile.fileName()))) return false;

    // Copio i dati
    if (!copyData(inFile, outFile) || outFile.getZipError()!=UNZ_OK) {
        return false;
    }

    // Chiudo i file
    outFile.close();
    if (outFile.getZipError()!=UNZ_OK) return false;
    inFile.close();

    return true;
}

bool JlCompress::compressFile(QuaZip* zip, const QByteArray& data, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // fileName: nome del file reale
    // fileDest: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdCreate &&
        zip->getMode()!=QuaZip::mdAppend &&
        zip->getMode()!=QuaZip::mdAdd) return false;

    // Apro il file originale
    QBuffer inFile;
    inFile.setData(data);
    if(!inFile.open(QIODevice::ReadOnly)) return false;

    // Apro il file risulato
    QuaZipFile outFile(zip);
    if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest))) return false;

    // Copio i dati
    if (!copyData(inFile, outFile) || outFile.getZipError()!=UNZ_OK) {
        return false;
    }

    // Chiudo i file
    outFile.close();
    if (outFile.getZipError()!=UNZ_OK) return false;
    inFile.close();
    //
    return true;

}


/**OK
 * Comprime la cartella dir nel file fileCompressed, se recursive e true allora
 * comprime anche le sotto cartelle. I nomi dei file preceduti dal path creato
 * togliendo il pat della cartella origDir al path della cartella dir.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip e stato aperto in una modalita non compatibile con l'aggiunta di file;
 * * la cartella dir non esiste;
 * * la compressione di una sotto cartella fallisce (1);
 * * la compressione di un file fallisce;
 * (1) La funzione si richiama in maniera ricorsiva per comprimere le sotto cartelle
 * dunque gli errori di compressione di una sotto cartella sono gli stessi di questa
 * funzione.
 */
bool JlCompress::compressSubDir(QuaZip* zip, QString dir, QString origDir, bool recursive) {
    // zip: oggetto dove aggiungere il file
    // dir: cartella reale corrente
    // origDir: cartella reale originale
    // (path(dir)-path(origDir)) = path interno all'oggetto zip

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdCreate &&
        zip->getMode()!=QuaZip::mdAppend &&
        zip->getMode()!=QuaZip::mdAdd) return false;

    // Controllo la cartella
    QDir directory(dir);
    if (!directory.exists()) return false;

    // Se comprimo anche le sotto cartelle
    if (recursive) {
        // Per ogni sotto cartella
        QFileInfoList files = directory.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot);
        Q_FOREACH (QFileInfo file, files) {
            // Comprimo la sotto cartella
            if(!compressSubDir(zip,file.absoluteFilePath(),origDir,recursive)) return false;
        }
    }

    // Per ogni file nella cartella
    QFileInfoList files = directory.entryInfoList(QDir::Files);
    QDir origDirectory(origDir);
    Q_FOREACH (QFileInfo file, files) {
        // Se non e un file o e il file compresso che sto creando
        if(!file.isFile()||file.absoluteFilePath()==zip->getZipName()) continue;

        // Creo il nome relativo da usare all'interno del file compresso
        QString filename = origDirectory.relativeFilePath(file.absoluteFilePath());

        // Comprimo il file
        if (!compressFile(zip,file.absoluteFilePath(),filename)) return false;
    }

    return true;
}

/**OK
 * Estrae il file fileName, contenuto nell'oggetto zip, con il nome fileDest.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato di estrarre.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip e stato aperto in una modalita non compatibile con l'estrazione di file;
 * * non e possibile aprire il file all'interno dell'oggetto zip;
 * * non e possibile creare il file estratto;
 * * si e rilevato un errore nella copia dei dati (1);
 * * non e stato possibile chiudere il file all'interno dell'oggetto zip (1);
 *
 * (1): prima di uscire dalla funzione cancella il file estratto.
 */
bool JlCompress::extractFile(QuaZip* zip, QString fileName, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // filename: nome del file reale
    // fileincompress: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdUnzip) return false;

    // Apro il file compresso
    zip->setCurrentFile(fileName);
    QuaZipFile inFile(zip);
    if(!inFile.open(QIODevice::ReadOnly) || inFile.getZipError()!=UNZ_OK) return false;
    //
    int uncompressedSize = inFile.usize();

    // Controllo esistenza cartella file risultato
    QDir().mkpath(QFileInfo(fileDest).absolutePath());

    // Apro il file risultato
    if (QFile::exists(fileDest))
    {
        qDebug() << "delete exists file.";
        QFile::remove(fileDest);
    }
    QSaveFile outFile;
    outFile.setFileName(fileDest);
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

        outFile.setPermissions(QFile::ReadOwner
                               | QFile::WriteOwner
                               | QFile::ReadUser
                               | QFile::WriteUser
                               | QFile::ReadGroup
                               | QFile::ReadOther
                               );
        //
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return false;
    }

    // Copio i dati
    if (!copyData(inFile, outFile) || inFile.getZipError()!=UNZ_OK) {
        TOLOG1("failed to copy zip file data, %1", WizIntToStr(inFile.getZipError()));
        removeFile(QStringList(fileDest));
        return false;
    }

    // Chiudo i file
    inFile.close();
    if (inFile.getZipError()!=UNZ_OK) {
        removeFile(QStringList(fileDest));
        return false;
    }
    if (!outFile.commit())
    {
        qDebug() << "failed to save zip file: " << fileDest;
        return false;
    }
    //
#ifdef QT_DEBUG
    int len = Utils::WizMisc::getFileSize(fileDest);
    qDebug() << "file len after unzip: " << len;
    if (len != uncompressedSize)
    {
        qDebug() << "uncompressed size: " << uncompressedSize << ", but file size: " << len;
    }
#endif

    return true;
}

bool JlCompress::extractText(QuaZip* zip, QString fileName, QString& text)
{
    // zip: oggetto dove aggiungere il file
    // filename: nome del file reale
    // fileincompress: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdUnzip) return false;

    // Apro il file compresso
    zip->setCurrentFile(fileName);
    QuaZipFile inFile(zip);
    if(!inFile.open(QIODevice::ReadOnly) || inFile.getZipError()!=UNZ_OK) return false;

    // Apro il file risultato
    QByteArray ba;
    QBuffer buffer(&ba);
    if (!buffer.open(QIODevice::WriteOnly))
        return false;
    // Copio i dati
    if (!copyData(inFile, buffer) || inFile.getZipError()!=UNZ_OK) {
        return false;
    }

    // Chiudo i file
    inFile.close();
    if (inFile.getZipError()!=UNZ_OK) {
        return false;
    }
    //
    buffer.close();
    //
    QTextStream stream(buffer.data());
    text = stream.readAll();
    //
    return true;
}

bool JlCompress::extractFile(QuaZip* zip, QString fileName, QByteArray& data)
{
    // zip: oggetto dove aggiungere il file
    // filename: nome del file reale
    // fileincompress: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip) return false;
    if (zip->getMode()!=QuaZip::mdUnzip) return false;

    // Apro il file compresso
    zip->setCurrentFile(fileName);
    QuaZipFile inFile(zip);
    if(!inFile.open(QIODevice::ReadOnly) || inFile.getZipError()!=UNZ_OK) return false;

    // Apro il file risultato
    QBuffer buffer(&data);
    if (!buffer.open(QIODevice::WriteOnly))
        return false;
    // Copio i dati
    if (!copyData(inFile, buffer) || inFile.getZipError()!=UNZ_OK) {
        return false;
    }

    // Chiudo i file
    inFile.close();
    if (inFile.getZipError()!=UNZ_OK) {
        return false;
    }
    //
    buffer.close();
    //
    return true;
}



/**
 * Rimuove i file il cui nome e specificato all'interno di listFile.
 * Restituisce true se tutti i file sono stati cancellati correttamente, attenzione
 * perche puo restituire false anche se alcuni file non esistevano e si e tentato
 * di cancellarli.
 */
bool JlCompress::removeFile(QStringList listFile) {
    bool ret = true;
    // Per ogni file
    for (int i=0; i<listFile.count(); i++) {
        // Lo elimino
        ret = ret && QFile::remove(listFile.at(i));
    }
    return ret;
}

QuaZip* JlCompress::openWriteonlyZip(QString fileCompressed)
{
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip->open(QuaZip::mdCreate)) {
        delete zip;
        QFile::remove(fileCompressed);
        return NULL;
    }
    //
    return zip;
}

QuaZip* JlCompress::openReadonlyZip(QString fileCompressed)
{
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return NULL;
    }
    //
    return zip;
}

QuaZip* JlCompress::openReadonlyZip(QIODevice* device)
{
    QuaZip* zip  = new QuaZip(device);
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return NULL;
    }
    //
    return zip;
}


bool JlCompress::closeZip(QuaZip* pzip)
{
    if (!pzip)
        return false;
    //
    pzip->close();
    //
    int ret = pzip->getZipError();
    //
    delete pzip;
    //
    return 0 == ret;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**OK
 * Comprime il file fileName nel file fileCompressed.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione del file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool JlCompress::compressFile(QString fileCompressed, QString file) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip->open(QuaZip::mdCreate)) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Aggiungo il file
    if (!compressFile(zip,file,QFileInfo(file).fileName())) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }
    delete zip;

    return true;
}

/**OK
 * Comprime i file specificati in files nel file fileCompressed.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool JlCompress::compressFiles(QString fileCompressed, QStringList files) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip->open(QuaZip::mdCreate)) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Comprimo i file
    QFileInfo info;
    Q_FOREACH (QString file, files) {
        info.setFile(file);
        if (!info.exists() || !compressFile(zip,file,info.fileName())) {
            delete zip;
            QFile::remove(fileCompressed);
            return false;
        }
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }
    delete zip;

    return true;
}

/**OK
 * Comprime la cartella dir nel file fileCompressed, se recursive e true allora
 * comprime anche le sotto cartelle.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool JlCompress::compressDir(QString fileCompressed, QString dir, bool recursive) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if(!zip->open(QuaZip::mdCreate)) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Aggiungo i file e le sotto cartelle
    if (!compressSubDir(zip,dir,dir,recursive)) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        QFile::remove(fileCompressed);
        return false;
    }
    delete zip;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**OK
 * Estrae il file fileName, contenuto nel file fileCompressed, con il nome fileDest.
 * Se fileDest = "" allora il file viene estratto con lo stesso nome con cui e
 * stato compresso.
 * Se la funzione fallisce cancella il file che si e tentato di estrarre.
 * Restituisce il nome assoluto del file estratto.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * l'estrazione del file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QString JlCompress::extractFile(QString fileCompressed, QString fileName, QString fileDest) {
    // Apro lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QString();
    }

    // Estraggo il file
    if (fileDest.isEmpty()) fileDest = fileName;
    if (!extractFile(zip,fileName,fileDest)) {
        delete zip;
        return QString();
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        removeFile(QStringList(fileDest));
        return QString();
    }
    delete zip;

    return QFileInfo(fileDest).absoluteFilePath();
}

/**OK
 * Estrae i file specificati in files, contenuti nel file fileCompressed, nella
 * cartella dir. La struttura a cartelle del file compresso viene rispettata.
 * Se dir = "" allora il file viene estratto nella cartella corrente.
 * Se la funzione fallisce cancella i file che si e tentato di estrarre.
 * Restituisce i nomi assoluti dei file estratti.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * l'estrazione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList JlCompress::extractFiles(QString fileCompressed, QStringList files, QString dir) {
    // Creo lo zip
    QuaZip* zip  = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QStringList();
    }

    // Estraggo i file
    for (int i=0; i<files.count(); i++) {
        if (!extractFile(zip, files.at(i), QDir(dir).absoluteFilePath(files.at(i)))) {
            delete zip;
            removeFile(files);
            return QStringList();
        }
        files[i] = QDir(dir).absoluteFilePath(files.at(i));
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        removeFile(files);
        return QStringList();
    }
    delete zip;

    return files;
}

/**OK
 * Estrae il file fileCompressed nella cartella dir.
 * Se dir = "" allora il file viene estratto nella cartella corrente.
 * Se la funzione fallisce cancella i file che si e tentato di estrarre.
 * Restituisce i nomi assoluti dei file estratti.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList JlCompress::extractDir(QString fileCompressed, QString dir) {
    // Apro lo zip
    QuaZip* zip = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        TOLOG1("Failed to open zip file: %1", WizIntToStr(zip->getZipError()));
        delete zip;
        return QStringList();
    }

    // Estraggo i file
    QStringList lst = getFileList(fileCompressed);

    QDir directory(dir);
    for (int i=0; i<lst.count(); i++) {
        QString fileName = lst.at(i);
        if (fileName.endsWith('/') || fileName.endsWith('\\'))
            continue;
        QString absFilePath = directory.absoluteFilePath(fileName);
        if (!extractFile(zip, fileName, absFilePath)) {
            TOLOG2("Failed to extract zip file: %1, %2", WizIntToStr(zip->getZipError()), fileName);
            delete zip;
            removeFile(lst);
            return QStringList();
        }
        lst[i] = absFilePath;
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        TOLOG1("Failed to extract zip file: %1", WizIntToStr(zip->getZipError()));
        delete zip;
        removeFile(lst);
        return QStringList();
    }
    delete zip;

    return lst;
}

/**OK
 * Restituisce la lista dei file resenti nel file compresso fileCompressed.
 * Se la funzione fallisce, restituisce un elenco vuoto.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la richiesta di informazioni di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList JlCompress::getFileList(QString fileCompressed) {
    // Apro lo zip
    QuaZip* zip = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    if(!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QStringList();
    }

    // Estraggo i nomi dei file
    QStringList lst;
    QuaZipFileInfo info;
    for(bool more=zip->goToFirstFile(); more; more=zip->goToNextFile()) {
      if(!zip->getCurrentFileInfo(&info)) {
          delete zip;
          return QStringList();
      }
      lst << info.name;
      //info.name.toLocal8Bit().constData()
    }

    // Chiudo il file zip
    zip->close();
    if(zip->getZipError()!=0) {
        delete zip;
        return QStringList();
    }
    delete zip;

    return lst;
}



////////////////////////////////////////////////////////////////////////////////

WizZipFile::WizZipFile()
    : m_zip(NULL)
{

}

WizZipFile::~WizZipFile()
{
    close();
}

bool WizZipFile::open(const CString& strFileName)
{
    close();
    //
    m_zip = JlCompress::openWriteonlyZip(strFileName);
    return m_zip ? true : false;
}

bool WizZipFile::compressFile(const CString& strFileName, const CString& strNameInZip)
{
    if (!m_zip)
        return false;
    //
    return JlCompress::compressFile(m_zip, strFileName, strNameInZip);
}

bool WizZipFile::compressFile(const QByteArray& data, const CString& strNameInZip)
{
    if (!m_zip)
        return false;
    //
    return JlCompress::compressFile(m_zip, data, strNameInZip);
}

bool WizZipFile::close()
{
    if (!m_zip)
        return false;
    //
    bool ret = JlCompress::closeZip(m_zip);
    //
    m_zip = NULL;
    //
    return ret;
}



WizUnzipFile::WizUnzipFile()
    : m_zip(NULL)
{

}

WizUnzipFile::~WizUnzipFile()
{
    close();
}

bool WizUnzipFile::open(const CString& strFileName)
{
    close();
    //
    m_zip = ::JlCompress::openReadonlyZip(strFileName);
    if (m_zip)
    {
        m_names = m_zip->getFileNameList();
    }
    //
    return m_zip ? true : false;
}

bool WizUnzipFile::open(const QByteArray &data)
{
    close();
    //
    m_buffer.setData(data);
    if (!m_buffer.open(QIODevice::ReadOnly))
        return false;
    //
    m_zip = JlCompress::openReadonlyZip(&m_buffer);
    if (m_zip)
    {
        m_names = m_zip->getFileNameList();
    }
    //
    return m_zip ? true : false;
}

int WizUnzipFile::count()
{
    if (!m_zip)
        return -1;
    //
    return m_names.count();
}

CString WizUnzipFile::fileName(int index)
{
    if (!m_zip)
        return CString();
    //
    if (index < 0 || index >= count())
        return CString();
    //
    return m_names[index];
}
int WizUnzipFile::fileNameToIndex(const CString& strNameInZip)
{
    if (!m_zip)
        return -1;
    //
    return m_names.indexOf(strNameInZip);
}

bool WizUnzipFile::extractFile(int index, const CString& strFileName)
{
    if (!m_zip)
        return false;
    //
    if (index < 0 || index >= count())
        return false;
    //
    return JlCompress::extractFile(m_zip, fileName(index), strFileName);
}

bool WizUnzipFile::extractFile(const CString& strNameInZip, const CString& strFileName)
{
    if (!m_zip)
        return false;
    //
    return JlCompress::extractFile(m_zip, strNameInZip, strFileName);
}

bool WizUnzipFile::extractFile(const CString& strNameInZip, QByteArray& data)
{
    if (!m_zip)
        return false;
    //
    return JlCompress::extractFile(m_zip, strNameInZip, data);
}

bool WizUnzipFile::extractFile(int index, QByteArray& data)
{
    if (!m_zip)
        return false;
    //
    QString fileName = m_names[index];
    //
    return JlCompress::extractFile(m_zip, fileName, data);
}


bool WizUnzipFile::extractAll(const CString& strDestPath)
{
    if (!m_zip)
        return false;
    //
    int succeeded = 0;
    //
    QDir directory(strDestPath);
    for (int i = 0; i < m_names.count(); i++) {
        QString absFilePath = directory.absoluteFilePath(m_names.at(i));
        if (JlCompress::extractFile(m_zip, m_names.at(i), absFilePath)) {
            succeeded++;
        }
    }
    return succeeded > 0;
}

bool WizUnzipFile::readResources(CWizStdStringArray& resources)
{
    for (int i = 0; i < m_names.count(); i++) {
        //
        QString name = m_names[i];
        if (name.toLower() == "index.html")
            continue;
        //
        name = Utils::WizMisc::extractFileName(name);
        //
        resources.push_back(name);
    }
    //
    return true;
}

bool WizUnzipFile::readMainHtmlAndResources(QString& html, CWizStdStringArray& resources)
{
    //
    if (!JlCompress::extractText(m_zip, "index.html", html))
    {
        qDebug() << "can't extract index.html";
        return false;
    }
    //
    for (int i = 0; i < m_names.count(); i++) {
        //
        QString name = m_names[i];
        if (name.toLower() == "index.html")
            continue;
        //
        name = Utils::WizMisc::extractFileName(name);
        //
        resources.push_back(name);
    }
    //
    return true;
}

bool WizUnzipFile::readMainHtmlAndResources(QString& html, std::vector<WIZZIPENTRYDATA>& resources)
{
    if (!JlCompress::extractText(m_zip, "index.html", html))
    {
        qDebug() << "can't extract index.html";
        return false;
    }
    //
    QList<QuaZipFileInfo> infos = m_zip->getFileInfoList();
    for (auto info : infos)
    {
        if (info.name.toLower() == "index.html")
            continue;
        //
        WIZZIPENTRYDATA data;
        data.name = Utils::WizMisc::extractFileName(info.name);
        data.time = info.dateTime;
        data.size = info.uncompressedSize;
        resources.push_back(data);
    }
    //
    return true;
}



bool WizUnzipFile::close()
{
    if (m_buffer.isOpen())
    {
        m_buffer.close();
    }
    //
    if (!m_zip)
        return false;
    //
    m_names.clear();;
    //
    bool ret = JlCompress::closeZip(m_zip);
    //
    m_zip = NULL;
    //
    return ret;
}

bool WizUnzipFile::extractZip(const CString& strZipFileName, const CString& strDestPath)
{
    QStringList sl = JlCompress::extractDir(strZipFileName, strDestPath);
    if (sl.empty()) {
        TOLOG2("no files extracted: %1, %2", strZipFileName, strDestPath);
        return false;
    }
    return !sl.empty();
}

