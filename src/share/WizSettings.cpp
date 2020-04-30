#include "WizSettings.h"
#include "utils/WizPathResolve.h"
#include "WizUIBase.h"

#include "WizDef.h"
#include <QLocale>
#include <algorithm>

WizSettings::WizSettings(const QString& strFileName)
    : QSettings(strFileName, QSettings::IniFormat)
{
    setIniCodec("utf-8");
}

QString WizSettings::getString(const QString& strSection, const QString& strKey, const QString& strDef /*= ""*/)
{
    QVariant v = value(strSection + "/" + strKey, strDef);
    return v.toString();
}

BOOL WizSettings::setString(const QString& strSection, const QString& strKey, const QString& str)
{
    setValue(strSection + "/" + strKey, str);
    return TRUE;
}

int WizSettings::getInt(const QString& strSection, const QString& strKey, int nDef /*= 0*/)
{
    QString str = getString(strSection, strKey, WizIntToStr(nDef));
    int n = wiz_ttoi(str);
    if (WizIntToStr(n) != str)
        return nDef;

    return n;
}

BOOL WizSettings::setInt(const QString& strSection, const QString& strKey, int val)
{
    return setString(strSection, strKey, WizIntToStr(val));
}

BOOL WizSettings::getBool(const QString& strSection, const QString& strKey, bool def /* = false */)
{
    return getInt(strSection, strKey, def ? 1 : 0) != 0;
}

BOOL WizSettings::setBool(const QString& strSection, const QString& strKey, bool val)
{
    return setInt(strSection, strKey, val ? 1 : 0);
}

QColor WizSettings::getColor(const QString& strSection,
                              const QString& strKey,
                              QColor defColor /* = "FFFFFF" */)
{
    QString str = getString(strSection, strKey, "");
    if (str.isEmpty())
        return defColor;

    return WizStringToColor2(str);
}

QString WizSettings::getEncryptedString(const QString& strSection, const QString& strKey, const QString& strDef /*= ""*/)
{
    return WizStringFromBase64(getString(strSection, strKey, strDef));
}

BOOL WizSettings::setEncryptedString(const QString& strSection, const QString& strKey, const QString& str)
{
    return setString(strSection, strKey, WizStringToBase64(str));
}

void WizSettings::getSections(CWizStdStringArray& arrayAction)
{
    QStringList sl = childGroups();
    arrayAction.assign(sl.begin(), sl.end());
}

void WizSettings::getKeys(const QString& strSection, CWizStdStringArray& arrayAction)
{
    beginGroup(strSection);
    QStringList sl = childKeys();
    endGroup();
    arrayAction.assign(sl.begin(), sl.end());
}


QString WizSettings::getProxyHost()
{
    return getString("Sync", "ProxyHost");
}

void WizSettings::setProxyHost(const QString& val)
{
    setString("Sync", "ProxyHost", val);
}

WizProxyType WizSettings::getProxyType()
{
    int port = getInt("Sync", "ProxyType", -1);
    if (port < 0)
        port = WizProxy_HttpProxy;

    return (WizProxyType)port;
}

void WizSettings::setProxyType(WizProxyType type)
{
    setInt("Sync", "ProxyType", type);
}

int WizSettings::getProxyPort()
{
    int port = getInt("Sync", "ProxyPort", 0);
    if (port <= 0)
        port = 80;

    return port;
}

void WizSettings::setProxyPort(int val)
{
    setInt("Sync", "ProxyPort", val);
}

QString WizSettings::getProxyUserName()
{
    return getString("Sync", "ProxyUserName");
}

void WizSettings::setProxyUserName(const QString& val)
{
    setString("Sync", "ProxyUserName", val);
}

QString WizSettings::getProxyPassword()
{
    return getString("Sync", "ProxyPassword");
}

void WizSettings::setProxyPassword(const QString& val)
{
    setString("Sync", "ProxyPassword", val);
}

bool WizSettings::getProxyStatus()
{
    return getBool("Sync", "ProxyStatus", false);
}

