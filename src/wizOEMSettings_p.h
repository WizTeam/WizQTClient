#ifndef WIZOEMSETTINGS_P
#define WIZOEMSETTINGS_P

#include <QSettings>

class CWizOEMSettingsPrivate : public QSettings
{
public:
    CWizOEMSettingsPrivate(const QString & fileName);
    ~CWizOEMSettingsPrivate();

    void updateOEMSettings(const QString& strOEMData);

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

#endif // WIZOEMSETTINGS_P

