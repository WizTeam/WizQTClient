#include "WizAnalyzer.h"
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include "WizDef.h"
#include "WizMisc.h"
#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include "sync/WizApiEntry.h"
#include "share/WizEventLoop.h"
#include "share/WizThreads.h"
#include "share/jsoncpp/json/json.h"
#include "WizMainWindow.h"
#include "WizDatabase.h"
#include "WizDatabaseManager.h"

WizAnalyzer::WizAnalyzer(const CString& strRecordFileName)
    : m_strRecordFileName(strRecordFileName)
{

    m_strRecordFileNameNoDelete = Utils::WizMisc::extractFilePath(strRecordFileName) + Utils::WizMisc::extractFileTitle(strRecordFileName) + "Ex" + Utils::WizMisc::extractFileExt(strRecordFileName);
	//
	m_tLastLog = ::WizIniReadDateTimeDef(m_strRecordFileNameNoDelete, "Common", "Last", WizOleDateTime(2015, 1, 1, 0, 0, 0));
}

CString WizAnalyzer::guid()
{
	CString str = ::WizIniReadStringDef(m_strRecordFileNameNoDelete, "Common", "guid");
	if (str.isEmpty())
	{
		str = ::WizGenGUIDLowerCaseLetterOnly();
		::WizIniWriteString(m_strRecordFileNameNoDelete, "Common", "guid", str);
	}
	return str;
}
CString WizAnalyzer::getUseDays()
{
	int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, "Common", "useDays", 0);
	//
    return WizIntToStr(days);
}

CString WizAnalyzer::getInstallDays()
{
    int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, "Common", "useDays", 0);
    //
    if (days == 0)
    {
        QFileInfo info(QApplication::applicationFilePath());
        days = info.created().daysTo(QDateTime::currentDateTime());
        ::WizIniWriteInt(m_strRecordFileNameNoDelete, "Common", "useDays", days);
    }
    //
    return WizIntToStr(days);
}
void WizAnalyzer::increaseCounter(const CString& strSection, const CString& strKey)
{
    int count = ::WizIniReadIntDef(m_strRecordFileName, strSection, strKey, 0);
	count++;
    ::WizIniWriteInt(m_strRecordFileName, strSection, strKey, count);
}

void WizAnalyzer::addDuration(const CString& strFunctionName, int seconds)
{
    int count = ::WizIniReadIntDef(m_strRecordFileName, "Durations", strFunctionName, 0);
	count += seconds;
    ::WizIniWriteInt(m_strRecordFileName, "Durations", strFunctionName, count);
}

void WizAnalyzer::logTimes()
{
	WizOleDateTime t = ::WizGetCurrentTime();
	//
	int hour = t.getHour();
	int week = t.getDayOfWeek();
	if (week == 1)
		week = 7;
	else
		week = week - 1;
	//
	CString hourString = "Hour" + WizIntToStr(hour);
	increaseCounter("Actions", hourString);
	//
	CString weekString = "Week" + WizIntToStr(week);
	increaseCounter("Actions", weekString);
}
//
void WizAnalyzer::logUseDays()
{
	WizOleDateTime t = ::WizGetCurrentTime();
	//
	if (m_tLastLog.getYear() == t.getYear()
		&& m_tLastLog.getDayOfYear() == t.getDayOfYear())
		return;
	//
	int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, "Common", "useDays", 0);
	days++;
	::WizIniWriteInt(m_strRecordFileNameNoDelete, "Common", "useDays", days);
	//
	::WizIniWriteDateTime(m_strRecordFileNameNoDelete, "Common", "Last", t);
}
//
CString WizAnalyzer::keyOfFirstAction(int index)
{
	index++;
	//
	CString key = "firstAction";
	if (index >= 2)
	{
		key += WizIntToStr(index);
	}
	return key;
}
//
CString WizAnalyzer::getFirstAction(int index)
{
	CString strKey = keyOfFirstAction(index);
	CString strAction = ::WizIniReadStringDef(m_strRecordFileNameNoDelete, "firstAction", strKey);
	return strAction;
}
void WizAnalyzer::logFirstAction(const CString& strActionName)
{
    if (!strActionName || !*strActionName)
		return;
	//
    if (0 == WizStrStrI_Pos(strActionName, "Init"))
		return;
	//
	static BOOL logged = FALSE;
	if (logged)
		return;
	//
	for (int i = 0; i < 10; i++)
	{
		CString old = getFirstAction(i);
		if (old.isEmpty())
		{
			CString strKey = keyOfFirstAction(i);
            ::WizIniWriteString(m_strRecordFileNameNoDelete, "firstAction", strKey, strActionName);
			return;
		}
	}
	//
	logged = TRUE;
}

void WizAnalyzer::logAction(const CString& strAction)
{
    QMutexLocker locker(&m_csLog);
	//
	logUseDays();
	logTimes();
    logFirstAction(strAction);
	//
    increaseCounter("Actions", strAction);
}

void WizAnalyzer::logDurations(const CString& strAction, int seconds)
{
    QMutexLocker locker(&m_csLog);
    //
	logUseDays();
    increaseCounter("Functions", strAction);
    addDuration(strAction, seconds);
}
//
//
void WizAnalyzer::post(IWizSyncableDatabase* db)
{
    //
    class CPostRunnable
    {
    public:
        CPostRunnable(IWizSyncableDatabase* db)
            : m_db(db)
        {}

        //
        void run()
        {
            WizGetAnalyzer().postBlocked(m_db);
        }

    private:
        IWizSyncableDatabase* m_db;
    };

    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        CPostRunnable runnable(db);
        runnable.run();
    });
}


