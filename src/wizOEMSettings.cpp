#include "wizOEMSettings.h"
#include <QVariant>
#include <QFile>
#include <QDebug>
#include "utils/pathresolve.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"
#include "rapidjson/document.h"


#define HideShareByEmail        "HideShareByEmail"
#define HidePersonalGroup       "HidePersonalGroup"
#define HideFeedback              "HideFeedback"
#define HideRegister                   "HideRegister"
#define EncryptPassword        "EncryptPassword"
#define HideSocialLogin            "HideSocialLogin"
#define HideForgotPassword            "HideForgotPassword"
#define HideShare                     "HideShare"
#define AccountPlaceholder            "AccountPlaceholder"
#define HideMyShare                "HideMyShare"
#define HideBuyVip                 "HideBuyVip"
#define ForbidCreateBiz              "ForbidCreateBiz"
#define LoginLogoPath              "LoginLogoPath"


void getBoolValueFromJSON(const rapidjson::Document& d, const char* strMember, QSettings* settings)
{
    if (d.FindMember(strMember)) {
        bool b = d.FindMember(strMember)->value.GetBool();
        qDebug() << strMember << " : " << b;
        settings->setValue(strMember, b);
    }
}

CWizOEMSettings::CWizOEMSettings(const QString& strUserAccountPath)
    : QSettings(strUserAccountPath + "oem.ini", QSettings::IniFormat)
{
}


bool CWizOEMSettings::settingFileExists(const QString& strUserAccountPath)
{
    return QFile::exists(strUserAccountPath + "oem.ini");
}

void CWizOEMSettings::updateOEMSettings(const QString& strUserAccountPath, const QString& strOEMJSONData)
{
    if (strUserAccountPath.isEmpty() || strOEMJSONData.isEmpty())
        return;

    QString strFile = strUserAccountPath + "oem.ini";
    QSettings settings(strFile, QSettings::IniFormat);

    rapidjson::Document d;
    d.Parse<0>(strOEMJSONData.toUtf8().constData());

    getBoolValueFromJSON(d, HidePersonalGroup, &settings);
    getBoolValueFromJSON(d, HideShareByEmail, &settings);
    getBoolValueFromJSON(d, HideFeedback, &settings);
    getBoolValueFromJSON(d, HideRegister, &settings);
    getBoolValueFromJSON(d, EncryptPassword, &settings);
    getBoolValueFromJSON(d, HideSocialLogin, &settings);
    getBoolValueFromJSON(d, HideForgotPassword, &settings);
    getBoolValueFromJSON(d, HideShare, &settings);
    getBoolValueFromJSON(d, AccountPlaceholder, &settings);
    getBoolValueFromJSON(d, HideMyShare, &settings);
    getBoolValueFromJSON(d, HideBuyVip, &settings);
    getBoolValueFromJSON(d, ForbidCreateBiz, &settings);
}

bool CWizOEMSettings::isHideShareByEmail()
{
    return value(HideShareByEmail, false).toBool();
}

bool CWizOEMSettings::isHidePersonalGroup()
{
    return value(HidePersonalGroup, false).toBool();
}

bool CWizOEMSettings::isHideFeedback()
{
    return value(HideFeedback, false).toBool();
}

bool CWizOEMSettings::isHideRegister()
{
    return value(HideRegister, false).toBool();
}

bool CWizOEMSettings::isEncryptPassword()
{
    return value(EncryptPassword, false).toBool();
}

bool CWizOEMSettings::isHideSocialLogin()
{
    return value(HideSocialLogin, false).toBool();
}

bool CWizOEMSettings::isHideForgotPassword()
{
    return value(HideForgotPassword, false).toBool();
}

bool CWizOEMSettings::isHideShare()
{
    return value(HideShare, false).toBool();
}

bool CWizOEMSettings::isAccountPlaceholder()
{
    return value(AccountPlaceholder, false).toBool();
}

bool CWizOEMSettings::isHideMyShare()
{
    return value(HideMyShare, false).toBool();
}

bool CWizOEMSettings::isHideBuyVip()
{
    return value(HideBuyVip, false).toBool();
}

bool CWizOEMSettings::isForbidCreateBiz()
{
    return value(ForbidCreateBiz, false).toBool();
}

void CWizOEMSettings::setLogoPath(const QString& path)
{
    setValue(LoginLogoPath, path);
}

QString CWizOEMSettings::logoPath()
{
    return value(LoginLogoPath, "").toString();
}
