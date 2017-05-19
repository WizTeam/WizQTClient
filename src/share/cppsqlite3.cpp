////////////////////////////////////////////////////////////////////////////////
// CppSQLite3 - A C++ wrapper around the SQLite3 embedded database library.
//
// Copyright (c) 2004 Rob Groves. All Rights Reserved. rob.groves@btinternet.com
// 
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice, 
// this paragraph and the following two paragraphs appear in all copies, 
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// V3.0		03/08/2004	-Initial Version for sqlite3
//
// V3.1		16/09/2004	-Implemented getXXXXField using sqlite3 functions
//						-Added CppSQLiteDB3::tableExists()
////////////////////////////////////////////////////////////////////////////////
#include "cppsqlite3.h"
#include <cstdlib>
#include <assert.h>

#include "../utils/WizPathResolve.h"
#include "../utils/WizLogger.h"


// Named constant for passing to CppSQLite3Exception when passing it a string
// that cannot be deleted.
static const bool DONT_DELETE_MSG=false;

////////////////////////////////////////////////////////////////////////////////
// Prototypes for SQLite functions not included in SQLite DLL, but copied below
// from SQLite encode.c
////////////////////////////////////////////////////////////////////////////////
int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out);
//int sqlite3_decode_binary(const unsigned char *in, unsigned char *out);


////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////


CppSQLite3Exception::CppSQLite3Exception(const int nErrCode, const CString& errMessage)
    : mnErrCode(nErrCode)
{
    msErrMessage = WizFormatString3("%1[%2]: %3", errorCodeAsString(nErrCode), WizIntToStr(nErrCode), errMessage);
}
//

CppSQLite3Exception::CppSQLite3Exception(const CppSQLite3Exception&  e)
    : mnErrCode(e.mnErrCode)
    , msErrMessage(e.msErrMessage)
{
}

void CppSQLite3Exception::throwException(int nErrCode, char* errMessage)
{
    if (errMessage)
    {
        CString str(errMessage);
        ::sqlite3_free(errMessage);
        throw CppSQLite3Exception(nErrCode, str);
    }
    else
    {
        throw CppSQLite3Exception(nErrCode, "");
    }
}

const char* CppSQLite3Exception::errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
		case SQLITE_OK          : return "SQLITE_OK";
		case SQLITE_ERROR       : return "SQLITE_ERROR";
		case SQLITE_INTERNAL    : return "SQLITE_INTERNAL";
		case SQLITE_PERM        : return "SQLITE_PERM";
		case SQLITE_ABORT       : return "SQLITE_ABORT";
		case SQLITE_BUSY        : return "SQLITE_BUSY";
		case SQLITE_LOCKED      : return "SQLITE_LOCKED";
		case SQLITE_NOMEM       : return "SQLITE_NOMEM";
		case SQLITE_READONLY    : return "SQLITE_READONLY";
		case SQLITE_INTERRUPT   : return "SQLITE_INTERRUPT";
		case SQLITE_IOERR       : return "SQLITE_IOERR";
		case SQLITE_CORRUPT     : return "SQLITE_CORRUPT";
		case SQLITE_NOTFOUND    : return "SQLITE_NOTFOUND";
		case SQLITE_FULL        : return "SQLITE_FULL";
		case SQLITE_CANTOPEN    : return "SQLITE_CANTOPEN";
		case SQLITE_PROTOCOL    : return "SQLITE_PROTOCOL";
		case SQLITE_EMPTY       : return "SQLITE_EMPTY";
		case SQLITE_SCHEMA      : return "SQLITE_SCHEMA";
		case SQLITE_TOOBIG      : return "SQLITE_TOOBIG";
		case SQLITE_CONSTRAINT  : return "SQLITE_CONSTRAINT";
		case SQLITE_MISMATCH    : return "SQLITE_MISMATCH";
		case SQLITE_MISUSE      : return "SQLITE_MISUSE";
		case SQLITE_NOLFS       : return "SQLITE_NOLFS";
		case SQLITE_AUTH        : return "SQLITE_AUTH";
		case SQLITE_FORMAT      : return "SQLITE_FORMAT";
		case SQLITE_RANGE       : return "SQLITE_RANGE";
		case SQLITE_ROW         : return "SQLITE_ROW";
		case SQLITE_DONE        : return "SQLITE_DONE";
		case CPPSQLITE_ERROR    : return "CPPSQLITE_ERROR";
		default: return "UNKNOWN_ERROR";
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Query::CppSQLite3Query()
{
	mpVM = 0;
	mbEof = true;
	mnCols = 0;
	mbOwnVM = false;
}


CppSQLite3Query::CppSQLite3Query(const CppSQLite3Query& rQuery)
{
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3Query&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
}


CppSQLite3Query::CppSQLite3Query(sqlite3* pDB,
							sqlite3_stmt* pVM,
                            bool bEof,
                            bool bOwnVM/*=true*/)
{
	mpDB = pDB;
	mpVM = pVM;
	mbEof = bEof;
	mnCols = sqlite3_column_count(mpVM);
	mbOwnVM = bOwnVM;
}


CppSQLite3Query::~CppSQLite3Query()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3Query& CppSQLite3Query::operator=(const CppSQLite3Query& rQuery)
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3Query&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
	return *this;
}


int CppSQLite3Query::numFields()
{
	checkVM();
	return mnCols;
}


const char* CppSQLite3Query::fieldValue(int nField)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
	}

	return (const char*)sqlite3_column_text(mpVM, nField);
}


