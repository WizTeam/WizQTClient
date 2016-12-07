#include "WizThumbIndex.h"

#include <QBuffer>
#include <QDebug>

#include "WizDef.h"
#include "WizMisc.h"
#include "WizIndex.h"
#include "utils/WizLogger.h"

static const char* PAD_TYPE = "PAD";
static const char* PHONE_TYPE = "PHONE";

static const char* ABSTRACT_TABLE_SQL = "CREATE TABLE WIZ_ABSTRACT (\n\
ABSTRACT_GUID                  char(36)                       not null,\n\
ABSTRACT_TYPE                  varchar(50)                     not null ,\n\
ABSTRACT_TEXT                  varchar(3000)                    ,\n\
ABSTRACT_IMAGE                  blob   ,\n\
primary key (ABSTRACT_GUID, ABSTRACT_TYPE)\n\
);";

static const char* FIELD_LIST_ABSTRACT = "ABSTRACT_GUID, ABSTRACT_TYPE, ABSTRACT_TEXT, ABSTRACT_IMAGE";

static const char* TABLE_NAME_ABSTRACT = "WIZ_ABSTRACT";

WizThumbIndex::WizThumbIndex()
{
    
}

WizThumbIndex::~WizThumbIndex()
{
    closeThumb();
}

bool WizThumbIndex::openThumb(const CString& strFileName, const QString& strVersion)
{
    if (m_dbThumb.isOpened())
		return true;

	try {
        m_dbThumb.open(strFileName);

        // need rebuild thumb index
        if (strVersion.isEmpty() || strVersion.toInt() < QString(WIZNOTE_THUMB_VERSION).toInt()) {
            qDebug() << "Thumb update triggered...";
            if (m_dbThumb.tableExists("WIZ_ABSTRACT")) {
                m_dbThumb.execDML("drop table WIZ_ABSTRACT");
            }
        }

        if (!initThumbDB())
			return false;

		return true;
	}
	catch (const CppSQLite3Exception& e)
	{
		TOLOG(e.errorMessage());
		return false;
	}
	catch (...) {
		TOLOG("Unknown exception while close DB");
		return false;
	}
}

void WizThumbIndex::closeThumb()
{
    if (!m_dbThumb.isOpened())
		return;

	try {
        m_dbThumb.close();
	}
	catch (const CppSQLite3Exception& e)
	{
		TOLOG(e.errorMessage());
	}
	catch (...) {
		TOLOG("Unknown exception while close DB");
	}
}

bool WizThumbIndex::isThumbOpened()
{
    return m_dbThumb.isOpened();
}

bool  WizThumbIndex::checkThumbTable(const CString& strTableName, const CString& strTableSQL)
{
    if (m_dbThumb.tableExists(strTableName))
		return true;

	try {

        m_dbThumb.execDML(strTableSQL);
		return true;
	}
	catch (const CppSQLite3Exception& e)
	{
		TOLOG(e.errorMessage());
		return false;
	}
	catch (...) {
		TOLOG("Unknown exception while close DB");
		return false;
	}
}

bool WizThumbIndex::initThumbDB()
{
    if (!m_dbThumb.isOpened())
		return false;

    if (!checkThumbTable(TABLE_NAME_ABSTRACT, ABSTRACT_TABLE_SQL))
        return false;

	return true;
}

bool WizThumbIndex::padAbstractFromGuid(const CString& guid, WIZABSTRACT &abstract)
{
    return abstractFromGuid(guid, abstract, PAD_TYPE);
}

bool WizThumbIndex::phoneAbstractFromGuid(const CString& guid, WIZABSTRACT &abstract)
{
    return abstractFromGuid(guid, abstract, PHONE_TYPE);
}

bool WizThumbIndex::abstractFromGuid(const CString& guid, WIZABSTRACT &abstract,const CString& type)
{
    if(!m_dbThumb.isOpened())
        return false;

    CString sql = CString("select ") + FIELD_LIST_ABSTRACT + " from " + TABLE_NAME_ABSTRACT +" where ABSTRACT_GUID='"
                    + guid + ("' AND ABSTRACT_TYPE=")
                    + STR2SQL(type)
                    + (";");
    try
    {
        CppSQLite3Query query = m_dbThumb.execQuery(sql);

        while (!query.eof())
        {
            abstract.guid = query.getStringField(0);
            abstract.text = query.getStringField(2);
            int length;
            const unsigned char * imageData = query.getBlobField(3, length);
            if (imageData && length)
            {
                abstract.image.loadFromData(imageData, length);
            }
            return true;
        }
		return false;
	}
	catch (const CppSQLite3Exception& e)
	{
		TOLOG(e.errorMessage());
        TOLOG(sql);
		return false;
	}
	catch (...) {
		TOLOG("Unknown exception while close DB");
		return false;
	}
}

bool WizThumbIndex::updatePadAbstract(const WIZABSTRACT &abstract)
{
    return updateAbstract(abstract, PAD_TYPE);
}

bool WizThumbIndex::updateIphoneAbstract(const WIZABSTRACT &abstract)
{
    return updateAbstract(abstract, PHONE_TYPE);
}

