#ifndef WIZUSERINFOWIDGET_H
#define WIZUSERINFOWIDGET_H

#include <QToolButton>

class QMenu;
class CWizDatabase;
class CWizExplorerApp;
class CWizUserAvatarDownloaderHost;

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
    QMenu* m_menu;

    void resetAvatar();
    void resetUserInfo();

private Q_SLOTS:
    void downloadAvatar();
    void on_userAvatar_downloaded(const QString& strGUID);
};

#endif // WIZUSERINFOWIDGET_H