void WizSettings::setProxyStatus(bool val)
{
    setBool("Sync", "ProxyStatus", val);
}


#ifndef Q_OS_MAC

bool WizSettings::isDarkMode()
{
    return getBool("Common", "DarkMode", false);
}

void WizSettings::setDarkMode(bool b)
{
    setBool("Common", "DarkMode", b);
}

#endif


CString WizGetShortcut(const CString& strName, const CString& strDef /*= ""*/)
{
    WizSettings settings(Utils::WizPathResolve::globalSettingsFile());
    return settings.getString("Shortcut", strName, strDef);
}

WizUserSettings* WizUserSettings::s_currentSettings = nullptr;

WizUserSettings::WizUserSettings(const QString& strAccountFolderName)
    : m_strAccountFolderName(strAccountFolderName)
    , m_db(NULL)
{
    s_currentSettings = this;
}

WizUserSettings::WizUserSettings(WizDatabase& db)
    : m_db(&db)
{
    s_currentSettings = this;
}

void WizUserSettings::setAccountFolderName(const QString& strAccountFolderName)
{
    if (!m_db) {
        m_strAccountFolderName = strAccountFolderName;
        m_strSkinName.clear();
        m_strLocale.clear();
    }
}

QString WizUserSettings::myWizMail() const
{
    return get("ACCOUNT", "MYWIZMAIL");
}

QString WizUserSettings::get(const QString& section, const QString& strKey) const
{
    if (!m_strAccountFolderName.isEmpty()) {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            return db.getMetaDef(section, strKey);
        }
    }

    if (m_db) {
        return m_db->getMetaDef(section, strKey);
    }

    return QString();
}

void WizUserSettings::set(const QString& section, const QString& strKey, const QString& strValue)
{
    if (!m_strAccountFolderName.isEmpty()) {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            db.setMeta(section, strKey, strValue);
            return;
        }
    }

    if (m_db) {
        m_db->setMeta(section, strKey, strValue);
        return;
    }
}

QString WizUserSettings::userId() const
{
    if (m_db)
        return m_db->getUserId();

    if (!m_strAccountFolderName.isEmpty()) {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            return db.getUserId();
        }
    }

    return QString();
}

void WizUserSettings::setUserId(const QString& strUserId)
{
    if (m_db)
    {
        m_db->setMeta("Account", "USERID", strUserId);
    }
    else if (!m_strAccountFolderName.isEmpty())
    {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            db.setMeta("Account", "USERID", strUserId);
        }
    }
}

QString WizUserSettings::get(const QString& strKey) const
{
    if (!m_strAccountFolderName.isEmpty()) {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            return db.getMetaDef(USER_SETTINGS_SECTION, strKey);
        }
    }

    if (m_db) {
        return m_db->getMetaDef(USER_SETTINGS_SECTION, strKey);
    }

    return QString();
}

void WizUserSettings::set(const QString& strKey, const QString& strValue)
{
    if (!m_strAccountFolderName.isEmpty()) {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            db.setMeta(USER_SETTINGS_SECTION, strKey, strValue);
            return;
        }
    }

    if (m_db) {
        m_db->setMeta(USER_SETTINGS_SECTION, strKey, strValue);
        return;
    }
}

QString WizUserSettings::password() const
{
    QString strPassword;
    if (!m_strAccountFolderName.isEmpty()) {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            strPassword = db.getMetaDef("Account", "Password");
        }
    }

    if (m_db) {
        strPassword = m_db->getMetaDef("Account", "Password");
    }

    return ::WizDecryptPassword(strPassword);
}

void WizUserSettings::setPassword(const QString& strPassword /* = NULL */)
{
    if (!m_strAccountFolderName.isEmpty()) {
        WizDatabase db;
        if (db.open(m_strAccountFolderName)) {
            db.setMeta("Account", "Password", strPassword);
            return;
        }
    }

    if (m_db) {
        m_db->setMeta("Account", "Password", strPassword);
        return;
    }
}

