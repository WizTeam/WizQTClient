#ifndef WIZSETTINGS_H
#define WIZSETTINGS_H

#include <QSettings>

#include "wizqthelper.h"
#include "wizmisc.h"

#include "wiznotesettings.h"
#include "wizdatabase.h"

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

    BOOL GetBool(const QString& strSection, const QString& strKey, bool def);
    BOOL SetBool(const QString& strSection, const QString& strKey, bool def);

    QColor GetColor(const QString& strSection, const QString& strKey, QColor defColor);

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

};


BOOL WizSetString(const CString& strSection, const CString& strKey, const CString& str);

CString WizGetShortcut(const CString& strName, const CString& strDef = "");

QColor WizGetSkinColor(const CString& strSection, const CString& strName, const QColor& colorDef);
int WizGetSkinInt(const CString& strSection, const CString& strName, int def);




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
    CWizDatabase* m_db;

    QString get(const QString& key) const;
    void set(const QString& key, const QString& value);

public:
    QString user() const { return m_strUserId; }
    void setUser(const QString& strUser);

    QString password() const;
    void setPassword(const QString& strPassword = "");

    QString locale() const { return get("Locale"); }
    void setLocale(const QString& strLocale) { set("Locale", strLocale); }

    QString skin() const { return get("Skin"); }
    void setSkin(const QString& strSkin) { set("Skin", strSkin); }

    WizDocumentViewMode noteViewMode() const { return WizDocumentViewMode(get("NoteViewMode").toInt()); }
    void setNoteViewMode(WizDocumentViewMode strMode) { return set("NoteViewMode", QString(strMode)); }

    bool autoSync() const { return get("AutoSync").toInt() ? true : false; }
    void setAutoSync(bool b) { set("AutoSync", b ? "1" : "0"); }

    bool downloadAllNotesData() const { return get("DownloadAllNoteData").toInt() ? true : false; }
    void setDownloadAllNotesData(bool b) { set("DownloadAllNotesData", b ? "1" : "0"); }
};

#endif // WIZSETTINGS_H
