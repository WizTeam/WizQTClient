#pragma once

//#include "WizKMDatabase.h"

#include "WizQtHelper.h"
#include "WizDatabase.h"
#include "WizDef.h"
#include <QDir>
#include <QMutex>

class WizAnalyzer
{
protected:
    WizAnalyzer(const CString& strRecordFileName);

	CString m_strRecordFileName;
	CString m_strRecordFileNameNoDelete;
	WizOleDateTime m_tLastLog;
    QMutex m_csLog;
    QMutex m_csPost;

    void increaseCounter(const CString& strSection, const CString& strKey);
    void addDuration(const CString& strFunctionName, int seconds);

	void logTimes();
	void logUseDays();
	//
	CString getFirstAction(int index);
    void logFirstAction(const CString& strActionName);
	CString keyOfFirstAction(int index);
	//
	CString guid();
	CString getUseDays();
    CString getInstallDays();
public:
    void logAction(const CString& strAction);
    void logDurations(const CString& strAction, int seconds);

    void post(IWizSyncableDatabase* db);
    void postBlocked(IWizSyncableDatabase* db);
    static WizAnalyzer& getAnalyzer();

private:
    QByteArray constructUploadData(IWizSyncableDatabase* db);

};

WizAnalyzer& WizGetAnalyzer();

class WizFunctionDurationLogger
{
public:
    WizFunctionDurationLogger(const CString& strFunctionName);
	virtual ~WizFunctionDurationLogger();
private:
	CString m_strFunctionName;
    QTime m_tStart;
};


#define LOG_ACTION(x)	WizGetAnalyzer().logAction(x)

#define LOG_FUNCTION_DURATION(x)	class CWizInnerFunctionDurationLogger : public WizFunctionDurationLogger\
{\
public:\
	CWizInnerFunctionDurationLogger() : WizFunctionDurationLogger(x) {}\
} m_wizDurationLogger
//

#define LOG_FUNCTION_DURATION_INLINE(x) WizFunctionDurationLogger __durationLogger(x)

/*
usage:
void buttonClicked()
{
	LOG_ACTION("NewNote");
}
//
class COptionsDialog : public CDialog
{
	LOG_FUNCTION_DURATION("Options");
};
*/
