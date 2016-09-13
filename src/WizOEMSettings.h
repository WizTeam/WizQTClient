#ifndef CWIZOEMSETTINGS_H
#define CWIZOEMSETTINGS_H

#include <QSettings>

class WizOEMSettings : public QSettings
{
public:
    WizOEMSettings(const QString& strUserAccountPath);

    static bool settingFileExists(const QString& strUserAccountPath);
    //
    static void updateOEMSettings(const QString& strUserAccountPath, const QString& strOEMJSONData);

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

    //
    void setLogoPath(const QString& path);
    QString logoPath();

private:


};



#endif // CWIZOEMSETTINGS_H