const char* CppSQLite3Query::fieldValue(const CString& szField)
{
    int nField = fieldIndex(szField);
    return fieldValue(nField);
}
const unsigned short* CppSQLite3Query::fieldValue16(const CString& szField)
{
    checkVM();

    int nField = fieldIndex(szField);
    if (nField < 0 || nField > mnCols-1)
    {
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
    }

    return (const unsigned short*)sqlite3_column_text16(mpVM, nField);
}


int CppSQLite3Query::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return sqlite3_column_int(mpVM, nField);
	}
}


int CppSQLite3Query::getIntField(const CString& szField, int nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getIntField(nField, nNullValue);
}


__int64 CppSQLite3Query::getInt64Field(int nField, int nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return sqlite3_column_int64(mpVM, nField);
	}
}


__int64 CppSQLite3Query::getInt64Field(const CString& szField, int nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getInt64Field(nField, nNullValue);
}


BOOL CppSQLite3Query::getBoolField(int nField, BOOL bNullValue/*=FALSE*/)
{
	int nDefaultValue = bNullValue ? 1 : 0;
	//
    return getIntField(nField, nDefaultValue) ? TRUE : FALSE;
}
BOOL CppSQLite3Query::getBoolField(const CString& szField, BOOL bNullValue/*=FALSE*/)
{
	int nField = fieldIndex(szField);
    return getBoolField(nField, bNullValue);
}


double CppSQLite3Query::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return fNullValue;
	}
	else
	{
		return sqlite3_column_double(mpVM, nField);
	}
}


double CppSQLite3Query::getFloatField(const CString& szField, double fNullValue/*=0.0*/)
{
	int nField = fieldIndex(szField);
	return getFloatField(nField, fNullValue);
}


WizOleDateTime CppSQLite3Query::getTimeField(int nField, time_t tNullValue /*= 0*/)
{
    CString str = getStringField(nField);
    if (str.isEmpty())
	{
		if (0 == tNullValue)
            return WizGetCurrentTime();
		else
            return WizOleDateTime(tNullValue);
	}
	else
	{
		return WizStringToDateTime(str);
	}
}
WizOleDateTime CppSQLite3Query::getTimeField(const CString& szField, time_t tNullValue /*= 0*/)
{
	int nField = fieldIndex(szField);
	return getTimeField(nField, tNullValue);
}

COLORREF CppSQLite3Query::getColorField(int nField, COLORREF crNullValue /*= 0*/)
{
    CString str = getStringField(nField);
    if (str.isEmpty())
	{
		return crNullValue;
	}
	else
	{
		return WizStringToColor(str);
	}
}
QColor CppSQLite3Query::getColorField2(const CString& szField, QColor crNullValue /*= 0*/)
{
	int nField = fieldIndex(szField);
    return getColorField2(nField, crNullValue);
}

QColor CppSQLite3Query::getColorField2(int nField, QColor crNullValue /*= 0*/)
{
    CString str = getStringField(nField);
    if (str.isEmpty())
    {
        return crNullValue;
    }
    else
    {
        return WizStringToColor2(str);
    }
}
COLORREF CppSQLite3Query::getColorField(const CString& szField, COLORREF crNullValue /*= 0*/)
{
    int nField = fieldIndex(szField);
    return getColorField(nField, crNullValue);
}

CString CppSQLite3Query::getStringField(int nField, const CString& szNullValue/*=""*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
        return CString::fromUtf8(fieldValue(nField));
	}
}

CString CppSQLite3Query::getStringField(const CString& szField, const CString& szNullValue/*=""*/)
{
	int nField = fieldIndex(szField);
	return getStringField(nField, szNullValue);
}

int CppSQLite3Query::getColumnLength(int nField)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
	}

	int nLen = sqlite3_column_bytes(mpVM, nField);
	//
	return nLen;
}
const unsigned char* CppSQLite3Query::getBlobField(int nField, int& nLen)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
	}

	nLen = sqlite3_column_bytes(mpVM, nField);
	return (const unsigned char*)sqlite3_column_blob(mpVM, nField);
}


const unsigned char* CppSQLite3Query::getBlobField(const CString& szField, int& nLen)
{
	int nField = fieldIndex(szField);
	return getBlobField(nField, nLen);
}


bool CppSQLite3Query::fieldIsNull(int nField)
{
	return (fieldDataType(nField) == SQLITE_NULL);
}


bool CppSQLite3Query::fieldIsNull(const CString& szField)
{
	int nField = fieldIndex(szField);
	return (fieldDataType(nField) == SQLITE_NULL);
}

int CppSQLite3Query::fieldIndex(const CString& szField)
{
	checkVM();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			const char* szTemp = sqlite3_column_name(mpVM, nField);

            if (szField.compareNoCase(szTemp) == 0)
			{
				return nField;
			}
		}
	}

    throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field name requested");
}


CString CppSQLite3Query::fieldName(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
	}

	return sqlite3_column_name(mpVM, nCol);
}

CString CppSQLite3Query::fieldDeclType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
	}

	return sqlite3_column_decltype(mpVM, nCol);
}


int CppSQLite3Query::fieldDataType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
	}

	return sqlite3_column_type(mpVM, nCol);
}


bool CppSQLite3Query::eof()
{
	checkVM();
	return mbEof;
}


