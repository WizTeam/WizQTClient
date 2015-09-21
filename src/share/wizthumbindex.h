#ifndef WIZTHUMBINDEX_H
#define WIZTHUMBINDEX_H

#include <QImage>

#include "cppsqlite3.h"
#include "wizobject.h"

const int nThumbnailPixmapMaxWidth = 50;

class CThumbIndex
{
private:
    CppSQLite3DB m_dbThumb;

public:
    CThumbIndex();
    ~CThumbIndex();

private:
    bool InitThumbDB();
    bool checkThumbTable(const CString& strTableName, const CString& strTableSQL);
    bool UpdateAbstract(const WIZABSTRACT& abstract, const CString& type);
    bool AbstractFromGUID(const CString& guid, WIZABSTRACT& lpszAbstract,const CString& type);
    bool AbstractIsExist(const CString& guid,const CString& type);

public:
    bool OpenThumb(const CString& strFileName, const QString& strVersion);
    void CloseThumb();
    bool IsThumbOpened();
    bool UpdatePadAbstract(const WIZABSTRACT &lpszAbstract);
    bool UpdateIphoneAbstract(const WIZABSTRACT &lpszAbstract);
    bool PhoneAbstractFromGUID(const CString& guid, WIZABSTRACT& lpszAbstract);
    bool PadAbstractFromGUID(const CString& guid, WIZABSTRACT& lpszAbstract);
    bool DeleteAbstractByGUID(const CString& guid);
    bool PhoneAbstractExist(const CString& guid);
    bool PadAbstractExist(const CString& guid);
};


#endif //WIZTHUMBINDEX_H
