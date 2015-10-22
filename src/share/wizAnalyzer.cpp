#include "wizAnalyzer.h"
#include <QMutexLocker>
#include <QRunnable>
#include <QThreadPool>
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
#include "wizmainwindow.h"
#include "wizDatabase.h"
#include "wizDatabaseManager.h"
#include "share/wizEventLoop.h"

CWizAnalyzer::CWizAnalyzer(const CString& strRecordFileName)
    : m_strRecordFileName(strRecordFileName)
{

    m_strRecordFileNameNoDelete = Utils::Misc::extractFilePath(strRecordFileName) + Utils::Misc::extractFileTitle(strRecordFileName) + _T("Ex") + Utils::Misc::extractFileExt(strRecordFileName);
	//
	m_tLastLog = ::WizIniReadDateTimeDef(m_strRecordFileNameNoDelete, _T("Common"), _T("Last"), COleDateTime(2015, 1, 1, 0, 0, 0));
}

CString CWizAnalyzer::guid()
{
	CString str = ::WizIniReadStringDef(m_strRecordFileNameNoDelete, _T("Common"), _T("guid"));
	if (str.IsEmpty())
	{
		str = ::WizGenGUIDLowerCaseLetterOnly();
		::WizIniWriteString(m_strRecordFileNameNoDelete, _T("Common"), _T("guid"), str);
	}
	return str;
}
CString CWizAnalyzer::GetUseDays()
{
	int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), 0);
	//
    return WizIntToStr(days);
}

CString CWizAnalyzer::GetInstallDays()
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
void CWizAnalyzer::IncreaseCounter(const CString& strSection, const CString& strKey)
{
    int count = ::WizIniReadIntDef(m_strRecordFileName, strSection, strKey, 0);
	count++;
    ::WizIniWriteInt(m_strRecordFileName, strSection, strKey, count);
}

void CWizAnalyzer::AddDuration(const CString& strFunctionName, int seconds)
{
    int count = ::WizIniReadIntDef(m_strRecordFileName, _T("Durations"), strFunctionName, 0);
	count += seconds;
    ::WizIniWriteInt(m_strRecordFileName, _T("Durations"), strFunctionName, count);
}

void CWizAnalyzer::LogTimes()
{
	COleDateTime t = ::WizGetCurrentTime();
	//
	int hour = t.GetHour();
	int week = t.GetDayOfWeek();
	if (week == 1)
		week = 7;
	else
		week = week - 1;
	//
	CString hourString = _T("Hour") + WizIntToStr(hour);
	IncreaseCounter(_T("Actions"), hourString);
	//
	CString weekString = _T("Week") + WizIntToStr(week);
	IncreaseCounter(_T("Actions"), weekString);
}
//
void CWizAnalyzer::LogUseDays()
{
	COleDateTime t = ::WizGetCurrentTime();
	//
	if (m_tLastLog.GetYear() == t.GetYear()
		&& m_tLastLog.GetDayOfYear() == t.GetDayOfYear())
		return;
	//
	int days = ::WizIniReadIntDef(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), 0);
	days++;
	::WizIniWriteInt(m_strRecordFileNameNoDelete, _T("Common"), _T("useDays"), days);
	//
	::WizIniWriteDateTime(m_strRecordFileNameNoDelete, _T("Common"), _T("Last"), t);
}
//
CString CWizAnalyzer::KeyOfFirstAction(int index)
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
CString CWizAnalyzer::GetFirstAction(int index)
{
	CString strKey = KeyOfFirstAction(index);
	CString strAction = ::WizIniReadStringDef(m_strRecordFileNameNoDelete, _T("firstAction"), strKey);
	return strAction;
}
void CWizAnalyzer::LogFirstAction(const CString& strActionName)
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
		CString old = GetFirstAction(i);
		if (old.IsEmpty())
		{
			CString strKey = KeyOfFirstAction(i);
            ::WizIniWriteString(m_strRecordFileNameNoDelete, _T("firstAction"), strKey, strActionName);
			return;
		}
	}
	//
	logged = TRUE;
}

void CWizAnalyzer::LogAction(const CString& strAction)
{
    QMutexLocker locker(&m_csLog);
	//
	LogUseDays();
	LogTimes();
    LogFirstAction(strAction);
	//
    IncreaseCounter(_T("Actions"), strAction);
}