void CppSQLite3Query::nextRow()
{
	checkVM();

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		mbEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		nRet = sqlite3_finalize(mpVM);
		mpVM = 0;
        const char* szError = sqlite3_errmsg(mpDB);
        throw CppSQLite3Exception(nRet, szError);
	}
}


void CppSQLite3Query::finalize()
{
	if (mpVM && mbOwnVM)
	{
		int nRet = sqlite3_finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			const char* szError = sqlite3_errmsg(mpDB);
            throw CppSQLite3Exception(nRet, szError);
		}
	}
}


void CppSQLite3Query::checkVM()
{
	if (mpVM == 0)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Null Virtual Machine pointer");
	}
}

////////////////////////////////////////////////////////////////////////////////

CppSQLite3Statement::CppSQLite3Statement()
{
	mpDB = 0;
	mpVM = 0;
}


CppSQLite3Statement::CppSQLite3Statement(const CppSQLite3Statement& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3Statement&>(rStatement).mpVM = 0;
}


CppSQLite3Statement::CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pVM)
{
	mpDB = pDB;
	mpVM = pVM;
}


CppSQLite3Statement::~CppSQLite3Statement()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3Statement& CppSQLite3Statement::operator=(const CppSQLite3Statement& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3Statement&>(rStatement).mpVM = 0;
	return *this;
}


int CppSQLite3Statement::execDML()
{
	checkDB();
	checkVM();

	const char* szError=0;

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = sqlite3_changes(mpDB);

		nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			szError = sqlite3_errmsg(mpDB);
            throw CppSQLite3Exception(nRet, szError);
		}

		return nRowsChanged;
	}
	else
	{
		nRet = sqlite3_reset(mpVM);
		szError = sqlite3_errmsg(mpDB);
        throw CppSQLite3Exception(nRet, szError);
	}
}


CppSQLite3Query CppSQLite3Statement::execQuery()
{
	checkDB();
	checkVM();

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3Query(mpDB, mpVM, true/*eof*/, false);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3Query(mpDB, mpVM, false/*eof*/, false);
	}
	else
	{
		nRet = sqlite3_reset(mpVM);
		const char* szError = sqlite3_errmsg(mpDB);
        throw CppSQLite3Exception(nRet, szError);
	}
}


void CppSQLite3Statement::bind(int nParam, const char* szValue)
{
	checkVM();
	int nRes = sqlite3_bind_text(mpVM, nParam, szValue, -1, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
        throw CppSQLite3Exception(nRes, "Error binding string param");
	}
}


void CppSQLite3Statement::bind(int nParam, const int nValue)
{
	checkVM();
	int nRes = sqlite3_bind_int(mpVM, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
        throw CppSQLite3Exception(nRes, "Error binding int param");
	}
}


void CppSQLite3Statement::bind(int nParam, const double dValue)
{
	checkVM();
	int nRes = sqlite3_bind_double(mpVM, nParam, dValue);

	if (nRes != SQLITE_OK)
	{
        throw CppSQLite3Exception(nRes, "Error binding double param");
	}
}


void CppSQLite3Statement::bind(int nParam, const unsigned char* blobValue, int nLen)
{
	checkVM();
	int nRes = sqlite3_bind_blob(mpVM, nParam,
								(const void*)blobValue, nLen, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
        throw CppSQLite3Exception(nRes, "Error binding blob param");
	}
}

	
void CppSQLite3Statement::bindNull(int nParam)
{
	checkVM();
	int nRes = sqlite3_bind_null(mpVM, nParam);

	if (nRes != SQLITE_OK)
	{
        throw CppSQLite3Exception(nRes, "Error binding NULL param");
	}
}


void CppSQLite3Statement::reset()
{
	if (mpVM)
	{
		int nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			const char* szError = sqlite3_errmsg(mpDB);
            throw CppSQLite3Exception(nRet, szError);
		}
	}
}


void CppSQLite3Statement::finalize()
{
	if (mpVM)
	{
		int nRet = sqlite3_finalize(mpVM);
		mpVM = 0;

		if (nRet != SQLITE_OK)
		{
			const char* szError = sqlite3_errmsg(mpDB);
            throw CppSQLite3Exception(nRet, szError);
		}
	}
}


void CppSQLite3Statement::checkDB()
{
	if (mpDB == 0)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Database not open");
	}
}


