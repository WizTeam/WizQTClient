#pragma once

//#include "WizKMDatabase.h"

#include "wizqthelper.h"
#include "wizDatabase.h"
#include "wizdef.h"
#include <QDir>
#include <QMutex>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

class CWizAnalyzer
{
protected:
    CWizAnalyzer(const CString& strRecordFileName);

	CString m_strRecordFileName;
	CString m_strRecordFileNameNoDelete;
	COleDateTime m_tLastLog;
    QMutex m_csLog;
    QMutex m_csPost;

    void IncreaseCounter(const CString& strSection, const CString& strKey);
    void AddDuration(const CString& strFunctionName, int seconds);

	void LogTimes();
	void LogUseDays();
	//
	CString GetFirstAction(int index);
    void LogFirstAction(const CString& strActionName);
	CString KeyOfFirstAction(int index);
	//
	CString guid();
	CString GetUseDays();
    CString GetInstallDays();
public:
    void LogAction(const CString& strAction);
    void LogDurations(const CString& strAction, int seconds);

    void Post(IWizSyncableDatabase* db);
    void PostBlocked(IWizSyncableDatabase* db);
    static CWizAnalyzer& GetAnalyzer();

};

CWizAnalyzer& WizGetAnalyzer();

class CWizFunctionDurationLogger
{
public:
    CWizFunctionDurationLogger(const CString& strFunctionName);
	virtual ~CWizFunctionDurationLogger();
private:
	CString m_strFunctionName;
    QTime m_tStart;
};


#define LOG_ACTION(x)	WizGetAnalyzer().LogAction(x)

#define LOG_FUNCTION_DURATION(x)	class CWizInnerFunctionDurationLogger : public CWizFunctionDurationLogger\
{\
public:\
	CWizInnerFunctionDurationLogger() : CWizFunctionDurationLogger(x) {}\
} m_wizDurationLogger
//

#define LOG_FUNCTION_DURATION_INLINE(x) CWizFunctionDurationLogger __durationLogger(x)

/*
usage:
void buttonClicked()
{
	LOG_ACTION(_T("NewNote"));
}
//
class COptionsDialog : public CDialog
{
	LOG_FUNCTION_DURATION(_T("Options"));
};
*/
