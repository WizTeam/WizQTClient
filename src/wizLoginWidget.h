#ifndef WIZLOGINWIDGET_H
#define WIZLOGINWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

#ifndef Q_OS_MAC
#include "share/wizshadowwindow.h"
#endif

class QLabel;

class CWizIconLineEditContainer : public QWidget
{
    Q_OBJECT
public:
    CWizIconLineEditContainer(QWidget* parent);
private:
    CWizSkin9GridImage* m_background;
    QLayout* m_layout;
    QLineEdit* m_edit;
    QLabel* m_leftIcon;
    QLabel* m_dropdownIcon;
public:
    void setBackgroundImage(QString fileName, QPoint pt);
    void setLeftIcon(QString fileName);
    void setDropdownIcon(QString fileName);
    //
    QLineEdit* edit() const { return m_edit; }
protected:
    virtual void paintEvent(QPaintEvent *event);
};

class LoginLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LoginLineEdit(QWidget *parent = 0);
    void setElementStyle(const QString& strBgFile, QLineEdit::EchoMode mode = QLineEdit::Normal, const QString& strPlaceHoldTxt = "");
    void setErrorStatus(bool bErrorStatus);
public slots:
    virtual void on_containt_changed(const QString& strText);

protected:
    void paintEvent(QPaintEvent *event);

    QRect getExtraIconBorder();
protected:
    QPixmap m_extraIcon;
};

class LoginMenuLineEdit : public LoginLineEdit
{
    Q_OBJECT
public:
    LoginMenuLineEdit(QWidget* parent = 0);

public slots:
    void on_containt_changed(const QString& strText);

signals:
    void showMenuRequest(QPoint point);

protected:
    void mousePressEvent(QMouseEvent* event);
};

class LoginButton : public QPushButton
{
    Q_OBJECT
public:
    LoginButton(QWidget *parent = 0);
    void setElementStyle();

public slots:
    void setEnabled(bool bEnable);
};





namespace Ui {
class wizLoginWidget;
}

class CWizLoginWidget
#ifdef Q_OS_MAC
        : public QDialog
#else
        : public CWizShadowWindow<QDialog>
#endif
{
    Q_OBJECT

public:
    explicit CWizLoginWidget(const QString& strDefaultUserId, const QString& strLocale, QWidget *parent = 0);
    ~CWizLoginWidget();

    QString userId() const;
    QString password() const;

    void setUsers(const QString& strDefault);
    void setUser(const QString& strUserId);

    void doAccountVerify();
    void doOnlineVerify();
    bool updateGlobalProfile();
    bool updateUserProfile(bool bLogined);
    void enableLoginControls(bool bEnable);
    void enableSignInControls(bool bEnable);

#ifdef Q_OS_MAC
private:
    QPoint m_mousePoint;
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
#endif
private slots:
    void on_btn_close_clicked();
    void on_btn_changeToSignin_clicked();
    void on_btn_changeToLogin_clicked();
    void on_btn_thridpart_clicked();
    void on_btn_fogetpass_clicked();
    void on_btn_login_clicked();
    void on_btn_singin_clicked();


    void onLoginInputChanged();
    void onSignUpInputDataChanged();
    void userListMenuClicked(QAction* action);
    void showUserListMenu(QPoint point);

    void onTokenAcquired(const QString& strToken);
    void onRegisterAccountFinished(bool bFinish);
    void on_cbx_remberPassword_toggled(bool checked);

    void on_cbx_autologin_toggled(bool checked);

    void on_lineEdit_userName_textEdited(const QString &arg1);

private:
    void setElementStyles();
    bool checkSingMessage();
    QAction* findActionInMenu(const QString& strActName);
private:
    Ui::wizLoginWidget *ui;
    QMenu* m_menu;
};

#endif // WIZLOGINWIDGET_H
