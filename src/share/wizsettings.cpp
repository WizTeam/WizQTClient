#include "wizsettings.h"
#include "utils/pathresolve.h"

#include "wizdef.h"
#include <QLocale>

CWizSettings::CWizSettings(const QString& strFileName)
    : QSettings(strFileName, QSettings::IniFormat)
{
    setIniCodec("utf-8");
}

QString CWizSettings::GetString(const QString& strSection, const QString& strKey, const QString& strDef /*= ""*/)
{
    QVariant v = value(strSection + "/" + strKey, strDef);
    return v.toString();
}

BOOL CWizSettings::SetString(const QString& strSection, const QString& strKey, const QString& str)
{
    setValue(strSection + "/" + strKey, str);
    return TRUE;
}

int CWizSettings::GetInt(const QString& strSection, const QString& strKey, int nDef /*= 0*/)
{
    QString str = GetString(strSection, strKey, WizIntToStr(nDef));
    int n = _ttoi(str);
    if (WizIntToStr(n) != str)
        return nDef;

    return n;
}

BOOL CWizSettings::SetInt(const QString& strSection, const QString& strKey, int val)
{
    return SetString(strSection, strKey, WizIntToStr(val));
}

BOOL CWizSettings::GetBool(const QString& strSection, const QString& strKey, bool def /* = false */)
{
    return GetInt(strSection, strKey, def ? 1 : 0) != 0;
}

BOOL CWizSettings::SetBool(const QString& strSection, const QString& strKey, bool val)
{
    return SetInt(strSection, strKey, val ? 1 : 0);
}

QColor CWizSettings::GetColor(const QString& strSection,
                              const QString& strKey,
                              QColor defColor /* = "FFFFFF" */)
{
    QString str = GetString(strSection, strKey, "");
    if (str.isEmpty())
        return defColor;

    return WizStringToColor2(str);
}

QString CWizSettings::GetEncryptedString(const QString& strSection, const QString& strKey, const QString& strDef /*= ""*/)
{
    return WizStringFromBase64(GetString(strSection, strKey, strDef));
}

BOOL CWizSettings::SetEncryptedString(const QString& strSection, const QString& strKey, const QString& str)
{
    return SetString(strSection, strKey, WizStringToBase64(str));
}

void CWizSettings::GetSections(CWizStdStringArray& arrayAction)
{
    QStringList sl = childGroups();
    arrayAction.assign(sl.begin(), sl.end());
}

void CWizSettings::GetKeys(const QString& strSection, CWizStdStringArray& arrayAction)
{
    beginGroup(strSection);
    QStringList sl = childKeys();
    endGroup();
    arrayAction.assign(sl.begin(), sl.end());
}


QString CWizSettings::GetProxyHost()
{
    return GetString("Sync", "ProxyHost");
}

void CWizSettings::SetProxyHost(const QString& val)
{
    SetString("Sync", "ProxyHost", val);
}

WizProxyType CWizSettings::GetProxyType()
{
    int port = GetInt("Sync", "ProxyType", -1);
    if (port < 0)
        port = WizProxy_HttpProxy;

    return (WizProxyType)port;
}

void CWizSettings::SetProxyType(WizProxyType type)
{
    SetInt("Sync", "ProxyType", type);
}

int CWizSettings::GetProxyPort()
{
    int port = GetInt("Sync", "ProxyPort", 0);
    if (port <= 0)
        port = 80;

    return port;
}

void CWizSettings::SetProxyPort(int val)
{
    SetInt("Sync", "ProxyPort", val);
}

QString CWizSettings::GetProxyUserName()
{
    return GetString("Sync", "ProxyUserName");
}

void CWizSettings::SetProxyUserName(const QString& val)
{
    SetString("Sync", "ProxyUserName", val);
}

QString CWizSettings::GetProxyPassword()
{
    return GetString("Sync", "ProxyPassword");
}

void CWizSettings::SetProxyPassword(const QString& val)
{
    SetString("Sync", "ProxyPassword", val);
}

bool CWizSettings::GetProxyStatus()
{
    return GetBool("Sync", "ProxyStatus", false);
}

void CWizSettings::SetProxyStatus(bool val)
{
    SetBool("Sync", "ProxyStatus", val);
}


CString WizGetShortcut(const CString& strName, const CString& strDef /*= ""*/)
{
    CWizSettings settings(Utils::PathResolve::globalSettingsFile());
    return settings.GetString("Shortcut", strName, strDef);
}


CWizUserSettings::CWizUserSettings(const QString& strAccountFolderName)
    : m_strAccountFolderName(strAccountFolderName)
    , m_db(NULL)
{
}

CWizUserSettings::CWizUserSettings(CWizDatabase& db)
    : m_db(&db)
{
}

