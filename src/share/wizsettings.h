#ifndef WIZSETTINGS_H
#define WIZSETTINGS_H

#include <QSettings>

#include "wizqthelper.h"
#include "wizmisc.h"

#include "wizDatabase.h"

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
    wizoptionsFont
};

const QString USER_SETTINGS_SECTION = "QT_WIZNOTE";

class CWizUserSettings
{
public:

    // m_db should always 0 if init as this way.
    CWizUserSettings(const QString& strUserId);

    // m_strUserId should always 0 if init as this way.
    CWizUserSettings(CWizDatabase& db);

private:
    QString m_strUserId;
    QString m_strSkinName;
    QString m_strLocale;
    CWizDatabase* m_db;

public:
    QString get(const QString& key) const;
    void set(const QString& key, const QString& value);

    QString get(const QString& section, const QString& strKey) const;
    void set(const QString& section, const QString& strKey, const QString& strValue);

    QString user() const { return m_strUserId; }
    void setUser(const QString& strUser);

    QString password() const;
    void setPassword(const QString& strPassword = "");

    bool autoLogin() const;
    void setAutoLogin(bool bAutoLogin);

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
};

#endif // WIZSETTINGS_H
