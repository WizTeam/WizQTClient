#ifndef WIZUSERINFOWIDGET_H
#define WIZUSERINFOWIDGET_H

#include <QToolButton>
#include <QPointer>

class QMenu;
class QFileDialog;

class CWizDatabase;
class CWizExplorerApp;
class CWizUserAvatarDownloaderHost;
class CWizWebSettingsDialog;


class CWizUserInfoWidget : public QToolButton
{
    Q_OBJECT

public:
    explicit CWizUserInfoWidget(CWizExplorerApp& app, QWidget *parent = 0);

protected:
    CWizExplorerApp& m_app;
    CWizDatabase& m_db;

    virtual void paintEvent(QPaintEvent *event);
    virtual QSize sizeHint() const;

private:
    CWizUserAvatarDownloaderHost* m_avatarDownloader;
    QIcon m_iconVipIndicator;
    QIcon m_iconArraw;
    QMenu* m_menuMain;

    QPointer<CWizWebSettingsDialog> m_userSettings;

protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual bool hitButton(const QPoint& pos) const;

private Q_SLOTS:
    void resetAvatar(bool bForce);
    void resetUserInfo();

    void on_userInfo_changed();

    void downloadAvatar();
    void on_userAvatar_downloaded(const QString& strGUID);
    void on_action_accountInfo_triggered();
    void on_action_accountSetup_triggered();
    void on_action_accountSetup_requested(const QString& strToken, const QString& strMsg);

    void on_action_changeAvatar_triggered();
    void on_action_changeAvatar_uploaded(bool ok);
};

#endif // WIZUSERINFOWIDGET_H
