#include "wizthumbindex.h"
#include "wizmisc.h"
#include "wizindex.h"
#include <QBuffer>

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

CThumbIndex::CThumbIndex()
{
    
}

CThumbIndex::~CThumbIndex()
{
    
}
bool CThumbIndex::OpenThumb(const CString& strFileName)
{
    if (m_dbThumb.IsOpened())
		return true;
	//
	try {
        m_dbThumb.open(strFileName);
		//
        if (!InitThumbDB())
			return false;
		//
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
void CThumbIndex::CloseThumb()
{
    if (!m_dbThumb.IsOpened())
		return;
	//
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

bool CThumbIndex::IsThumbOpened()
{
    return m_dbThumb.IsOpened();
}

bool  CThumbIndex::checkThumbTable(const CString& strTableName, const CString& strTableSQL)
{
    if (m_dbThumb.tableExists(strTableName))
		return true;
	//
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

bool CThumbIndex::InitThumbDB()
{
    if (!m_dbThumb.IsOpened())
		return false;
	//
    if (!checkThumbTable(TABLE_NAME_ABSTRACT, ABSTRACT_TABLE_SQL))
        return false;
	//
	return true;
}
bool CThumbIndex::PadAbstractFromGUID(const CString& guid, WIZABSTRACT &abstract)
{
    return AbstractFromGUID(guid, abstract, PAD_TYPE);
}

bool CThumbIndex::PhoneAbstractFromGUID(const CString& guid, WIZABSTRACT &abstract)
{
    return AbstractFromGUID(guid, abstract, PHONE_TYPE);
}

bool CThumbIndex::AbstractFromGUID(const CString& guid, WIZABSTRACT &abstract,const CString& type)
{
    if(!m_dbThumb.IsOpened())
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

bool CThumbIndex::UpdatePadAbstract(const WIZABSTRACT &abstract)
{
    return UpdateAbstract(abstract, PAD_TYPE);
}

bool CThumbIndex::UpdateIphoneAbstract(const WIZABSTRACT &abstract)
{
    return UpdateAbstract(abstract, PHONE_TYPE);
}

bool CThumbIndex::UpdateAbstract(const WIZABSTRACT &abstractNew, const CString& type)
{
    if(!m_dbThumb.IsOpened())
    {
        TOLOG(_T("Fault error: thumb database does not opened"));
        return false;
    }
    //
    QByteArray data;
    if (abstractNew.image.width() > 0 && abstractNew.image.height() > 0)
    {
        int width = abstractNew.image.width();
        int height = abstractNew.image.height();
        //
        QImage img;
        if (width > 120
            || height > 120)
        {
            img = abstractNew.image.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        else
        {
            img = abstractNew.image;
        }
        //
        if (img.isNull())
        {
            TOLOG(_T("Faile to scale image to abstract"));
            return false;
        }
        //
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if (!img.save(&buffer, "JPG"))
        {
            TOLOG(_T("Faile to save abstract image data to buffer"));
            return false;
        }
        buffer.close();
    }
    //
    //
    WIZABSTRACT abstractOld;
    if (AbstractFromGUID(abstractNew.guid ,abstractOld, type))
    {
        CString whereFiled = CString("ABSTRACT_GUID=")
                                + STR2SQL(abstractNew.guid)
                                +(" AND ABSTRACT_TYPE = ")
                                +STR2SQL(type);

        CString sql = CString("update ") + TABLE_NAME_ABSTRACT + " set ABSTRACT_TEXT" + "='" + abstractNew.text + "' where " + whereFiled;
        try
        {
            m_dbThumb.execDML(sql);
            m_dbThumb.updateBlob(TABLE_NAME_ABSTRACT, "ABSTRACT_IMAGE", (const unsigned char*)data.constData() , data.length(), whereFiled);
            return true;
        }
        catch (const CppSQLite3Exception& e)
        {
            TOLOG(e.errorMessage());
            TOLOG(sql);
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
        CString sql = CString("insert into ") + TABLE_NAME_ABSTRACT + ("(") + FIELD_LIST_ABSTRACT + (")")
            + " values("
        + STR2SQL(abstractNew.guid) + (",")
        + STR2SQL(type) + (",")
        + STR2SQL(abstractNew.text) + (",?)");
        try
        {
            m_dbThumb.insertBlob(sql, (const unsigned char*)data.constData() , data.length());
            return true;
        }
        catch (const CppSQLite3Exception& e)
        {
            TOLOG(e.errorMessage());
            TOLOG(sql);
            return false;
        }
        catch (...)
        {
            TOLOG("Unknown exception while update document");
            return false;
        }
    }
    
	//
	return true;
    
}

bool CThumbIndex::DeleteAbstractByGUID(const CString& guid)
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

bool CThumbIndex::AbstractIsExist(const CString& guid, const CString& type)
{
    if(!m_dbThumb.IsOpened())
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

bool CThumbIndex::PhoneAbstractExist(const CString& guid)
{
    return AbstractIsExist(guid, PHONE_TYPE);
}

bool CThumbIndex::PadAbstractExist(const CString& guid)
{
    return AbstractIsExist(guid, PAD_TYPE);
}
