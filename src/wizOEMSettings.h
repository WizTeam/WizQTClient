#ifndef CWIZOEMSETTINGS_H
#define CWIZOEMSETTINGS_H

#include <QSettings>

class CWizOEMSettings
{
public:


private:
    CWizOEMSettings();
    ~CWizOEMSettings();

    static void updateOEMSettings();

    static bool isHideShareByEmail();
    static bool isHidePersonalGroup();
    static bool isHideFeedback();
    static bool isHideRegister();
    static bool isEncryptPassword();
    static bool isHideSocialLogin();
    static bool isHideForgotPassword();
    static bool isHideShare();                         //隐藏所有分享到外部方式
    static bool isAccountPlaceholder();            //登录框显示的域账号
    static bool isHideMyShare();
    static bool isHideBuyVip();
    static bool isForbidCreateBiz();

private:
    static CWizOEMSettingsPrivate* m_settings;

    class CGarbo
    {
    public:
        ~CGarbo()
        {
            if(CWizOEMSettings::m_settings)
                delete CWizOEMSettings::m_settings;
        }
    };
    static CGarbo Garbo;

    static CWizOEMSettingsPrivate* helper();
};


class CWizOEMSettingsPrivate : public QSettings
{
public:
    CWizOEMSettingsPrivate(const QString & fileName);
    ~CWizOEMSettingsPrivate();

    void updateOEMSettings();

    bool isHideShareByEmail();
    bool isHidePersonalGroup();
    bool isHideFeedback();
    bool isHideRegister();
    bool isEncryptPassword();
    bool isHideSocialLogin();
    bool isHideForgotPassword();
    bool isHideShare();                         //隐藏所有分享到外部方式
    bool isAccountPlaceholder();            //登录框显示的域账号
    bool isHideMyShare();
    bool isHideBuyVip();
    bool isForbidCreateBiz();
};

#endif // CWIZOEMSETTINGS_H
