#include "wizAnalyzer.h"
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
#include "wizdef.h"
#include "wizmisc.h"
#include "utils/pathresolve.h"
#include "utils/misc.h"
#include "sync/apientry.h"
#include "share/wizEventLoop.h"
#include "share/wizthreads.h"
#include "wizmainwindow.h"
#include "wizDatabase.h"
#include "wizDatabaseManager.h"

WizAnalyzer::WizAnalyzer(const CString& strRecordFileName)
    : m_strRecordFileName(strRecordFileName)
{

    m_strRecordFileNameNoDelete = Utils::WizMisc::extractFilePath(strRecordFileName) + Utils::WizMisc::extractFileTitle(strRecordFileName) + _T("Ex") + Utils::WizMisc::extractFileExt(strRecordFileName);
	//
	m_tLastLog = ::WizIniReadDateTimeDef(m_strRecordFileNameNoDelete, _T("Common"), _T("Last"), WizOleDateTime(2015, 1, 1, 0, 0, 0));
}

CString WizAnalyzer::guid()
{
	CString str = ::WizIniReadStringDef(m_strRecordFileNameNoDelete, _T("Common"), _T("guid"));
	if (str.isEmpty())
	{
		str = ::WizGenGUIDLowerCaseLetterOnly();
		::WizIniWriteString(m_strRecordFileNameNoDelete, _T("Common"), _T("guid"), str);
	}
	return str;
}
CString WizAnalyzer::getUseDays()
{
	int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), 0);
	//
    return WizIntToStr(days);
}

CString WizAnalyzer::getInstallDays()
{
    int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), 0);
    //
    if (days == 0)
    {
        QFileInfo info(QApplication::applicationFilePath());
        days = info.created().daysTo(QDateTime::currentDateTime());
        ::WizIniWriteInt(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), days);
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
    int count = ::WizIniReadIntDef(m_strRecordFileName, _T("Durations"), strFunctionName, 0);
	count += seconds;
    ::WizIniWriteInt(m_strRecordFileName, _T("Durations"), strFunctionName, count);
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
	CString hourString = _T("Hour") + WizIntToStr(hour);
	increaseCounter(_T("Actions"), hourString);
	//
	CString weekString = _T("Week") + WizIntToStr(week);
	increaseCounter(_T("Actions"), weekString);
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
	int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), 0);
	days++;
	::WizIniWriteInt(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), days);
	//
	::WizIniWriteDateTime(m_strRecordFileNameNoDelete, _T("Common"), _T("Last"), t);
}
//
CString WizAnalyzer::keyOfFirstAction(int index)
{
	index++;
	//
	CString key = _T("firstAction");
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
	CString strAction = ::WizIniReadStringDef(m_strRecordFileNameNoDelete, _T("firstAction"), strKey);
	return strAction;
}
void WizAnalyzer::logFirstAction(const CString& strActionName)
{
    if (!strActionName || !*strActionName)
		return;
	//
    if (0 == WizStrStrI_Pos(strActionName, _T("Init")))
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
            ::WizIniWriteString(m_strRecordFileNameNoDelete, _T("firstAction"), strKey, strActionName);
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
    increaseCounter(_T("Actions"), strAction);
}