WizServerType WizUserSettings::serverType() const
{
    QString strServerType = get("ServerType");
    if (strServerType.isEmpty()) {
        return NoServer;
    }

    return WizServerType(strServerType.toInt());
}

void WizUserSettings::setServerType(WizServerType server)
{
    QString strServerType = QString::number(server);
    set("ServerType", strServerType);
}

QString WizUserSettings::enterpriseServerIP()
{
    QString strServerType = get("EnterpriseServerIP");
    return strServerType;
}

void WizUserSettings::setEnterpriseServerIP(const QString& strEnterpriseServerd)
{
    set("EnterpriseServerIP", strEnterpriseServerd);
}

QString WizUserSettings::serverLicence()
{
    QString strServerType = get("ServerLicence");
    return strServerType;
}

void WizUserSettings::setServerLicence(const QString& strLicence)
{
    set("ServerLicence", strLicence);
}

bool WizUserSettings::autoLogin() const
{
    QString strAutoLogin = get("AutoLogin");
    if (!strAutoLogin.isEmpty()) {
        return strAutoLogin.toInt() ? true : false;
    }

    return false;
}

void WizUserSettings::setAutoLogin(bool bAutoLogin)
{
    set("AutoLogin", bAutoLogin ? "1" : "0");
}

bool WizUserSettings::autoCheckUpdate() const
{
    QString strAutoCheckUpdate = get("AutoCheckUpdate");
    if (!strAutoCheckUpdate.isEmpty()) {
        return strAutoCheckUpdate.toInt() ? true : false;
    }

    return true;
}

void WizUserSettings::setAutoCheckUpdate(bool bAutoCheckUpdate)
{
    set("AutoCheckUpdate", bAutoCheckUpdate ? "1" : "0");
}

bool WizUserSettings::showSystemTrayIcon() const
{
    QString strShowTrayIcon = get("ShowSystemTrayIcon");
    if (!strShowTrayIcon.isEmpty()) {
        return strShowTrayIcon.toInt() ? true : false;
    }

    return true;
}


void WizUserSettings::setShowSystemTrayIcon(bool bShowTrayIcon)
{
    set("ShowSystemTrayIcon", bShowTrayIcon ? "1" : "0");
}

bool WizUserSettings::useSystemBasedStyle() const
{
    QString strUseSystemStyle = get("UseSystemBasedStyle");
    if (!strUseSystemStyle.isEmpty()) {
        return strUseSystemStyle.toInt() ? true : false;
    }

    return true;
}

void WizUserSettings::setUseSystemBasedStyle(bool bSystemStyle)
{
    set("UseSystemBasedStyle", bSystemStyle ? "1" : "0");
}

bool WizUserSettings::isEnableSpellCheck() const
{
    QString strSpellCheck = get("SpellCheck");
    if (!strSpellCheck.isEmpty()) {
        return strSpellCheck.toInt() ? true : false;
    }

    return false;
}
void WizUserSettings::setEnableSpellCheck(bool b)
{
    set("SpellCheck", b ? "1" : "0");
}


bool WizUserSettings::receiveMobileFile() const
{
    QString strReceiveMobileFile = get("RecevieMobileFile");
    if (!strReceiveMobileFile.isEmpty()) {
        return strReceiveMobileFile.toInt() ? true : false;
    }

    return false;
}

void WizUserSettings::setReceiveMobileFile(bool bReceiveFile)
{
    set("RecevieMobileFile", bReceiveFile ? "1" : "0");
}

double WizUserSettings::printMarginValue(WizPositionType posType)
{
    QString strMarginValue = get("PrintMarginValue_" + QString::number(posType));
    if (strMarginValue.isEmpty()) {
        return 5.0;
    }

    return strMarginValue.toDouble();
}

void WizUserSettings::setPrintMarginValue(WizPositionType posType, double dValue)
{
    set("PrintMarginValue_" + QString::number(posType), QString::number(dValue));
}

