#ifndef WIZSETTINGS_H
#define WIZSETTINGS_H

#include <QSettings>

#include "wizqthelper.h"
#include "wizmisc.h"

#include "wizDatabase.h"

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

class CWizSettings : public QSettings
{
public:
    CWizSettings(const QString& strFileName);

    void GetSections(CWizStdStringArray& arrayAction);
    void GetKeys(const QString& strSection, CWizStdStringArray& arrayAction);

    QString GetString(const QString& strSection, const QString& strKey, const QString& strDef = "");
    BOOL SetString(const QString& strSection, const QString& strKey, const QString& str);

    int GetInt(const QString& strSection, const QString& strKey, int nDef = 0);
    BOOL SetInt(const QString& strSection, const QString& strKey, int val);

    BOOL GetBool(const QString& strSection, const QString& strKey, bool def = false);
    BOOL SetBool(const QString& strSection, const QString& strKey, bool def);

    QColor GetColor(const QString& strSection, const QString& strKey, QColor defColor = "#FFFFFF");

    QString GetEncryptedString(const QString& strSection, const QString& strKey, const QString& strDef = "");
    BOOL SetEncryptedString(const QString& strSection, const QString& strKey, const QString& str);

    // proxy settings
    QString GetProxyHost();
    void SetProxyHost(const QString& val);
    WizProxyType GetProxyType();
    void SetProxyType(WizProxyType type);
    int GetProxyPort();
    void SetProxyPort(int val);
    QString GetProxyUserName();
    void SetProxyUserName(const QString& val);
    QString GetProxyPassword();
    void SetProxyPassword(const QString& val);
    bool GetProxyStatus();
    void SetProxyStatus(bool val);

};


CString WizGetShortcut(const CString& strName, const CString& strDef = "");

enum WizDocumentViewMode
{
    viewmodeAlwaysEditing,
    viewmodeAlwaysReading,
    viewmodeKeep
};

enum WizOptionsType
{
    wizoptionsNoteView,
    wizoptionsSync,
    wizoptionsSkin,
    wizoptionsFont,
    wizoptionsFolders,
    wizoptionsMarkdown
};

enum WizPositionType
{
    wizPositionTop,
    wizPositionBottom,
    wizPositionLeft,
    wizPositionRight
};

class CWizUserSettings
{
public:

    // m_db should always 0 if init as this way.
    CWizUserSettings(const QString& strAccountFolderName);

    // m_strUserId should always 0 if init as this way.
    CWizUserSettings(CWizDatabase& db);

private:
    QString m_strAccountFolderName;
    QString m_strSkinName;
    QString m_strLocale;
    CWizDatabase* m_db;

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
};

#endif // WIZSETTINGS_H