void CWizUserSettings::setAccountFolderName(const QString& strAccountFolderName)
{
    if (!m_db) {
        m_strAccountFolderName = strAccountFolderName;
        m_strSkinName.clear();
        m_strLocale.clear();
    }
}

QString CWizUserSettings::myWizMail() const
{
    return get("ACCOUNT", "MYWIZMAIL");
}

QString CWizUserSettings::get(const QString& section, const QString& strKey) const
{
    if (!m_strAccountFolderName.isEmpty()) {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            return db.GetMetaDef(section, strKey);
        }
    }

    if (m_db) {
        return m_db->GetMetaDef(section, strKey);
    }

    return NULL;
}

void CWizUserSettings::set(const QString& section, const QString& strKey, const QString& strValue)
{
    if (!m_strAccountFolderName.isEmpty()) {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            db.SetMeta(section, strKey, strValue);
            return;
        }
    }

    if (m_db) {
        m_db->SetMeta(section, strKey, strValue);
        return;
    }
}

QString CWizUserSettings::userId() const
{
    if (m_db)
        return m_db->GetUserId();

    if (!m_strAccountFolderName.isEmpty()) {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            return db.GetUserId();
        }
    }

    return QString();
}

void CWizUserSettings::setUserId(const QString& strUserId)
{
    if (m_db)
    {
        m_db->SetMeta("Account", "USERID", strUserId);
    }
    else if (!m_strAccountFolderName.isEmpty())
    {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            db.SetMeta("Account", "USERID", strUserId);
        }
    }
}

QString CWizUserSettings::get(const QString& strKey) const
{
    if (!m_strAccountFolderName.isEmpty()) {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            return db.GetMetaDef(USER_SETTINGS_SECTION, strKey);
        }
    }

    if (m_db) {
        return m_db->GetMetaDef(USER_SETTINGS_SECTION, strKey);
    }

    return NULL;
}

void CWizUserSettings::set(const QString& strKey, const QString& strValue)
{
    if (!m_strAccountFolderName.isEmpty()) {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            db.SetMeta(USER_SETTINGS_SECTION, strKey, strValue);
            return;
        }
    }

    if (m_db) {
        m_db->SetMeta(USER_SETTINGS_SECTION, strKey, strValue);
        return;
    }
}

QString CWizUserSettings::password() const
{
    QString strPassword;
    if (!m_strAccountFolderName.isEmpty()) {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            strPassword = db.GetMetaDef("Account", "Password");
        }
    }

    if (m_db) {
        strPassword = m_db->GetMetaDef("Account", "Password");
    }

    return ::WizDecryptPassword(strPassword);
}

void CWizUserSettings::setPassword(const QString& strPassword /* = NULL */)
{
    if (!m_strAccountFolderName.isEmpty()) {
        CWizDatabase db;
        if (db.Open(m_strAccountFolderName)) {
            db.SetMeta("Account", "Password", strPassword);
            return;
        }
    }

    if (m_db) {
        m_db->SetMeta("Account", "Password", strPassword);
        return;
    }
}

WizServerType CWizUserSettings::serverType() const
{
    QString strServerType = get("ServerType");
    if (strServerType.isEmpty()) {
        return NoServer;
    }

    return WizServerType(strServerType.toInt());
}

void CWizUserSettings::setServerType(WizServerType server)
{
    QString strServerType = QString::number(server);
    set("ServerType", strServerType);
}

QString CWizUserSettings::enterpriseServerIP()
{
    QString strServerType = get("EnterpriseServerIP");
    return strServerType;
}

void CWizUserSettings::setEnterpriseServerIP(const QString& strEnterpriseServerd)
{
    set("EnterpriseServerIP", strEnterpriseServerd);
}

QString CWizUserSettings::serverLicence()
{
    QString strServerType = get("ServerLicence");
    return strServerType;
}

void CWizUserSettings::setServerLicence(const QString& strLicence)
{
    set("ServerLicence", strLicence);
}

bool CWizUserSettings::autoLogin() const
{
    QString strAutoLogin = get("AutoLogin");
    if (!strAutoLogin.isEmpty()) {
        return strAutoLogin.toInt() ? true : false;
    }

    return false;
}

void CWizUserSettings::setAutoLogin(bool bAutoLogin)
{
    set("AutoLogin", bAutoLogin ? "1" : "0");
}

bool CWizUserSettings::autoCheckUpdate() const
{
    QString strAutoCheckUpdate = get("AutoCheckUpdate");
    if (!strAutoCheckUpdate.isEmpty()) {
        return strAutoCheckUpdate.toInt() ? true : false;
    }

    return true;
}

void CWizUserSettings::setAutoCheckUpdate(bool bAutoCheckUpdate)
{
    set("AutoCheckUpdate", bAutoCheckUpdate ? "1" : "0");
}