int WizUserSettings::printMarginUnit()
{
    QString strMarginType = get("PrintMarginType");
    if (strMarginType.isEmpty()) {
        return 0;
    }

    return strMarginType.toInt();
}

void WizUserSettings::setPrintMarginUnit(int nUnit)
{
    set("PrintMarginType", QString::number(nUnit));
}

QString WizUserSettings::newFeatureGuideVersion()
{
    QString strGuideVersion = get("NewFeatureGuideVersion");
    return strGuideVersion;
}

void WizUserSettings::setNewFeatureGuideVersion(const QString& strGuideVersion)
{
    set("NewFeatureGuideVersion", strGuideVersion);
}

bool WizUserSettings::needShowMobileFileReceiverUserGuide()
{
    QString strShowGuide = get("ShowMobileFileReceiverUserGuide");
    if (!strShowGuide.isEmpty()) {
        return strShowGuide.toInt() ? true : false;
    }

    return true;
}

void WizUserSettings::setNeedShowMobileFileReceiverUserGuide(bool bNeedShow)
{
    set("ShowMobileFileReceiverUserGuide", bNeedShow ? "1" : "0");
}

bool WizUserSettings::searchEncryptedNote()
{
    QString strShowGuide = get("SearchEncryptedNote");
    if (!strShowGuide.isEmpty()) {
        return strShowGuide.toInt() ? true : false;
    }

    return false;
}

void WizUserSettings::setSearchEncryptedNote(bool bSearchEncryNote)
{
    set("SearchEncryptedNote", bSearchEncryNote ? "1" : "0");
}

QString WizUserSettings::encryptedNotePassword()
{
    QString strPassword = get("EncryptedNotePassword");
    return ::WizDecryptPassword(strPassword);
}

void WizUserSettings::setEncryptedNotePassword(const QString& strPassword)
{
    QString strEncryptPass = ::WizEncryptPassword(strPassword);
    set("EncryptedNotePassword", strEncryptPass);
}

static bool rememberPasswordForSession = false;
bool WizUserSettings::isRememberNotePasswordForSession()
{
    return rememberPasswordForSession;
}

void WizUserSettings::setRememberNotePasswordForSession(bool remember)
{
    rememberPasswordForSession = remember;
}

QString WizUserSettings::editorBackgroundColor()
{
    QString strColor = get("EditorBackgroundColor");    
    //
    if (strColor == WizColorLineEditorBackground.name()) {
        return "";
    }
    //
    return strColor;
}

void WizUserSettings::setEditorBackgroundColor(const QString& strColor)
{
    set("EditorBackgroundColor", strColor);
}

QString WizUserSettings::editorLineHeight()
{
    QString strLineHeight = get("EditorLineHeight");
    //
    if (strLineHeight.isEmpty()) {
        return "1.7";
    }
    //
    return strLineHeight;
}

void WizUserSettings::setEditorLineHeight(const QString& strLineHeight)
{
    set("EditorLineHeight", strLineHeight);
}


QString WizUserSettings::editorParaSpacing()
{
    QString strLineHeight = get("EditorParaSpacing");
    //
    if (strLineHeight.isEmpty()) {
        return "8";
    }
    //
    return strLineHeight;
}

void WizUserSettings::setEditorParaSpacing(const QString& spacing)
{
    set("EditorParaSpacing", spacing);
}


bool WizUserSettings::isManualSortingEnabled()
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

void WizUserSettings::setManualSortingEnable(bool bEnable)
{
    set("ManualSortingEnable", bEnable ? "1" : "0");
}

QString WizUserSettings::skin()
{
    // just return because no skin selection from v1.4
    return "default";

    if (!m_strSkinName.isEmpty())
        return m_strSkinName;

    QString strSkinName = get("Skin");

    if (!strSkinName.isEmpty()) {
        if (WizPathFileExists(::WizGetSkinResourcePath(strSkinName))) {
            m_strSkinName = strSkinName;
            return strSkinName;
        }
    }

    //CWizSettings settings(::WizGetDataStorePath() + "wiznote.ini");
    //strSkinName = settings.GetString("Common", "Skin");

    //if (!strSkinName.isEmpty()) {
    //    if (WizPathFileExists(::WizGetSkinResourcePath(strSkinName))) {
    //        m_strSkinName = strSkinName;
    //        return strSkinName;
    //    }
    //}

    // default skin depends on user's OS
    return ::WizGetDefaultSkinName();
}

