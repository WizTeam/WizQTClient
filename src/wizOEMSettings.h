#ifndef CWIZOEMSETTINGS_H
#define CWIZOEMSETTINGS_H

#include "wizOEMSettings_p.h"

class CWizOEMSettings
{
public:

    CWizOEMSettings();
    ~CWizOEMSettings();

    static void updateOEMSettings(const QString& strUserId, const QString& strOEMJSONData);

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



#endif // CWIZOEMSETTINGS_H