//
void WizAnalyzer::postBlocked(IWizSyncableDatabase* db)
{
    QByteArray buffer = constructUploadData(db);

    CString strURL = WizOfficialApiEntry::analyzerUploadUrl();

    if (0 != ::WizStrStrI_Pos(strURL, "http://")
        && 0 != ::WizStrStrI_Pos(strURL, "https://"))
        return;    

    QNetworkAccessManager net;
    QNetworkRequest request;
    request.setUrl(strURL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    QNetworkReply* reply = net.post(request, buffer);

    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    if (loop.error() != QNetworkReply::NoError || loop.result().isEmpty())
    {
        qDebug() << "[Analyzer]Upload failed!";
        return;
    }

    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(loop.result().constData(), d))
        return;

    if (!d.isMember("return_code"))
    {
        qDebug() << "[Analyzer]Can not get return code ";
        return;
    }

    int returnCode = d["return_code"].asInt();
    if (returnCode != 200)
    {
        qDebug() << "[Analyzer]Return code was not 200, error :  " << returnCode << loop.result();
        return;
    }
    else
    {
        qDebug() << "[Analyzer]Upload OK";
    }

//	//
    m_csLog.lock();
    ::WizDeleteFile(m_strRecordFileName);	//remove old file
    m_csLog.unlock();
}

QString analyzerFile()
{
    QString strFile = WizDatabaseManager::instance()->db().getAccountPath() + "analyzer.ini";
    return strFile;
}

WizAnalyzer& WizAnalyzer::getAnalyzer()
{    
    static WizAnalyzer analyzer(analyzerFile());
	//
    return analyzer;
}

QByteArray WizAnalyzer::constructUploadData(IWizSyncableDatabase* db)
{
    QMutexLocker locker(&m_csPost);

    Json::Value dd;

    dd["guid"] = guid().toStdString();

#if QT_VERSION >= 0x050400
    dd["platform"] = QSysInfo::prettyProductName().toStdString();
#else
    dd["platform"] = "Linux";
#endif


    dd["versionName"] = WIZ_CLIENT_VERSION;
    dd["versionCode"] = Utils::WizMisc::getVersionCode();
    //
    WizMainWindow *window = WizMainWindow::instance();
    QString local = QLocale::system().name();
    if (window)
    {
        local = window->userSettings().locale();
    }
    dd["locale"] = local.toStdString();

#ifdef Q_OS_MAC
    dd["deviceName"] = "MacOSX";
#ifdef BUILD4APPSTORE
    dd["packageType"] = "AppStore";
#else
    dd["packageType"] = "DMG";
#endif

#else
    dd["deviceName"] = "Linux";
#endif

    dd["isAnoymous"] = false;

    //
    dd["isBiz"] = db->hasBiz();

    int nUseDays = getUseDays().toInt();
    dd["useDays"] = nUseDays;

    int nInstallDays = getInstallDays().toInt();
    dd["installDays"] = nInstallDays;

    QDateTime dtSignUp = QDateTime::fromString(db->meta("Account", "DateSignUp"));
    int signUpDays = dtSignUp.daysTo(QDateTime::currentDateTime());
    dd["signUpDays"] = signUpDays;
    //
    QMap<QString, QString> firstActionMap;
    for (int i = 0; i < 10; i++)
    {
        CString strFirstAction = getFirstAction(i);
        if (!strFirstAction.isEmpty())
        {
            CString strKey = keyOfFirstAction(i);
            firstActionMap.insert(strKey, strFirstAction);
        }
        else
        {
            break;
        }
    }

    for (QMap<QString, QString>::iterator it = firstActionMap.begin();
        it != firstActionMap.end();
        it++)
    {
        QString key = it.key();
        QString value = it.value();
        dd[key.toUtf8().constData()] = value.toStdString();
    }

    QMutexLocker logLocker(&m_csLog);
    //
    WizIniFileEx iniFile;
    iniFile.loadFromFile(m_strRecordFileName);
    //
    Json::Value actions;
    //
    QMap<QString, QString> actionMap;
    iniFile.getSection("Actions", actionMap);
    for (QMap<QString, QString>::iterator it = actionMap.begin();
        it != actionMap.end();
        it++)
    {
        QString key = it.key();
        QString value = it.value();
        actions[key.toUtf8().constData()] = value.toStdString();
    }
    //
    dd["actions"] = actions;
    //
    Json::Value durations;
    //
    QMap<QString, QString> seconds;
    iniFile.getSection("Durations", seconds);
    //
    QMap<QString, QString> functionMap;

    iniFile.getSection("Functions", functionMap);
    for (QMap<QString, QString>::const_iterator it = functionMap.begin();
        it != functionMap.end();
        it++)
    {
        QString strKey = it.key();
        int sec = seconds.value(strKey).toInt();
        if (sec == 0)
            continue;

        Json::Value elem;

        elem["totalTime"] = sec;
        elem["count"] = it.value().toInt();
        //
        durations[strKey.toStdString()] = elem;
    }
    //
    dd["durations"] = durations;
    //
    Json::FastWriter writer;
    std::string ret = writer.write(dd);


    return QString::fromStdString(ret).toUtf8();
}



WizAnalyzer& WizGetAnalyzer()
{
	return WizAnalyzer::getAnalyzer();
}
////////////////////////////////////////////////////////
WizFunctionDurationLogger::WizFunctionDurationLogger(const CString& strFunctionName)
    : m_strFunctionName(strFunctionName)
{
    m_tStart = QTime::currentTime();
}

WizFunctionDurationLogger::~WizFunctionDurationLogger()
{
    QTime end = QTime::currentTime();
    if (end < m_tStart)
		return;
	//
    int seconds = m_tStart.secsTo(end); //(end - m_tStart) / 1000;
	//
	WizGetAnalyzer().logDurations(m_strFunctionName, seconds);
}