void CWizAnalyzer::LogDurations(const CString& strAction, int seconds)
{
    QMutexLocker locker(&m_csLog);
    //
	LogUseDays();
    IncreaseCounter(_T("Functions"), strAction);
    AddDuration(strAction, seconds);
}
//
//
void CWizAnalyzer::Post(IWizSyncableDatabase* db)
{
    //
    class CPostRunable : public QRunnable
    {
    public:
        CPostRunable(IWizSyncableDatabase* db)
            : m_db(db)
        {}

        //
        void run()
        {
            WizGetAnalyzer().PostBlocked(m_db);
        }

    private:
        IWizSyncableDatabase* m_db;
    };

    QThreadPool::globalInstance()->start(new CPostRunable(db));
}


//
void CWizAnalyzer::PostBlocked(IWizSyncableDatabase* db)
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

    dd.AddMember("versionCode", Utils::Misc::getVersionCode(), allocator);

    //
    Core::Internal::MainWindow *window = Core::Internal::MainWindow::instance();
    QByteArray baLocal = QLocale::system().name().toUtf8();
    if (window)
    {
        baLocal = window->userSettings().locale().toUtf8();
    }
    rapidjson::Value locale(baLocal.constData(), baLocal.size());
    dd.AddMember("locale", locale, allocator);

    //  only used for phone
    dd.AddMember("screenSize", 13, allocator);

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
    rapidjson::Value isBiz(db->HasBiz());
    dd.AddMember("isBiz", isBiz, allocator);

    int nUseDays = GetUseDays().toInt();
    dd.AddMember("useDays", nUseDays, allocator);

    int nInstallDays = GetInstallDays().toInt();
    dd.AddMember("installDays", nInstallDays, allocator);

    QDateTime dtSignUp = QDateTime::fromString(db->meta("Account", "DateSignUp"));
    int signUpDays = dtSignUp.daysTo(QDateTime::currentDateTime());
    dd.AddMember("signUpDays", signUpDays, allocator);

	//

    QMap<QByteArray, QByteArray> firstActionMap;
	for (int i = 0; i < 10; i++)
	{
        CString strFirstAction = GetFirstAction(i);
        if (!strFirstAction.IsEmpty())
        {
            CString strKey = KeyOfFirstAction(i);
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

	//
	CWizIniFileEx iniFile;
	iniFile.LoadFromFile(m_strRecordFileName);
	//
    rapidjson::Value actions(rapidjson::kObjectType);
    actions.SetObject();
	//
    QMap<QByteArray, QByteArray> actionMap;
    iniFile.GetSection(_T("Actions"), actionMap);
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
    iniFile.GetSection(_T("Durations"), seconds);
	//
    QMap<QByteArray, QByteArray> functionMap;

    iniFile.GetSection(_T("Functions"), functionMap);
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


    CString strURL = WizService::WizApiEntry::analyzerUploadUrl();

    if (0 != ::WizStrStrI_Pos(strURL, _T("http://"))
        && 0 != ::WizStrStrI_Pos(strURL, _T("https://")))
        return;    

    QNetworkAccessManager net;
    QNetworkRequest request;
    request.setUrl(strURL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    QNetworkReply* reply = net.post(request, buffer.GetString());    

    CWizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "[Analyzer]Upload failed!";
        return;
    }

    rapidjson::Document d;
    d.Parse<0>(loop.result().toUtf8().constData());

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
    ::DeleteFile(m_strRecordFileName);	//remove old file
}

QString analyzerFile()
{
    QString strFile = CWizDatabaseManager::instance()->db().GetAccountPath() + "analyzer.ini";
    return strFile;
}

CWizAnalyzer& CWizAnalyzer::GetAnalyzer()
{    
    static CWizAnalyzer analyzer(analyzerFile());
	//
    return analyzer;
}



CWizAnalyzer& WizGetAnalyzer()
{
	return CWizAnalyzer::GetAnalyzer();
}
////////////////////////////////////////////////////////
CWizFunctionDurationLogger::CWizFunctionDurationLogger(const CString& strFunctionName)
    : m_strFunctionName(strFunctionName)
{
    m_tStart = QTime::currentTime();
}

CWizFunctionDurationLogger::~CWizFunctionDurationLogger()
{
    QTime end = QTime::currentTime();
    if (end < m_tStart)
		return;
	//
    int seconds = m_tStart.secsTo(end); //(end - m_tStart) / 1000;
	//
	WizGetAnalyzer().LogDurations(m_strFunctionName, seconds);
}

