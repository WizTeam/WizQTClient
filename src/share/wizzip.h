#ifndef WIZZIP_H
#define WIZZIP_H

#include "../share/wizqthelper.h"
#include <QStringList>

class QuaZip;

class CWizZipFile
{
public:
    CWizZipFile();
    virtual ~CWizZipFile();
protected:
    QuaZip* m_zip;
public:
    bool open(const CString& strFileName);
    bool compressFile(const CString& strFileName, const CString& strNameInZip);
    bool close();
};


class CWizUnzipFile
{
public:
    CWizUnzipFile();
    virtual ~CWizUnzipFile();
protected:
    QuaZip* m_zip;
    QStringList m_names;
public:
    bool open(const CString& strFileName);
    int count();
    CString fileName(int index);
    int fileNameToIndex(const CString& strNameInZip);
    bool extractFile(int index, const CString& strFileName);
    bool extractFile(const CString& strNameInZip, const CString& strFileName);
    bool extractAll(const CString& strDestPath);
    bool close();

public:
    static bool extractZip(const CString& strZipFileName, const CString& strDestPath);
};


#endif //WIZZIP_H
