#include "wizsettings.h"


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

BOOL CWizSettings::GetBool(const QString& strSection, const QString& strKey, bool def)
{
    return GetInt(strSection, strKey, def ? 1 : 0) != 0;
}

BOOL CWizSettings::SetBool(const QString& strSection, const QString& strKey, bool val)
{
    return SetInt(strSection, strKey, val ? 1 : 0);
}

QColor CWizSettings::GetColor(const QString& strSection, const QString& strKey, QColor defColor)
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



CString WizGetString(const CString& strSection, const CString& strKey, const CString& strDef /*= ""*/)
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.GetString(strSection, strKey, strDef);
}

BOOL WizSetString(const CString& strSection, const CString& strKey, const CString& str)
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.SetString(strSection, strKey, str);
}

CString WizGetShortcut(const CString& strName, const CString& strDef /*= ""*/)
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.GetString("Shortcut", strName, strDef);
}

QColor WizGetSkinColor(const CString& strSection, const CString& strName, const QColor& colorDef)
{
    CWizSettings settings(WizGetSkinPath() + "skin.ini");
    return settings.GetColor(strSection, strName, colorDef);
}

int WizGetSkinInt(const CString& strSection, const CString& strName, int def)
{
    CWizSettings settings(WizGetSkinPath() + "skin.ini");
    return settings.GetInt(strSection, strName, def);
}