void CppSQLite3Statement::checkVM()
{
	if (mpVM == 0)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Null Virtual Machine pointer");
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3DB::CppSQLite3DB()
{
	mpDB = 0;
	mnBusyTimeoutMs = 60000; // 60 seconds
}


CppSQLite3DB::CppSQLite3DB(const CppSQLite3DB& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
}


CppSQLite3DB::~CppSQLite3DB()
{
	close();
}


CppSQLite3DB& CppSQLite3DB::operator=(const CppSQLite3DB& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}


void CppSQLite3DB::open(const CString& strFile)
{
    int nRet = sqlite3_open16(strFile.utf16(), &mpDB);

	if (nRet != SQLITE_OK)
	{
		const char* szError = sqlite3_errmsg(mpDB);
        throw CppSQLite3Exception(nRet, szError);
	}

	setBusyTimeout(mnBusyTimeoutMs);
}


void CppSQLite3DB::close()
{
	if (mpDB)
	{
		sqlite3_close(mpDB);
		mpDB = 0;
	}
}


CppSQLite3Statement CppSQLite3DB::compileStatement(const CString& szSQL)
{
	checkDB();

	sqlite3_stmt* pVM = compile(szSQL);
	return CppSQLite3Statement(mpDB, pVM);
}


bool CppSQLite3DB::tableExists(const CString& strTable)
{
    CString strSQL = WizFormatString1(_T("select count(*) from sqlite_master where type='table' and name='%1'"), strTable);
    int nRet = execScalar(strSQL);
	return (nRet > 0);
}

bool CppSQLite3DB::columnExists(const CString& strTable, const CString& strColumn)
{
    try
    {
        CString strSQL = WizFormatString1("select * from %1 limit 0, 1", strTable);
        CppSQLite3Query query = execQuery(strSQL);
        return query.fieldIndex(strColumn) != -1;
    }
    catch (const CppSQLite3Exception& )
    {
        return false;
    }
}

int CppSQLite3DB::execDML(const CString& strSQL)
{
	checkDB();

	char* szError=0;
    //
    QByteArray utf8 = strSQL.toUtf8();
    //
    int nRet = sqlite3_exec(mpDB, utf8.constData(), 0, 0, &szError);

	if (nRet == SQLITE_OK)
	{
		return sqlite3_changes(mpDB);
	}
	else
	{
        CppSQLite3Exception::throwException(nRet, szError);
        return 0;
	}
}

CppSQLite3Query CppSQLite3DB::execQuery(const CString& strSQL)
{
	checkDB();

    sqlite3_stmt* pVM = compile(strSQL);

	int nRet = sqlite3_step(pVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3Query(mpDB, pVM, true/*eof*/);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3Query(mpDB, pVM, false/*eof*/);
	}
	else
	{
		nRet = sqlite3_finalize(pVM);
		const char* szError= sqlite3_errmsg(mpDB);
        throw CppSQLite3Exception(nRet, szError);
	}
}


int CppSQLite3DB::execScalar(const CString& strSQL)
{
    CppSQLite3Query q = execQuery(strSQL);

	if (q.eof() || q.numFields() < 1)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid scalar query");
	}

	return atoi(q.fieldValue(0));
}

sqlite_int64 CppSQLite3DB::lastRowId()
{
	return sqlite3_last_insert_rowid(mpDB);
}


void CppSQLite3DB::setBusyTimeout(int nMillisecs)
{
	mnBusyTimeoutMs = nMillisecs;
	sqlite3_busy_timeout(mpDB, mnBusyTimeoutMs);
}

BOOL CppSQLite3DB::isOpened()
{
    return mpDB ? TRUE : FALSE;
}

void CppSQLite3DB::checkDB()
{
	if (!mpDB)
	{
        throw CppSQLite3Exception(CPPSQLITE_ERROR, "Database not open");
	}
}


sqlite3_stmt* CppSQLite3DB::compile(const CString& strSQL)
{
	checkDB();

    const void* szTail=0;
	sqlite3_stmt* pVM;

    int nRet = sqlite3_prepare16(mpDB, strSQL, -1, &pVM, &szTail);

	if (nRet != SQLITE_OK)
	{
        throw CppSQLite3Exception(nRet, (const unsigned short*)szTail);
	}

	return pVM;
}


int CppSQLite3DB::updateBlob(const CString& szTableName, const CString& szFieldName, const unsigned char* data, int dataLength, const CString& szWhere)
{

    CString sql = CString("update ") + szTableName + " set " + szFieldName + "=? where " + szWhere;

    CppSQLite3Statement statement = compileStatement(sql);
    statement.bind(1, data, dataLength);

    return statement.execDML();
}