bool WizThumbIndex::updateAbstract(const WIZABSTRACT &abstractNew, const CString& type)
{
    if(!m_dbThumb.isOpened()) {
        TOLOG("Fault error: thumb database does not opened");
        return false;
    }

    QByteArray data;
    if (abstractNew.image.width() > 0 && abstractNew.image.height() > 0)
    {
        int width = abstractNew.image.width();
        int height = abstractNew.image.height();

        // process thumbnail
        QImage img;
        if (width >= nThumbnailPixmapMaxWidth && height >= nThumbnailPixmapMaxWidth) {
            if (width > height) {
                img = abstractNew.image.scaledToHeight(nThumbnailPixmapMaxWidth, Qt::SmoothTransformation);
                img = img.copy((img.width() - nThumbnailPixmapMaxWidth)/2, 0, nThumbnailPixmapMaxWidth, nThumbnailPixmapMaxWidth);
            } else {
                img = abstractNew.image.scaledToWidth(nThumbnailPixmapMaxWidth, Qt::SmoothTransformation);
                img = img.copy(0, (img.height() - nThumbnailPixmapMaxWidth)/2, nThumbnailPixmapMaxWidth, nThumbnailPixmapMaxWidth);
            }
        } else if (width > nThumbnailPixmapMaxWidth || height > nThumbnailPixmapMaxWidth) {
            img = abstractNew.image.scaled(nThumbnailPixmapMaxWidth, nThumbnailPixmapMaxWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            img = abstractNew.image;
        }

        if (img.isNull())
        {
            TOLOG("Failed to scale image to abstract");
            return false;
        }
        //
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if (!img.save(&buffer, "PNG"))
        {
            TOLOG("Failed to save abstract image data to buffer");
            return false;
        }
        buffer.close();
    }


    WIZABSTRACT abstractOld;
    if (abstractFromGuid(abstractNew.guid ,abstractOld, type))
    {
        CString whereField = CString("ABSTRACT_GUID=%1 and ABSTRACT_TYPE=%2")
                .arg(STR2SQL(abstractNew.guid))
                .arg(STR2SQL(type));

        CString strSql = CString("update %1 set ABSTRACT_TEXT=%2 where %3")
                .arg(TABLE_NAME_ABSTRACT)
                .arg(STR2SQL(abstractNew.text))
                .arg(whereField);

        try
        {
            m_dbThumb.execDML(strSql);
            m_dbThumb.updateBlob(TABLE_NAME_ABSTRACT, "ABSTRACT_IMAGE", (const unsigned char*)data.constData() , data.length(), whereField);
            return true;
        }
        catch (const CppSQLite3Exception& e)
        {
            TOLOG(e.errorMessage());
            TOLOG(strSql);
            return false;
        }
        catch (...)
        {
            TOLOG("Unknown exception while update document");
            return false;
        }
    }
    else
    {
        CString strSql = CString("insert into %1 (%2) values(%3, %4, %5, ?)")
                      .arg(TABLE_NAME_ABSTRACT)
                      .arg(FIELD_LIST_ABSTRACT)
                      .arg(STR2SQL(abstractNew.guid))
                      .arg(STR2SQL(type))
                      .arg(STR2SQL(abstractNew.text));

        try
        {
            m_dbThumb.insertBlob(strSql, (const unsigned char*)data.constData() , data.length());
            return true;
        }
        catch (const CppSQLite3Exception& e)
        {
            TOLOG(e.errorMessage());
            //TOLOG(strSql);
            return false;
        }
        catch (...)
        {
            TOLOG("Unknown exception while update document");
            return false;
        }
    }
    
	return true;
}

bool WizThumbIndex::deleteAbstractByGuid(const CString& guid)
{
    CString sql = CString("delete from ") + TABLE_NAME_ABSTRACT + " where ABSTRACT_GUID='"+guid+"'";
    try {
        m_dbThumb.execDML(sql);
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        TOLOG(e.errorMessage());
        TOLOG(sql);
        return false;
    }
    catch (...) {
        TOLOG("Unknown exception while update abstract");
        return false;
    }
}

bool WizThumbIndex::abstractIsExist(const CString& guid, const CString& type)
{
    if(!m_dbThumb.isOpened())
        return false;
    CString sql = CString("select ") + "ABSTRACT_GUID" + " from " +TABLE_NAME_ABSTRACT+" where ABSTRACT_GUID='"
    +guid+ ("' AND ABSTRACT_TYPE=")
    +STR2SQL(type)
    +(";");
    try {
        CppSQLite3Query query = m_dbThumb.execQuery(sql);
        while (!query.eof()) {
            return true;
        }
		return false;
	}
	catch (const CppSQLite3Exception& e)
	{
		TOLOG(e.errorMessage());
        TOLOG(sql);
		return false;
	}
	catch (...) {
		TOLOG("Unknown exception while close DB");
		return false;
	}
}

bool WizThumbIndex::phoneAbstractExist(const CString& guid)
{
    return abstractIsExist(guid, PHONE_TYPE);
}

bool WizThumbIndex::padAbstractExist(const CString& guid)
{
    return abstractIsExist(guid, PAD_TYPE);
}
