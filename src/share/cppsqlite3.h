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
#ifndef _CppSQLite3_H_
#define _CppSQLite3_H_

#include "sqlite3.h"
#include <cstdio>
#include <cstring>
#include "WizMisc.h"

#define CPPSQLITE_ERROR 1000

class CppSQLite3Exception
{
public:

    CppSQLite3Exception(const int nErrCode, const CString& errMessage);
    CppSQLite3Exception(const CppSQLite3Exception&  e);

    virtual ~CppSQLite3Exception() {}

    int errorCode() const { return mnErrCode; }
    const CString& errorMessage() const { return msErrMessage; }

    static const char* errorCodeAsString(int nErrCode);

public:
    static void throwException(int nErrCode, char* errMessage);

private:
    int mnErrCode;
    CString msErrMessage;
};


class CppSQLite3Query
{
public:

    CppSQLite3Query();

    CppSQLite3Query(const CppSQLite3Query& rQuery);

    CppSQLite3Query(sqlite3* pDB, sqlite3_stmt* pVM, bool bEof, bool bOwnVM=true);

    CppSQLite3Query& operator=(const CppSQLite3Query& rQuery);

    virtual ~CppSQLite3Query();

    int numFields();

    int fieldIndex(const CString& szField);
    CString fieldName(int nCol);

    CString fieldDeclType(int nCol);
    int fieldDataType(int nCol);

    const char* fieldValue(int nField);
    const char* fieldValue(const CString& szField);
    const unsigned short* fieldValue16(const CString& szField);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(const CString& szField, int nNullValue=0);

    __int64 getInt64Field(int nField, int nNullValue=0);
    __int64 getInt64Field(const CString& szField, int nNullValue=0);

    BOOL getBoolField(int nField, BOOL bNullValue=FALSE);
    BOOL getBoolField(const CString& szField, BOOL bNullValue=FALSE);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(const CString& szField, double fNullValue=0.0);

    WizOleDateTime getTimeField(int nField, time_t tNullValue = 0);
    WizOleDateTime getTimeField(const CString& szField, time_t tNullValue = 0);

    COLORREF getColorField(int nField, COLORREF crNullValue = 0);
    COLORREF getColorField(const CString& szField, COLORREF crNullValue = 0);

    QColor getColorField2(int nField, QColor crNullValue = QColor());
    QColor getColorField2(const CString& szField, QColor crNullValue = QColor());

    CString getStringField(int nField, const CString& szNullValue = "");
    CString getStringField(const CString& szField, const CString& szNullValue="");
    //
    int getColumnLength(int nField);

    const unsigned char* getBlobField(int nField, int& nLen);
    const unsigned char* getBlobField(const CString& szField, int& nLen);

    bool fieldIsNull(int nField);
    bool fieldIsNull(const CString& szField);

    bool eof();

    void nextRow();

    void finalize();


    void checkVM();

private:
	sqlite3* mpDB;
    sqlite3_stmt* mpVM;
    bool mbEof;
    int mnCols;
    bool mbOwnVM;
};



class CppSQLite3Statement
{
public:

    CppSQLite3Statement();

    CppSQLite3Statement(const CppSQLite3Statement& rStatement);

    CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pVM);

    virtual ~CppSQLite3Statement();

    CppSQLite3Statement& operator=(const CppSQLite3Statement& rStatement);

    int execDML();

    CppSQLite3Query execQuery();

    void bind(int nParam, const char* szValue);
    void bind(int nParam, const int nValue);
    void bind(int nParam, const double dwValue);
    void bind(int nParam, const unsigned char* blobValue, int nLen);
    void bindNull(int nParam);

    void reset();

    void finalize();

private:

    void checkDB();
    void checkVM();

    sqlite3* mpDB;
    sqlite3_stmt* mpVM;
};


class CppSQLite3DB
{
public:

    CppSQLite3DB();

    virtual ~CppSQLite3DB();

    void open(const CString& strFile);

    void close();

    bool tableExists(const CString& strTable);
    bool columnExists(const CString& strTable, const CString& strColumn);

    int execDML(const CString& strSQL);

    CppSQLite3Query execQuery(const CString& strSQL);

    int execScalar(const CString& strSQL);

    CppSQLite3Statement compileStatement(const CString& szSQL);

    sqlite_int64 lastRowId();

    void interrupt() { sqlite3_interrupt(mpDB); }

    void setBusyTimeout(int nMillisecs);

    static const char* SQLiteVersion() { return SQLITE_VERSION; }
	//
    BOOL isOpened();
	//
    int updateBlob(const CString& szTableName, const CString& szFieldName, const unsigned char* data, int dataLength, const CString& szWhere);
    int insertBlob(const CString& szSQL, const unsigned char* data, int dataLength);

    static bool repair(const CString& strDBFileName, const CString& strRetFileName);

private:

    CppSQLite3DB(const CppSQLite3DB& db);
    CppSQLite3DB& operator=(const CppSQLite3DB& db);

    sqlite3_stmt* compile(const CString& strSQL);

    void checkDB();

    sqlite3* mpDB;
    int mnBusyTimeoutMs;
	//
    bool dump(const CString& strNewFileName);
    bool read(const CString& strNewFileName);
};

#define STR2SQL(x)		WizStringToSQL(x)
#define TIME2SQL(x)		WizTimeToSQL(x)
#define COLOR2SQL(x)	WizColorToSQL(x)

#define STR2TIME(x)		WizStringToDateTime(x)
#define STR2COLOR(x)	WizStringToColor(x)

#define STR2SQL_LIKE_BOTH(x)		WizStringToSQL(WizFormatString1(_T("%%1%"), x))
#define STR2SQL_LIKE_RIGHT(x)		WizStringToSQL(WizFormatString1(_T("%1%_"), x))

#endif

