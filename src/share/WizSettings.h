#ifndef WIZSETTINGS_H
#define WIZSETTINGS_H

#include <QSettings>

#include "WizQtHelper.h"
#include "WizMisc.h"

#include "WizDatabase.h"

enum WizProxyType
{
    WizProxy_NoProxy,
    WizProxy_HttpProxy,
    WizProxy_Socks5Proxy,
};


enum WizServerType
{
    NoServer = 0,
    WizServer = 99,
    EnterpriseServer = 199
};

class WizSettings : public QSettings
{
public:
    WizSettings(const QString& strFileName);

    void getSections(CWizStdStringArray& arrayAction);
    void getKeys(const QString& strSection, CWizStdStringArray& arrayAction);

    QString getString(const QString& strSection, const QString& strKey, const QString& strDef = "");
    BOOL setString(const QString& strSection, const QString& strKey, const QString& str);

    int getInt(const QString& strSection, const QString& strKey, int nDef = 0);
    BOOL setInt(const QString& strSection, const QString& strKey, int val);

    BOOL getBool(const QString& strSection, const QString& strKey, bool def = false);
    BOOL setBool(const QString& strSection, const QString& strKey, bool def);

    QColor getColor(const QString& strSection, const QString& strKey, QColor defColor = "#FFFFFF");

    QString getEncryptedString(const QString& strSection, const QString& strKey, const QString& strDef = "");
    BOOL setEncryptedString(const QString& strSection, const QString& strKey, const QString& str);

    // proxy settings
    QString getProxyHost();
    void setProxyHost(const QString& val);
    WizProxyType getProxyType();
    void setProxyType(WizProxyType type);
    int getProxyPort();
    void setProxyPort(int val);
    QString getProxyUserName();
    void setProxyUserName(const QString& val);
    QString getProxyPassword();
    void setProxyPassword(const QString& val);
    bool getProxyStatus();
    void setProxyStatus(bool val);
    //
#ifndef Q_OS_MAC
    bool isDarkMode();
    void setDarkMode(bool b);
#endif
};


CString WizGetShortcut(const CString& strName, const CString& strDef = "");


enum WizOptionsType
{
    wizoptionsNoteView,
    wizoptionsSync,
    wizoptionsSkin,
    wizoptionsFont,
    wizoptionsFolders,
    wizoptionsSpellCheck,
};

enum WizPositionType
{
    wizPositionTop,
    wizPositionBottom,
    wizPositionLeft,
    wizPositionRight
};

class WizUserSettings
{
public:

    // m_db should always 0 if init as this way.
    WizUserSettings(const QString& strAccountFolderName);

    // m_strUserId should always 0 if init as this way.
    WizUserSettings(WizDatabase& db);

    static WizUserSettings* currentSettings() { return s_currentSettings; }
private:
    QString m_strAccountFolderName;
    QString m_strSkinName;
    QString m_strLocale;
    WizDatabase* m_db;
    static WizUserSettings* s_currentSettings;

public:
    QString get(const QString& key) const;
    void set(const QString& key, const QString& value);

    QString get(const QString& section, const QString& strKey) const;
    void set(const QString& section, const QString& strKey, const QString& strValue);

    QString userId() const;
    void setUserId(const QString& strUserId);

    void setAccountFolderName(const QString& strAccountFolderName);

    QString myWizMail() const;

    QString password() const;
    void setPassword(const QString& strPassword = "");

    WizServerType serverType() const;
    void setServerType(WizServerType server = NoServer);

    QString enterpriseServerIP ();
    void setEnterpriseServerIP(const QString& strEnterpriseServerd = "");

    QString serverLicence();
    void setServerLicence(const QString& strLicence = "");

    bool autoLogin() const;
    void setAutoLogin(bool bAutoLogin);

    bool autoCheckUpdate() const;
    void setAutoCheckUpdate(bool bAutoCheckUpdate);

    bool showSystemTrayIcon() const;
    void setShowSystemTrayIcon(bool bShowTrayIcon);

    bool useSystemBasedStyle() const;
    void setUseSystemBasedStyle(bool bSystemStyle);

    bool isEnableSpellCheck() const;
    void setEnableSpellCheck(bool b);

    bool receiveMobileFile() const;
    void setReceiveMobileFile(bool bReceiveFile);

    double printMarginValue(WizPositionType posType);
    void setPrintMarginValue(WizPositionType posType, double dValue);

    int printMarginUnit();
    void setPrintMarginUnit(int nUnit);

    QString newFeatureGuideVersion();
    void setNewFeatureGuideVersion(const QString& strGuideVersion);

    bool needShowMobileFileReceiverUserGuide();
    void setNeedShowMobileFileReceiverUserGuide(bool bNeedShow);

    bool searchEncryptedNote();
    void setSearchEncryptedNote(bool bSearchEncryNote);

    QString encryptedNotePassword();
    void setEncryptedNotePassword(const QString& strPassword);

    bool isRememberNotePasswordForSession();
    void setRememberNotePasswordForSession(bool remember);

    //NOTE:  editor background color string could be empty!!!   if it's empty, editor in seperate window could be grey
    QString editorBackgroundColor();
    void setEditorBackgroundColor(const QString& strColor);
    //

    QString editorLineHeight();
    void setEditorLineHeight(const QString& strLineHeight);
    //
    QString editorParaSpacing();
    void setEditorParaSpacing(const QString& strSpacing);

    bool isManualSortingEnabled();
    void setManualSortingEnable(bool bEnable);

    QString locale();
    void setLocale(const QString& strLocale);

    QString skin();
    void setSkin(const QString& strSkinName);

    WizDocumentViewMode noteViewMode() const;
    void setNoteViewMode(WizDocumentViewMode strMode) { set("NoteViewMode", QString::number(strMode)); }

    QString defaultFontFamily();
    void setDefaultFontFamily(const QString& strFont);

    int defaultFontSize();
    void setDefaultFontSize(int nSize);

    // default: 5, 15, 30, 60, -1(manual)
    int syncInterval() const;
    void setSyncInterval(int minutes);

    // time line of personal data be downloaded
    // set: 1, 7, 30, 99999(all), -1(no), default: 99999
    int syncMethod() const;
    void setSyncMethod(int days);

    // time line of group data be downloaded
    // set: 1, 7, 30, 99999(all), -1(no), default: 1
    int syncGroupMethod() const;
    void setSyncGroupMethod(int days);

    void appendRecentSearch(const QString& search);
    QStringList getRecentSearches(bool reverseOrder = false);
    //
    void setLastAttachmentPath(const QString& path);
    QString lastAttachmentPath() const;
    //
    bool showSubFolderDocuments();
    void setShowSubFolderDocuments(bool b);
};

#endif // WIZSETTINGS_H