int CppSQLite3DB::insertBlob(const CString& szSQL, const unsigned char *data, int dataLength)
{
    CppSQLite3Statement statement = compileStatement(szSQL);

    statement.bind(1, data, dataLength);

    return statement.execDML();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
** An pointer to an instance of this structure is passed from
** the main program to the callback.  This is used to communicate
** state and mode information.
*/
struct callback_data {
	sqlite3 *db;           /* The database */
	FILE *out;             /* Write results here */
	int writableSchema;    /* True if PRAGMA writable_schema=ON */
	sqlite3_stmt *pStmt;   /* Current statement if any. */
	int cnt;               /* Number of records displayed so far */
};



/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
static int strlen30(const char *z){
  const char *z2 = z;
  while( *z2 ){ z2++; }
  return 0x3fffffff & (int)(z2 - z);
}


/* zIn is either a pointer to a NULL-terminated string in memory obtained
** from malloc(), or a NULL pointer. The string pointed to by zAppend is
** added to zIn, and the result returned in memory obtained from malloc().
** zIn, if it was not NULL, is freed.
**
** If the third argument, quote, is not '\0', then it is used as a 
** quote character for zAppend.
*/
static char *appendText(char *zIn, char const *zAppend, char quote){
	int len;
	int i;
	int nAppend = strlen30(zAppend);
	int nIn = (zIn?strlen30(zIn):0);

	len = nAppend+nIn+1;
	if( quote ){
		len += 2;
		for(i=0; i<nAppend; i++){
			if( zAppend[i]==quote ) len++;
		}
	}

	zIn = (char *)realloc(zIn, len);
	if( !zIn ){
		return 0;
	}

	if( quote ){
		char *zCsr = &zIn[nIn];
		*zCsr++ = quote;
		for(i=0; i<nAppend; i++){
			*zCsr++ = zAppend[i];
			if( zAppend[i]==quote ) *zCsr++ = quote;
		}
		*zCsr++ = quote;
		*zCsr++ = '\0';
		assert( (zCsr-zIn)==len );
	}else{
		memcpy(&zIn[nIn], zAppend, nAppend);
		zIn[len-1] = '\0';
	}

	return zIn;
}


/*
** Execute a query statement that has a single result column.  Print
** that result column on a line by itself with a semicolon terminator.
**
** This is used, for example, to show the schema of the database by
** querying the SQLITE_MASTER table.
*/
static int run_table_dump_query(
								FILE *out,              /* Send output here */
								sqlite3 *db,            /* Database to query */
								const char *zSelect,    /* SELECT statement to extract content */
								const char *zFirstRow   /* Print before first row, if not NULL */
								)
{
	sqlite3_stmt *pSelect;
	int rc;
	rc = sqlite3_prepare(db, zSelect, -1, &pSelect, 0);
	if( rc!=SQLITE_OK || !pSelect ){
		return rc;
	}
	rc = sqlite3_step(pSelect);
	while( rc==SQLITE_ROW ){
		if( zFirstRow ){
			fprintf(out, "%s", zFirstRow);
			zFirstRow = 0;
		}
		fprintf(out, "%s;\n", sqlite3_column_text(pSelect, 0));
		rc = sqlite3_step(pSelect);
	}
	return sqlite3_finalize(pSelect);
}

/*
** This is a different callback routine used for dumping the database.
** Each row received by this callback consists of a table name,
** the table type ("index" or "table") and SQL to create the table.
** This routine should print text sufficient to recreate the table.
*/
static int dump_callback(void *pArg, int nArg, char **azArg, char **azCol){
	int rc;
	const char *zTable;
	const char *zType;
	const char *zSql;
	const char *zPrepStmt = 0;
	struct callback_data *p = (struct callback_data *)pArg;
        //
        Q_UNUSED(azCol);

	//UNUSED_PARAMETER(azCol);
	if( nArg!=3 ) return 1;
	zTable = azArg[0];
	zType = azArg[1];
	zSql = azArg[2];

	if( strcmp(zTable, "sqlite_sequence")==0 ){
		zPrepStmt = "DELETE FROM sqlite_sequence;\n";
	}else if( strcmp(zTable, "sqlite_stat1")==0 ){
		fprintf(p->out, "ANALYZE sqlite_master;\n");
	}else if( strncmp(zTable, "sqlite_", 7)==0 ){
		return 0;
	}else if( strncmp(zSql, "CREATE VIRTUAL TABLE", 20)==0 ){
		char *zIns;
		if( !p->writableSchema ){
			fprintf(p->out, "PRAGMA writable_schema=ON;\n");
			p->writableSchema = 1;
		}
		zIns = sqlite3_mprintf(
			"INSERT INTO sqlite_master(type,name,tbl_name,rootpage,sql)"
			"VALUES('table','%q','%q',0,'%q');",
			zTable, zTable, zSql);
		fprintf(p->out, "%s\n", zIns);
		sqlite3_free(zIns);
		return 0;
	}else{
		fprintf(p->out, "%s;\n", zSql);
	}

	if( strcmp(zType, "table")==0 ){
		sqlite3_stmt *pTableInfo = 0;
		char *zSelect = 0;
		char *zTableInfo = 0;
		char *zTmp = 0;
		int nRow = 0;

		zTableInfo = appendText(zTableInfo, "PRAGMA table_info(", 0);
		zTableInfo = appendText(zTableInfo, zTable, '"');
		zTableInfo = appendText(zTableInfo, ");", 0);

		rc = sqlite3_prepare(p->db, zTableInfo, -1, &pTableInfo, 0);
		free(zTableInfo);
		if( rc!=SQLITE_OK || !pTableInfo ){
			return 1;
		}

		zSelect = appendText(zSelect, "SELECT 'INSERT INTO ' || ", 0);
		zTmp = appendText(zTmp, zTable, '"');
		if( zTmp ){
			zSelect = appendText(zSelect, zTmp, '\'');
		}
		zSelect = appendText(zSelect, " || ' VALUES(' || ", 0);
		rc = sqlite3_step(pTableInfo);
		while( rc==SQLITE_ROW ){
			const char *zText = (const char *)sqlite3_column_text(pTableInfo, 1);
			zSelect = appendText(zSelect, "quote(", 0);
			zSelect = appendText(zSelect, zText, '"');
			rc = sqlite3_step(pTableInfo);
			if( rc==SQLITE_ROW ){
				zSelect = appendText(zSelect, ") || ',' || ", 0);
			}else{
				zSelect = appendText(zSelect, ") ", 0);
			}
			nRow++;
		}
		rc = sqlite3_finalize(pTableInfo);
		if( rc!=SQLITE_OK || nRow==0 ){
			free(zSelect);
			return 1;
		}
		zSelect = appendText(zSelect, "|| ')' FROM  ", 0);
		zSelect = appendText(zSelect, zTable, '"');

		rc = run_table_dump_query(p->out, p->db, zSelect, zPrepStmt);
		if( rc==SQLITE_CORRUPT ){
			zSelect = appendText(zSelect, " ORDER BY rowid DESC", 0);
			rc = run_table_dump_query(p->out, p->db, zSelect, 0);
		}
		if( zSelect ) free(zSelect);
	}
	return 0;
}


static int run_schema_dump_query(struct callback_data *p, const char *zQuery, char **pzErrMsg)
{
	int rc;
	rc = sqlite3_exec(p->db, zQuery, dump_callback, p, pzErrMsg);
	if( rc==SQLITE_CORRUPT ){
		char *zQ2;
		int len = strlen30(zQuery);
		if( pzErrMsg ) sqlite3_free(*pzErrMsg);
		zQ2 = (char*)malloc( len+100 );
		if( zQ2==0 ) return rc;
		sqlite3_snprintf(sizeof(zQ2), zQ2, "%s ORDER BY rowid DESC", zQuery);
		rc = sqlite3_exec(p->db, zQ2, dump_callback, p, pzErrMsg);
		free(zQ2);
	}
	return rc;
}

/*
** If the following flag is set, then command execution stops
** at an error if we are not interactive.
*/
static int bail_on_error = 0;

/*
** Threat stdin as an interactive input if the following variable
** is true.  Otherwise, assume stdin is connected to a file or pipe.
*/
static int stdin_is_interactive = 1;


/*
** True if an interrupt (Control-C) has been received.
*/
static volatile int seenInterrupt = 0;


# define readline(p) local_getline(p,stdin)


/*
** This routine reads a line of text from FILE in, stores
** the text in memory obtained from malloc() and returns a pointer
** to the text.  NULL is returned at end of file, or if malloc()
** fails.
**
** The interface is like "readline" but no command-line editing
** is done.
*/
static char *local_getline(char *zPrompt, FILE *in){
	char *zLine;
	int nLine;
	int n;
	int eol;

	if( zPrompt && *zPrompt ){
		printf("%s",zPrompt);
		fflush(stdout);
	}
	nLine = 100;
	zLine = (char*)malloc( nLine );
	if( zLine==0 ) return 0;
	n = 0;
	eol = 0;
	while( !eol ){
		if( n+100>nLine ){
			nLine = nLine*2 + 100;
			zLine = (char*)realloc(zLine, nLine);
			if( zLine==0 ) return 0;
		}
		if( fgets(&zLine[n], nLine - n, in)==0 ){
			if( n==0 ){
				free(zLine);
				return 0;
			}
			zLine[n] = 0;
			eol = 1;
			break;
		}
		while( zLine[n] ){ n++; }
		if( n>0 && zLine[n-1]=='\n' ){
			n--;
			if( n>0 && zLine[n-1]=='\r' ) n--;
			zLine[n] = 0;
			eol = 1;
		}
	}
	zLine = (char*)realloc( zLine, n+1 );
	return zLine;
}
/*
** Retrieve a single line of input text.
**
** zPrior is a string of prior text retrieved.  If not the empty
** string, then issue a continuation prompt.
*/
static char *one_input_line(FILE *in){
	if( in!=0 ){
		return local_getline(0, in);
	}
	//
	assert(false);
	exit(0);
	//
	return NULL;
}


/*
** Test to see if a line consists entirely of whitespace.
*/
static int _all_whitespace(const char *z){
  for(; *z; z++){
    if( isspace(*(unsigned char*)z) ) continue;
    if( *z=='/' && z[1]=='*' ){
      z += 2;
      while( *z && (*z!='*' || z[1]!='/') ){ z++; }
      if( *z==0 ) return 0;
      z++;
      continue;
    }
    if( *z=='-' && z[1]=='-' ){
      z += 2;
      while( *z && *z!='\n' ){ z++; }
      if( *z==0 ) return 1;
      continue;
    }
    return 0;
  }
  return 1;
}


/*
** Return TRUE if the line typed in is an SQL command terminator other
** than a semi-colon.  The SQL Server style "go" command is understood
** as is the Oracle "/".
*/
static int _is_command_terminator(const char *zLine){
	while( isspace(*(unsigned char*)zLine) ){ zLine++; };
	if( zLine[0]=='/' && _all_whitespace(&zLine[1]) ){
		return 1;  /* Oracle */
	}
	if( tolower(zLine[0])=='g' && tolower(zLine[1])=='o'
		&& _all_whitespace(&zLine[2]) ){
			return 1;  /* SQL Server */
	}
	return 0;
}

/*
** Return true if zSql is a complete SQL statement.  Return false if it
** ends in the middle of a string literal or C-style comment.
*/
static int _is_complete(char *zSql, int nSql){
	int rc;
	if( zSql==0 ) return 1;
	zSql[nSql] = ';';
	zSql[nSql+1] = 0;
	rc = sqlite3_complete(zSql);
	zSql[nSql] = 0;
	return rc;
}
/*
** Return TRUE if a semicolon occurs anywhere in the first N characters
** of string z[].
*/
static int _contains_semicolon(const char *z, int N){
	int i;
	for(i=0; i<N; i++){  if( z[i]==';' ) return 1; }
	return 0;
}



/*
** Allocate space and save off current error string.
*/
static char *save_err_msg(
						  sqlite3 *db            /* Database to query */
						  )
{
	int nErrMsg = 1+strlen30(sqlite3_errmsg(db));
	char *zErrMsg = (char*)sqlite3_malloc(nErrMsg);
	if( zErrMsg ){
	  memcpy(zErrMsg, sqlite3_errmsg(db), nErrMsg);
	}
	return zErrMsg;
}
/*
** Execute a statement or set of statements.  Print 
** any result rows/columns depending on the current mode 
** set via the supplied callback.
**
** This is very similar to SQLite's built-in sqlite3_exec() 
** function except it takes a slightly different callback 
** and callback data argument.
*/
static int shell_exec(
					  sqlite3 *db,                                /* An open database */
					  const char *zSql,                           /* SQL to be evaluated */
					  int (*xCallback)(void*,int,char**,char**,int*),   /* Callback function (not the same as sqlite3_exec) */
					  struct callback_data *pArg,                 /* Pointer to struct callback_data */
					  char **pzErrMsg                             /* Error msg written here */
	)
{
	sqlite3_stmt *pStmt = NULL;     /* Statement to execute. */
	int rc = SQLITE_OK;             /* Return Code */
	const char *zLeftover;          /* Tail of unprocessed SQL */

	if( pzErrMsg ){
		*pzErrMsg = NULL;
	}

	while( zSql[0] && (SQLITE_OK == rc) ){
		rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
		if( SQLITE_OK != rc ){
			if( pzErrMsg ){
				*pzErrMsg = save_err_msg(db);
			}
		}else{
			if( !pStmt ){
				/* this happens for a comment or white-space */
				zSql = zLeftover;
				while( isspace(zSql[0]) ) zSql++;
				continue;
			}

			/* save off the prepared statment handle and reset row count */
			if( pArg ){
				pArg->pStmt = pStmt;
				pArg->cnt = 0;
			}

			/* perform the first step.  this will tell us if we
			** have a result set or not and how wide it is.
			*/
			rc = sqlite3_step(pStmt);
			/* if we have a result set... */
			if( SQLITE_ROW == rc ){
				/* if we have a callback... */
				if( xCallback ){
					/* allocate space for col name ptr, value ptr, and type */
					int nCol = sqlite3_column_count(pStmt);
					void *pData = sqlite3_malloc(3*nCol*sizeof(const char*) + 1);
					if( !pData ){
						rc = SQLITE_NOMEM;
					}else{
						char **azCols = (char **)pData;      /* Names of result columns */
						char **azVals = &azCols[nCol];       /* Results */
						int *aiTypes = (int *)&azVals[nCol]; /* Result types */
						int i;
						assert(sizeof(int) <= sizeof(char *)); 
						/* save off ptrs to column names */
						for(i=0; i<nCol; i++){
							azCols[i] = (char *)sqlite3_column_name(pStmt, i);
						}
						do{
							/* extract the data and data types */
							for(i=0; i<nCol; i++){
								azVals[i] = (char *)sqlite3_column_text(pStmt, i);
								aiTypes[i] = sqlite3_column_type(pStmt, i);
								if( !azVals[i] && (aiTypes[i]!=SQLITE_NULL) ){
									rc = SQLITE_NOMEM;
									break; /* from for */
								}
							} /* end for */

							/* if data and types extracted successfully... */
							if( SQLITE_ROW == rc ){ 
								/* call the supplied callback with the result row data */
								if( xCallback(pArg, nCol, azVals, azCols, aiTypes) ){
									rc = SQLITE_ABORT;
								}else{
									rc = sqlite3_step(pStmt);
								}
							}
						} while( SQLITE_ROW == rc );
						sqlite3_free(pData);
					}
				}else{
					do{
						rc = sqlite3_step(pStmt);
					} while( rc == SQLITE_ROW );
				}
			}

			/* Finalize the statement just executed. If this fails, save a 
			** copy of the error message. Otherwise, set zSql to point to the
			** next statement to execute. */
			rc = sqlite3_finalize(pStmt);
			if( rc==SQLITE_OK ){
				zSql = zLeftover;
				while( isspace(zSql[0]) ) zSql++;
			}else if( pzErrMsg ){
				*pzErrMsg = save_err_msg(db);
			}

			/* clear saved stmt handle */
			if( pArg ){
				pArg->pStmt = NULL;
			}
		}
	} /* end while */

	return rc;
}


/*
** This is the callback routine that the shell
** invokes for each row of a query result.
*/
static int shell_callback(void *pArg, int nArg, char **azArg, char **azCol, int *aiType)
{
    Q_UNUSED(pArg);
    Q_UNUSED(nArg);
    Q_UNUSED(azArg);
    Q_UNUSED(azCol);
    Q_UNUSED(aiType);

	return 0;
}

/*
** Read input from *in and process it.  If *in==0 then input
** is interactive - the user is typing it it.  Otherwise, input
** is coming from a file or device.  A prompt is issued and history
** is saved only if input is interactive.  An interrupt signal will
** cause this routine to exit immediately, unless input is interactive.
**
** Return the number of errors.
*/
static int process_input(struct callback_data *p, FILE *in){
	char *zLine = 0;
	char *zSql = 0;
	int nSql = 0;
	int nSqlPrior = 0;
	char *zErrMsg;
	int rc;
	int errCnt = 0;
	int lineno = 0;
	int startline = 0;

	while( errCnt==0 || !bail_on_error || (in==0 && stdin_is_interactive) ){
		//fflush(p->out);
		free(zLine);
		zLine = one_input_line(in);
		if( zLine==0 ){
			break;  /* We have reached EOF */
		}
		if( seenInterrupt ){
			if( in!=0 ) break;
			seenInterrupt = 0;
		}
		lineno++;
		if( (zSql==0 || zSql[0]==0) && _all_whitespace(zLine) ) continue;
		if( zLine && zLine[0]=='.' && nSql==0 ){
			continue;
		}
		if( _is_command_terminator(zLine) && _is_complete(zSql, nSql) ){
			memcpy(zLine,";",2);
		}
		nSqlPrior = nSql;
		if( zSql==0 ){
			int i;
			for(i=0; zLine[i] && isspace((unsigned char)zLine[i]); i++){}
			if( zLine[i]!=0 ){
				nSql = strlen30(zLine);
				zSql = (char*)malloc( nSql+3 );
				if( zSql==0 ){
					fprintf(stderr, "Error: out of memory\n");
					exit(1);
				}
				memcpy(zSql, zLine, nSql+1);
				startline = lineno;
			}
		}else{
			int len = strlen30(zLine);
			zSql = (char*)realloc( zSql, nSql + len + 4 );
			if( zSql==0 ){
				fprintf(stderr,"Error: out of memory\n");
				exit(1);
			}
			zSql[nSql++] = '\n';
			memcpy(&zSql[nSql], zLine, len+1);
			nSql += len;
		}
		if( zSql && _contains_semicolon(&zSql[nSqlPrior], nSql-nSqlPrior)
			&& sqlite3_complete(zSql) ){
				p->cnt = 0;
				//
				rc = shell_exec(p->db, zSql, shell_callback, p, &zErrMsg);

				if( rc || zErrMsg ){
					char zPrefix[100];
					if( in!=0 || !stdin_is_interactive ){
						sqlite3_snprintf(sizeof(zPrefix), zPrefix, 
							"Error: near line %d:", startline);
					}else{
						sqlite3_snprintf(sizeof(zPrefix), zPrefix, "Error:");
					}
					if( zErrMsg!=0 ){
						fprintf(stderr, "%s %s\n", zPrefix, zErrMsg);
						sqlite3_free(zErrMsg);
						zErrMsg = 0;
					}else{
						fprintf(stderr, "%s %s\n", zPrefix, sqlite3_errmsg(p->db));
					}
					errCnt++;
				}
				free(zSql);
				zSql = 0;
				nSql = 0;
		}
	}
	if( zSql ){
		if( !_all_whitespace(zSql) ){
			fprintf(stderr, "Error: incomplete SQL: %s\n", zSql);
		}
		free(zSql);
	}
	free(zLine);
	return errCnt;
}

bool CppSQLite3DB::dump(const CString& strNewFileName)
{
    FILE* fp = fopen(strNewFileName.toLocal8Bit(), "wb");
    if (!fp)
	{
        TOLOG1("Failed to open file: %1", strNewFileName);
		return false;
	}
	//
	callback_data data;
	data.db = mpDB;
	data.out = fp;
	data.writableSchema = 0;
	data.cnt = 0;
	data.pStmt = NULL;
	//
	callback_data* p = &data;
	//
    fprintf(p->out, "PRAGMA foreign_keys=OFF;\n");
    fprintf(p->out, "BEGIN TRANSACTION;\n");
    p->writableSchema = 0;
    sqlite3_exec(p->db, "PRAGMA writable_schema=ON", 0, 0, 0);

	run_schema_dump_query(p, 
		"SELECT name, type, sql FROM sqlite_master "
		"WHERE sql NOT NULL AND type=='table' AND name!='sqlite_sequence'", 0
		);
	run_schema_dump_query(p, 
		"SELECT name, type, sql FROM sqlite_master "
		"WHERE name=='sqlite_sequence'", 0
		);
	run_table_dump_query(p->out, p->db,
		"SELECT sql FROM sqlite_master "
		"WHERE sql NOT NULL AND type IN ('index','trigger','view')", 0
		);
	//
	if( p->writableSchema ){
		fprintf(p->out, "PRAGMA writable_schema=OFF;\n");
		p->writableSchema = 0;
	}
	sqlite3_exec(p->db, "PRAGMA writable_schema=OFF", 0, 0, 0);
	fprintf(p->out, "COMMIT;\n");
	//
	fclose(fp);

	return true;
}

bool CppSQLite3DB::read(const CString& strNewFileName)
{
    FILE* fp = fopen(::WizBSTR2UTF8(strNewFileName).c_str(), "rb");
    if (!fp)
	{
        TOLOG1("Failed to open file: %1", strNewFileName);
		return false;
	}
	//
	callback_data data;
	data.db = mpDB;
	data.out = fp;
	data.writableSchema = 0;
	data.cnt = 0;
	data.pStmt = NULL;
	//
	callback_data* p = &data;
	//
	int errCnt = process_input(p, fp);
	//
	fclose(fp);
	//
	return 0 == errCnt;
}


bool CppSQLite3DB::repair(const CString& strDBFileName, const CString& strRetFileName)
{
	try
	{
        CString strTempFileName = Utils::WizPathResolve::tempPath() + WizIntToStr(WizGetTickCount()) + ".tmp";
		//
		CppSQLite3DB dbSrc;
        dbSrc.open(strDBFileName);
		//
		if (!dbSrc.dump(strTempFileName))
		{
			throw CppSQLite3Exception(-1, "Failed to dump database!");
		}
		//
		dbSrc.close();
		//
        if (WizPathFileExists(strRetFileName))
		{
            WizDeleteFile(strRetFileName);
		}
		//
		CppSQLite3DB dbDest;
        dbDest.open(strRetFileName);
		//
		if (!dbDest.read(strTempFileName))
		{
			throw CppSQLite3Exception(-1, "Failed to rebuild database form index!");
		}
		//
		dbDest.close();
		//
#ifdef Q_OS_WIN32
        _flushall();
#endif
		//
		return true;
	}
	catch (CppSQLite3Exception& e)
	{
         TOLOG(e.errorMessage());
		return false;
	}
	//
	return false;
}


