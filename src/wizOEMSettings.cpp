#include "wizOEMSettings.h"
#include <QVariant>
#include "utils/pathresolve.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"

CWizOEMSettingsPrivate* CWizOEMSettings::m_settings = 0;

CWizOEMSettings::CWizOEMSettings()
{
}

CWizOEMSettings::~CWizOEMSettings()
{
}

void CWizOEMSettings::updateOEMSettings()
{
    helper()->updateOEMSettings();
}

bool CWizOEMSettings::isHideShareByEmail()
{
    return helper()->isHideShareByEmail();
}

bool CWizOEMSettings::isHidePersonalGroup()
{
    return helper()->isHidePersonalGroup();
}

bool CWizOEMSettings::isHideFeedback()
{
    return helper()->isHideFeedback();
}

bool CWizOEMSettings::isHideRegister()
{
    return helper()->isHideRegister();
}

bool CWizOEMSettings::isEncryptPassword()
{
    return helper()->isEncryptPassword();
}

bool CWizOEMSettings::isHideSocialLogin()
{
    return helper()->isHideSocialLogin();
}

bool CWizOEMSettings::isHideForgotPassword()
{
    return helper()->isHideForgotPassword();
}

bool CWizOEMSettings::isHideShare()
{
    return helper()->isHideShare();
}

bool CWizOEMSettings::isAccountPlaceholder()
{
    return helper()->isAccountPlaceholder();
}

bool CWizOEMSettings::isHideMyShare()
{
    return helper()->isHideMyShare();
}

bool CWizOEMSettings::isHideBuyVip()
{
    return helper()->isHideBuyVip();
}

bool CWizOEMSettings::isForbidCreateBiz()
{
    return helper()->isForbidCreateBiz();
}

CWizOEMSettingsPrivate*CWizOEMSettings::helper()
{
    if (!m_settings)
    {
        QString strUserId = CWizDatabaseManager::instance()->db().GetUserId();
        QString strFile = Utils::PathResolve::dataStorePath() + strUserId + "/oem.ini";
        m_settings = new CWizOEMSettingsPrivate(strFile);
    }
    return m_settings;
}


CWizOEMSettingsPrivate::CWizOEMSettingsPrivate(const QString& fileName)
    : QSettings(fileName, QSettings::IniFormat)
{

}

bool CWizOEMSettingsPrivate::isHideShareByEmail()
{
    return value("HideShareByEmail", false).toBool();
}

bool CWizOEMSettingsPrivate::isHidePersonalGroup()
{
    return value("HidePersonalGroup", false).toBool();
}

bool CWizOEMSettingsPrivate::isHideFeedback()
{
    return value("HideFeedback", false).toBool();
}

bool CWizOEMSettingsPrivate::isHideRegister()
{
    return value("HideRegister", false).toBool();
}

bool CWizOEMSettingsPrivate::isEncryptPassword()
{
    return value("EncryptPassword", false).toBool();
}

bool CWizOEMSettingsPrivate::isHideSocialLogin()
{
    return value("HideSocialLogin", false).toBool();
}

bool CWizOEMSettingsPrivate::isHideForgotPassword()
{
    return value("HideForgotPassword", false).toBool();
}

bool CWizOEMSettingsPrivate::isHideShare()
{
    return value("HideShare", false).toBool();
}

bool CWizOEMSettingsPrivate::isAccountPlaceholder()
{
    return value("AccountPlaceholder", false).toBool();
}

bool CWizOEMSettingsPrivate::isHideMyShare()
{
    return value("HideMyShare", false).toBool();
}

bool CWizOEMSettingsPrivate::isHideBuyVip()
{
    return value("HideBuyVip", false).toBool();
}

bool CWizOEMSettingsPrivate::isForbidCreateBiz()
{
    return value("ForbidCreateBiz", false).toBool();
}
