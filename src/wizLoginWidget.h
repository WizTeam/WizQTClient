#ifndef WIZLOGINWIDGET_H
#define WIZLOGINWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class LoginLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LoginLineEdit(QWidget *parent = 0);
    void setElementStyle(const QString& strBgFile, QLineEdit::EchoMode mode = QLineEdit::Normal, const QString& strPlaceHoldTxt = "");
    void setErrorStatus(bool bErrorStatus);
public slots:
    void on_containt_changed(const QString& strText);

signals:
    void editorFocusIn();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void focusInEvent(QFocusEvent *);

private:
    QPixmap m_extraStatus;
};

class LoginMenuLineEdit : public LoginLineEdit
{
    Q_OBJECT
public:
    LoginMenuLineEdit(QWidget* parent = 0);

signals:
    void showMenuRequest(QPoint point);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent* event);

};

class LoginButton : public QPushButton
{
    Q_OBJECT
public:
    LoginButton(QWidget *parent = 0);
    void setElementStyle();

public slots:
    void on_password_changed(const QString& strText);
};

class LoginTipWidget : public QWidget
{
    Q_OBJECT
public:
    LoginTipWidget(QWidget *parent = 0);
};

namespace Ui {
class wizLoginWidget;
}

class CWizLoginWidget : public QDialog
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

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private slots:
    void on_btn_close_clicked();
    void on_btn_changeToSignin_clicked();
    void on_btn_changeToLogin_clicked();
    void on_btn_thridpart_clicked();
    void on_btn_fogetpass_clicked();
    void on_btn_login_clicked();
    void on_btn_singin_clicked();


    void inputDataChanged();
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
private:
    Ui::wizLoginWidget *ui;
    QPoint m_mousePoint;

    QMenu* m_menu;
};

#endif // WIZLOGINWIDGET_H
