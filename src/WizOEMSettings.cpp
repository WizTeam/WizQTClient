#include "WizOEMSettings.h"
#include <QVariant>
#include <QFile>
#include <QDebug>
#include "utils/WizPathResolve.h"
#include "share/WizDatabase.h"
#include "share/WizDatabaseManager.h"
#include "share/jsoncpp/json/json.h"


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


void getBoolValueFromJSON(const Json::Value& d, const char* strMember, QSettings* settings)
{
    if (d.isMember(strMember)) {
        bool b = d[strMember].asBool();
        qDebug() << strMember << " : " << b;
        settings->setValue(strMember, b);
    }
}

WizOEMSettings::WizOEMSettings(const QString& strUserAccountPath)
    : QSettings(strUserAccountPath + "oem.ini", QSettings::IniFormat)
{
}


bool WizOEMSettings::settingFileExists(const QString& strUserAccountPath)
{
    return QFile::exists(strUserAccountPath + "oem.ini");
}

void WizOEMSettings::updateOEMSettings(const QString& strUserAccountPath, const QString& strOEMJSONData)
{
    if (strUserAccountPath.isEmpty() || strOEMJSONData.isEmpty())
        return;

    QString strFile = strUserAccountPath + "oem.ini";
    QSettings settings(strFile, QSettings::IniFormat);

    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(strOEMJSONData.toUtf8().constData(), d))
        return;

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

bool WizOEMSettings::isHideShareByEmail()
{
    return value(HideShareByEmail, false).toBool();
}

bool WizOEMSettings::isHidePersonalGroup()
{
    return value(HidePersonalGroup, false).toBool();
}

bool WizOEMSettings::isHideFeedback()
{
    return value(HideFeedback, false).toBool();
}

bool WizOEMSettings::isHideRegister()
{
    return value(HideRegister, false).toBool();
}

bool WizOEMSettings::isEncryptPassword()
{
    return value(EncryptPassword, false).toBool();
}

bool WizOEMSettings::isHideSocialLogin()
{
    return value(HideSocialLogin, false).toBool();
}

bool WizOEMSettings::isHideForgotPassword()
{
    return value(HideForgotPassword, false).toBool();
}

bool WizOEMSettings::isHideShare()
{
    return value(HideShare, false).toBool();
}

bool WizOEMSettings::isAccountPlaceholder()
{
    return value(AccountPlaceholder, false).toBool();
}

bool WizOEMSettings::isHideMyShare()
{
    return value(HideMyShare, false).toBool();
}

bool WizOEMSettings::isHideBuyVip()
{
    return value(HideBuyVip, false).toBool();
}

bool WizOEMSettings::isForbidCreateBiz()
{
    return value(ForbidCreateBiz, false).toBool();
}

void WizOEMSettings::setLogoPath(const QString& path)
{
    setValue(LoginLogoPath, path);
}

QString WizOEMSettings::logoPath()
{
    return value(LoginLogoPath, "").toString();
}
