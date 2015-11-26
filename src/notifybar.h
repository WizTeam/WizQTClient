#ifndef CORE_NOTIFYBAR_H
#define CORE_NOTIFYBAR_H

#include <QWidget>
#include <QIcon>

class QLabel;
class wizImageButton;
class QPropertyAnimation;

#define NOTIFYBAR_LABELLINK_DOWNLOAD  "clicktodownload"

namespace Core {
namespace Internal {

class NotifyBar : public QWidget
{
    Q_OBJECT
public:
    enum NotifyType
    {
        NoNotify,
        Locked,
        Deleted,
        PermissionLack,
        LockForGruop,
        CustomMessage
    };

    explicit NotifyBar(QWidget *parent);
    void showPermissionNotify(int type);
    void showMessageTips(Qt::TextFormat format, const QString& info);
    void hideMessageTips(bool useAnimation);

public slots:
    void on_closeButton_Clicked();

signals:
    void labelLink_clicked(const QString& link);

private:
    QLabel* m_spacer;
    QWidget* m_childWgt;
    QLabel* m_labelNotify;
    wizImageButton* m_buttonCloseRed;
    wizImageButton* m_buttonCloseBlue;
    QPropertyAnimation* m_animation;
    NotifyType m_type;

    void setStyleForPermission();
    void setStyleForEditing();

    void showNotify();
    void hideNotify(bool bUseAnimation);
    void applyStyleSheet(bool isForbidden);
    void showCloseButton(bool isForbidden);
};

} // namespace Internal
} // namespace Core

#endif // CORE_NOTIFYBAR_H