void WizUserSettings::setSkin(const QString& strSkinName)
{
    Q_ASSERT(!strSkinName.isEmpty());

    m_strSkinName = strSkinName;
    set("Skin", strSkinName);
}

QString WizUserSettings::locale()
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
    if (::WizPathFileExists(Utils::WizPathResolve::localeFileName(strLocale))) {
        m_strLocale = strLocale;
        return strLocale;
    }

    return WizGetDefaultTranslatedLocal();
}

void WizUserSettings::setLocale(const QString& strLocale)
{
    Q_ASSERT(!strLocale.isEmpty());

    m_strLocale = strLocale;
    set("Locale", strLocale);
}

QString WizUserSettings::defaultFontFamily()
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

void WizUserSettings::setDefaultFontFamily(const QString& strFont)
{
    set("DefaultFontFamily", strFont);
}

void WizUserSettings::setLastAttachmentPath(const QString& path)
{
    set("LastAttachmentPath", path);
}

QString WizUserSettings::lastAttachmentPath() const
{
    QString path = get("LastAttachmentPath");
    if (path.isEmpty())
        return path;
    //
    ::WizPathAddBackslash(path);
    return path;
}


int WizUserSettings::defaultFontSize()
{
    int nSize = get("DefaultFontSize").toInt();
    if (nSize)
        return nSize;

    return 15; // default 15px
}

void WizUserSettings::setDefaultFontSize(int nSize)
{
    set("DefaultFontSize", QString::number(nSize));
}

WizDocumentViewMode WizUserSettings::noteViewMode() const
{
    QString mode = get("NoteViewMode");

    if (!mode.isEmpty()) {
        return WizDocumentViewMode(mode.toInt());
    }

    return viewmodeAlwaysReading;
}

int WizUserSettings::syncInterval() const
{
    int nInterval = get("SyncInterval").toInt();
    if (!nInterval)
        return 15; // default 15 minutes

    return nInterval;
}

void WizUserSettings::setSyncInterval(int minutes)
{
    set("SyncInterval", QString::number(minutes));
}

int WizUserSettings::syncMethod() const
{
    int nDays = get("SyncMethod").toInt();
    if (!nDays)
        return 99999; // default all

    return nDays;
}

void WizUserSettings::setSyncMethod(int days)
{
    set("SyncMethod", QString::number(days));
}

int WizUserSettings::syncGroupMethod() const
{
    int nDays = get("SyncGroupMethod").toInt();
    if (!nDays)
        return 1;   // default 1 day

    return nDays;
}

void WizUserSettings::setSyncGroupMethod(int days)
{
    set("SyncGroupMethod", QString::number(days));
}

bool WizUserSettings::showSubFolderDocuments()
{
    bool b = get("CategoryShowSubFolderDocuments").toInt() ? true : false;
    return b;
}
void WizUserSettings::setShowSubFolderDocuments(bool b)
{
    set("CategoryShowSubFolderDocuments", QString::number(b ? 1 : 0));
}

void WizUserSettings::appendRecentSearch(const QString& search)
{
    if (search.isEmpty())
        return;

    QStringList recentSearches = getRecentSearches();
    QStringList::const_iterator pos = std::find_if(recentSearches.begin(), recentSearches.end(), [search](QString it){
        return it == search;
    });

    if (pos != recentSearches.end())
        return;

    while (recentSearches.count() >= 5)
        recentSearches.pop_front();

    recentSearches.append(search);
    QString searches = recentSearches.join("/");
    set("RecentSearches", searches);
}

QStringList WizUserSettings::getRecentSearches(bool reverseOrder)
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
