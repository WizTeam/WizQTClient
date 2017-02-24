#ifndef WIZZIP_H
#define WIZZIP_H

#include "../share/WizQtHelper.h"
#include <QStringList>
#include <QBuffer>

class QuaZip;

class WizZipFile
{
public:
    WizZipFile();
    virtual ~WizZipFile();
protected:
    QuaZip* m_zip;
public:
    bool open(const CString& strFileName);
    bool compressFile(const CString& strFileName, const CString& strNameInZip);
    bool close();
};


class WizUnzipFile
{
public:
    WizUnzipFile();
    virtual ~WizUnzipFile();
protected:
    QuaZip* m_zip;
    QStringList m_names;
    QBuffer m_buffer;
public:
    bool open(const CString& strFileName);
    bool open(const QByteArray& data);
    int count();
    CString fileName(int index);
    int fileNameToIndex(const CString& strNameInZip);
    bool extractFile(int index, const CString& strFileName);
    bool extractFile(const CString& strNameInZip, const CString& strFileName);
    bool extractAll(const CString& strDestPath);
    bool close();
    //
    bool extractFile(const CString& strNameInZip, QByteArray& data);
    bool readMainHtmlAndResources(QString& html, CWizStdStringArray& resources);

public:
    static bool extractZip(const CString& strZipFileName, const CString& strDestPath);
};


#endif //WIZZIP_H