bool CWizUserSettings::showSystemTrayIcon() const
{
    QString strShowTrayIcon = get("ShowSystemTrayIcon");
    if (!strShowTrayIcon.isEmpty()) {
        return strShowTrayIcon.toInt() ? true : false;
    }

    return true;
}

void CWizUserSettings::setShowSystemTrayIcon(bool bShowTrayIcon)
{
    set("ShowSystemTrayIcon", bShowTrayIcon ? "1" : "0");
}

bool CWizUserSettings::useSystemBasedStyle() const
{
    QString strUseSystemStyle = get("UseSystemBasedStyle");
    if (!strUseSystemStyle.isEmpty()) {
        return strUseSystemStyle.toInt() ? true : false;
    }

    return true;
}

void CWizUserSettings::setUseSystemBasedStyle(bool bSystemStyle)
{
    set("UseSystemBasedStyle", bSystemStyle ? "1" : "0");
}

bool CWizUserSettings::receiveMobileFile() const
{
    QString strReceiveMobileFile = get("RecevieMobileFile");
    if (!strReceiveMobileFile.isEmpty()) {
        return strReceiveMobileFile.toInt() ? true : false;
    }

    return false;
}

void CWizUserSettings::setReceiveMobileFile(bool bReceiveFile)
{
    set("RecevieMobileFile", bReceiveFile ? "1" : "0");
}

double CWizUserSettings::printMarginValue(WizPositionType posType)
{
    QString strMarginValue = get("PrintMarginValue_" + QString::number(posType));
    if (strMarginValue.isEmpty()) {
        return 5.0;
    }

    return strMarginValue.toDouble();
}

void CWizUserSettings::setPrintMarginValue(WizPositionType posType, double dValue)
{
    set("PrintMarginValue_" + QString::number(posType), QString::number(dValue));
}

int CWizUserSettings::printMarginUnit()
{
    QString strMarginType = get("PrintMarginType");
    if (strMarginType.isEmpty()) {
        return 0;
    }

    return strMarginType.toInt();
}

void CWizUserSettings::setPrintMarginUnit(int nUnit)
{
    set("PrintMarginType", QString::number(nUnit));
}

QString CWizUserSettings::newFeatureGuideVersion()
{
    QString strGuideVersion = get("NewFeatureGuideVersion");
    return strGuideVersion;
}

void CWizUserSettings::setNewFeatureGuideVersion(const QString& strGuideVersion)
{
    set("NewFeatureGuideVersion", strGuideVersion);
}

bool CWizUserSettings::needShowMobileFileReceiverUserGuide()
{
    QString strShowGuide = get("ShowMobileFileReceiverUserGuide");
    if (!strShowGuide.isEmpty()) {
        return strShowGuide.toInt() ? true : false;
    }

    return true;
}

void CWizUserSettings::setNeedShowMobileFileReceiverUserGuide(bool bNeedShow)
{
    set("ShowMobileFileReceiverUserGuide", bNeedShow ? "1" : "0");
}

bool CWizUserSettings::searchEncryptedNote()
{
    QString strShowGuide = get("SearchEncryptedNote");
    if (!strShowGuide.isEmpty()) {
        return strShowGuide.toInt() ? true : false;
    }

    return false;
}

void CWizUserSettings::setSearchEncryptedNote(bool bSearchEncryNote)
{
    set("SearchEncryptedNote", bSearchEncryNote ? "1" : "0");
}

QString CWizUserSettings::encryptedNotePassword()
{
    QString strPassword = get("EncryptedNotePassword");
    return ::WizDecryptPassword(strPassword);
}

void CWizUserSettings::setEncryptedNotePassword(const QString& strPassword)
{
    QString strEncryptPass = ::WizEncryptPassword(strPassword);
    set("EncryptedNotePassword", strEncryptPass);
}

static bool rememberPasswordForSession = false;
bool CWizUserSettings::isRememberNotePasswordForSession()
{
    return rememberPasswordForSession;
}

void CWizUserSettings::setRememberNotePasswordForSession(bool remember)
{
    rememberPasswordForSession = remember;
}

QString CWizUserSettings::editorBackgroundColor()
{
    QString strColor = get("EditorBackgroundColor");    
    return strColor;
}

void CWizUserSettings::setEditorBackgroundColor(const QString& strColor)
{
    set("EditorBackgroundColor", strColor);
}

bool CWizUserSettings::isManualSortingEnabled()
{
    QString strManualSortingEnable = get("ManualSortingEnable");
    if (!strManualSortingEnable.isEmpty()) {
        return strManualSortingEnable.toInt() ? true : false;
    }

#ifdef Q_OS_LINUX
    return false;
#else
    return true;
#endif
}

void CWizUserSettings::setManualSortingEnable(bool bEnable)
{
    set("ManualSortingEnable", bEnable ? "1" : "0");
}

