#include "wizsettings.h"
#include "wizmisc.h"


CWizSettings::CWizSettings(const CString& strFileName)
    : QSettings(strFileName, QSettings::IniFormat)
{
    setIniCodec("utf-8");
}


CString CWizSettings::GetString(const CString& strSection, const CString& strKey, const CString& strDef /*= ""*/)
{
    QVariant v = value(strSection + "/" + strKey, strDef);
    return v.toString();
}

BOOL CWizSettings::SetString(const CString& strSection, const CString& strKey, const CString& str)
{
    setValue(strSection + "/" + strKey, str);
    return TRUE;
}

int CWizSettings::GetInt(const CString& strSection, const CString& strKey, int nDef /*= 0*/)
{
    CString str = GetString(strSection, strKey, WizIntToStr(nDef));
    int n = _ttoi(str);
    if (WizIntToStr(n) != str)
        return nDef;
    //
    return n;
}
QColor CWizSettings::GetColor(const CString& strSection, const CString& strKey, QColor defColor)
{
    CString str = GetString(strSection, strKey, "");
    if (str.isEmpty())
        return defColor;
    //
    return WizStringToColor2(str);
}

BOOL CWizSettings::SetInt(const CString& strSection, const CString& strKey, int val)
{
    return SetString(strSection, strKey, WizIntToStr(val));
}


CString CWizSettings::GetEncryptedString(const CString& strSection, const CString& strKey, const CString& strDef /*= ""*/)
{
    return WizStringFromBase64(GetString(strSection, strKey, strDef));
}

BOOL CWizSettings::SetEncryptedString(const CString& strSection, const CString& strKey, const CString& str)
{
    return SetString(strSection, strKey, WizStringToBase64(str));
}
void CWizSettings::GetSections(CWizStdStringArray& arrayAction)
{
    QStringList sl = childGroups();
    //
    arrayAction.assign(sl.begin(), sl.end());
}
void CWizSettings::GetKeys(const CString& strSection, CWizStdStringArray& arrayAction)
{
    beginGroup(strSection);
    QStringList sl = childKeys();
    endGroup();
    //
    arrayAction.assign(sl.begin(), sl.end());
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

int WizGetInt(const CString& strSection, const CString& strKey, int nDef /*= 0*/)
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.GetInt(strSection, strKey, nDef);
}

BOOL WizSetInt(const CString& strSection, const CString& strKey, int val)
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.SetInt(strSection, strKey, val);
}

BOOL WizGetBool(const CString& strSection, const CString& strKey, bool def)
{
    return WizGetInt(strSection, strKey, def ? 1 : 0) != 0;
}

BOOL WizSetBool(const CString& strSection, const CString& strKey, bool val)
{
    return WizSetInt(strSection, strKey, val ? 1 : 0);
}

CString WizGetEncryptedString(const CString& strSection, const CString& strKey, const CString& strDef /*= ""*/)
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.GetEncryptedString(strSection, strKey, strDef);
}

BOOL WizSetEncryptedString(const CString& strSection, const CString& strKey, const CString& str)
{
    CWizSettings settings(WizGetSettingsFileName());
    return settings.SetEncryptedString(strSection, strKey, str);
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



QString WizGetProxyHost()
{
    return WizGetString("Sync", "ProxyHost");
}
int WizGetProxyPort()
{
    int port = WizGetInt("Sync", "ProxyPort", 0);
    if (port <= 0)
        port = 80;
    //
    return port;
}
QString WizGetProxyUserName()
{
    return WizGetString("Sync", "ProxyUserName");
}
QString WizGetProxyPassword()
{
    return WizGetString("Sync", "ProxyPassword");
}

void WizSetProxyHost(const QString& val)
{
    WizSetString("Sync", "ProxyHost", val);
}

void WizSetProxyPort(int val)
{
    WizSetInt("Sync", "ProxyPort", val);
}

void WizSetProxyUserName(const QString& val)
{
    WizSetString("Sync", "ProxyUserName", val);
}
void WizSetProxyPassword(const QString& val)
{
    WizSetString("Sync", "ProxyPassword", val);
}
