#ifndef WIZTHUMBINDEX_H
#define WIZTHUMBINDEX_H

#include <QImage>

#include "cppsqlite3.h"
#include "WizObject.h"

const int nThumbnailPixmapMaxWidth = 50;

class WizThumbIndex
{
private:
    CppSQLite3DB m_dbThumb;

public:
    WizThumbIndex();
    ~WizThumbIndex();

private:
    bool initThumbDB();
    bool checkThumbTable(const CString& strTableName, const CString& strTableSQL);
    bool updateAbstract(const WIZABSTRACT& abstract, const CString& type);
    bool abstractFromGuid(const CString& guid, WIZABSTRACT& lpszAbstract,const CString& type);
    bool abstractIsExist(const CString& guid,const CString& type);

public:
    bool openThumb(const CString& strFileName, const QString& strVersion);
    void closeThumb();
    bool isThumbOpened();
    bool updatePadAbstract(const WIZABSTRACT &lpszAbstract);
    bool updateIphoneAbstract(const WIZABSTRACT &lpszAbstract);
    bool phoneAbstractFromGuid(const CString& guid, WIZABSTRACT& lpszAbstract);
    bool padAbstractFromGuid(const CString& guid, WIZABSTRACT& lpszAbstract);
    bool deleteAbstractByGuid(const CString& guid);
    bool phoneAbstractExist(const CString& guid);
    bool padAbstractExist(const CString& guid);
};


#endif //WIZTHUMBINDEX_H