void WizAnalyzer::logDurations(const CString& strAction, int seconds)
{
    QMutexLocker locker(&m_csLog);
    //
	logUseDays();
    increaseCounter(_T("Functions"), strAction);
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

    CString strURL = WizApiEntry::analyzerUploadUrl();

    if (0 != ::WizStrStrI_Pos(strURL, _T("http://"))
        && 0 != ::WizStrStrI_Pos(strURL, _T("https://")))
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

    rapidjson::Document d;
    d.Parse<0>(loop.result().constData());

    if (!d.HasMember("return_code"))
    {
        qDebug() << "[Analyzer]Can not get return code ";
        return;
    }

    int returnCode = d.FindMember("return_code")->value.GetInt();
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
    ::DeleteFile(m_strRecordFileName);	//remove old file
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

    rapidjson::Document dd;
    dd.SetObject();
    rapidjson::Document::AllocatorType& allocator = dd.GetAllocator();

    QByteArray baGuid = guid().toUtf8();
    rapidjson::Value vGuid(baGuid.constData(), baGuid.size());
    dd.AddMember("guid", vGuid, allocator);

#if QT_VERSION >= 0x050400
    QByteArray baPlat = QSysInfo::prettyProductName().toUtf8();
    rapidjson::Value vPlat(baPlat.constData(), baPlat.size());
    dd.AddMember("platform", vPlat, allocator);
#else
    dd.AddMember("platform", "Linux", allocator);
#endif


    rapidjson::Value versionName(rapidjson::kStringType);
    versionName.SetString(WIZ_CLIENT_VERSION);
    dd.AddMember("versionName", versionName, allocator);

    dd.AddMember("versionCode", Utils::WizMisc::getVersionCode(), allocator);

    //
    WizMainWindow *window = WizMainWindow::instance();
    QByteArray baLocal = QLocale::system().name().toUtf8();
    if (window)
    {
        baLocal = window->userSettings().locale().toUtf8();
    }
    rapidjson::Value locale(baLocal.constData(), baLocal.size());
    dd.AddMember("locale", locale, allocator);

//    //  only used for phone
//    dd.AddMember("screenSize", 13, allocator);

#ifdef Q_OS_MAC
    dd.AddMember("deviceName", "MacOSX", allocator);
#ifdef BUILD4APPSTORE
    dd.AddMember("packageType", "AppStore", allocator);
#else
    dd.AddMember("packageType", "DMG", allocator);
#endif

#else
    dd.AddMember("deviceName", "Linux", allocator);
#endif

    rapidjson::Value isAnoymous(false);
    dd.AddMember("isAnoymous", isAnoymous, allocator);

    //
    rapidjson::Value isBiz(db->hasBiz());
    dd.AddMember("isBiz", isBiz, allocator);

    int nUseDays = getUseDays().toInt();
    dd.AddMember("useDays", nUseDays, allocator);

    int nInstallDays = getInstallDays().toInt();
    dd.AddMember("installDays", nInstallDays, allocator);

    QDateTime dtSignUp = QDateTime::fromString(db->meta("Account", "DateSignUp"));
    int signUpDays = dtSignUp.daysTo(QDateTime::currentDateTime());
    dd.AddMember("signUpDays", signUpDays, allocator);

    //

    QMap<QByteArray, QByteArray> firstActionMap;
    for (int i = 0; i < 10; i++)
    {
        CString strFirstAction = getFirstAction(i);
        if (!strFirstAction.isEmpty())
        {
            CString strKey = keyOfFirstAction(i);
            firstActionMap.insert(strKey.toUtf8(), strFirstAction.toUtf8());
        }
        else
        {
            break;
        }
    }

    for (int i = 0; i < firstActionMap.count(); i++)
    {
        const QByteArray& baKey = firstActionMap.keys().at(i);
        const QByteArray& baValue = firstActionMap[baKey];
        rapidjson::Value fistAction(baValue.constData(), baValue.size());
        rapidjson::Value vKey(baKey.constData(), baKey.size());
        dd.AddMember(vKey, fistAction, allocator);
    }

    QMutexLocker logLocker(&m_csLog);
    //
    WizIniFileEx iniFile;
    iniFile.loadFromFile(m_strRecordFileName);
    //
    rapidjson::Value actions(rapidjson::kObjectType);
    actions.SetObject();
    //
    QMap<QByteArray, QByteArray> actionMap;
    iniFile.getSection(_T("Actions"), actionMap);
    for (QMap<QByteArray, QByteArray>::iterator it = actionMap.begin();
        it != actionMap.end();
        it++)
    {
        const QByteArray& baKey = it.key();
        rapidjson::Value vValue(it.value().toInt());
        rapidjson::Value vKey(baKey.constData(), baKey.size());
        actions.AddMember(vKey, vValue, allocator);
    }
    //
    dd.AddMember("actions", actions, allocator);
    //
    rapidjson::Value durations(rapidjson::kObjectType);
    durations.SetObject();
    //
    QMap<QString, QString> seconds;
    iniFile.getSection(_T("Durations"), seconds);
    //
    QMap<QByteArray, QByteArray> functionMap;

    iniFile.getSection(_T("Functions"), functionMap);
    for (QMap<QByteArray, QByteArray>::const_iterator it = functionMap.begin();
        it != functionMap.end();
        it++)
    {
        QString strKey = it.key();
        int sec = seconds.value(strKey).toInt();
        if (sec == 0)
            continue;

        rapidjson::Value elem(rapidjson::kObjectType);
        elem.SetObject();

        elem.AddMember("totalTime", sec, allocator);
        elem.AddMember("count", it.value().toInt(), allocator);
        //
        rapidjson::Value vKey(it.key().constData(), it.key().size());
        durations.AddMember(vKey, elem, allocator);
    }
    //
    dd.AddMember("durations", durations, allocator);
    //

    rapidjson::GenericStringBuffer< rapidjson::UTF8<> > buffer;
    rapidjson::Writer<rapidjson::GenericStringBuffer< rapidjson::UTF8<> > > writer(buffer);

    dd.Accept(writer);

    return buffer.GetString();
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