QString CWizUserSettings::skin()
{
    // just return because no skin selection from v1.4
    return "default";

    if (!m_strSkinName.isEmpty())
        return m_strSkinName;

    QString strSkinName = get("Skin");

    if (!strSkinName.isEmpty()) {
        if (PathFileExists(::WizGetSkinResourcePath(strSkinName))) {
            m_strSkinName = strSkinName;
            return strSkinName;
        }
    }

    //CWizSettings settings(::WizGetDataStorePath() + "wiznote.ini");
    //strSkinName = settings.GetString("Common", "Skin");

    //if (!strSkinName.isEmpty()) {
    //    if (PathFileExists(::WizGetSkinResourcePath(strSkinName))) {
    //        m_strSkinName = strSkinName;
    //        return strSkinName;
    //    }
    //}

    // default skin depends on user's OS
    return ::WizGetDefaultSkinName();
}

void CWizUserSettings::setSkin(const QString& strSkinName)
{
    Q_ASSERT(!strSkinName.isEmpty());

    m_strSkinName = strSkinName;
    set("Skin", strSkinName);
}

QString CWizUserSettings::locale()
{
    if (!m_strLocale.isEmpty()) {
        return m_strLocale;
    }

    QString strLocale = get("Locale");

    if (!strLocale.isEmpty()) {
        m_strLocale = strLocale;
        return strLocale;
    }

    //CWizSettings settings(::WizGetDataStorePath() + "wiznote.ini");
    //strLocale = settings.GetString("Common", "Locale");

    //if (!strLocale.isEmpty()) {
    //    m_strLocale = strLocale;
    //    return strLocale;
    //}

    strLocale = QLocale::system().name();
    if (::PathFileExists(Utils::PathResolve::localeFileName(strLocale))) {
        m_strLocale = strLocale;
        return strLocale;
    }

    return WizGetDefaultTranslatedLocal();
}

void CWizUserSettings::setLocale(const QString& strLocale)
{
    Q_ASSERT(!strLocale.isEmpty());

    m_strLocale = strLocale;
    set("Locale", strLocale);
}

QString CWizUserSettings::defaultFontFamily()
{
    QString strFont = get("DefaultFontFamily");
    if (!strFont.isEmpty())
        return strFont;

#ifdef Q_OS_MAC
    return "Helvetica Neue";
#else
    return "Arial";
#endif
}

void CWizUserSettings::setDefaultFontFamily(const QString& strFont)
{
    set("DefaultFontFamily", strFont);
}

int CWizUserSettings::defaultFontSize()
{
    int nSize = get("DefaultFontSize").toInt();
    if (nSize)
        return nSize;

    return 15; // default 15px
}

void CWizUserSettings::setDefaultFontSize(int nSize)
{
    set("DefaultFontSize", QString::number(nSize));
}

WizDocumentViewMode CWizUserSettings::noteViewMode() const
{
    QString mode = get("NoteViewMode");

    if (!mode.isEmpty()) {
        return WizDocumentViewMode(mode.toInt());
    }

    return viewmodeAlwaysReading;
}

int CWizUserSettings::syncInterval() const
{
    int nInterval = get("SyncInterval").toInt();
    if (!nInterval)
        return 15; // default 15 minutes

    return nInterval;
}

void CWizUserSettings::setSyncInterval(int minutes)
{
    set("SyncInterval", QString::number(minutes));
}

int CWizUserSettings::syncMethod() const
{
    int nDays = get("SyncMethod").toInt();
    if (!nDays)
        return 99999; // default all

    return nDays;
}

void CWizUserSettings::setSyncMethod(int days)
{
    set("SyncMethod", QString::number(days));
}

int CWizUserSettings::syncGroupMethod() const
{
    int nDays = get("SyncGroupMethod").toInt();
    if (!nDays)
        return 1;   // default 1 day

    return nDays;
}

void CWizUserSettings::setSyncGroupMethod(int days)
{
    set("SyncGroupMethod", QString::number(days));
}

void CWizUserSettings::appendRecentSearch(const QString& search)
{
    if (search.isEmpty())
        return;

    QStringList recentSearches = getRecentSearches();
    while (recentSearches.count() >= 5)
        recentSearches.pop_front();

    recentSearches.append(search);
    QString searches = recentSearches.join("/");
    set("RecentSearches", searches);
}

QStringList CWizUserSettings::getRecentSearches(bool reverseOrder)
{
    QStringList recentSearches = get("RecentSearches").split('/', QString::SkipEmptyParts);

    if (reverseOrder)
    {
        QStringList reverseList;
        for (QString str : recentSearches)
        {
            reverseList.push_front(str);
        }
        return reverseList;
    }

    return recentSearches;
}
