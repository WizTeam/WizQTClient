#ifndef WIZSETTINGS_H
#define WIZSETTINGS_H

#include <QSettings>

#include "wizqthelper.h"
#include "wizmisc.h"


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


CString WizGetString(const CString& strSection, const CString& strKey, const CString& strDef = "");
BOOL WizSetString(const CString& strSection, const CString& strKey, const CString& str);

CString WizGetShortcut(const CString& strName, const CString& strDef = "");

QColor WizGetSkinColor(const CString& strSection, const CString& strName, const QColor& colorDef);
int WizGetSkinInt(const CString& strSection, const CString& strName, int def);


#endif // WIZSETTINGS_H
