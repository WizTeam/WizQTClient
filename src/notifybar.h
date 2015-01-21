#ifndef CORE_NOTIFYBAR_H
#define CORE_NOTIFYBAR_H

#include <QWidget>

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
        Locked,
        Deleted,
        PermissionLack,
        LockForGruop
    };

    explicit NotifyBar(QWidget *parent);
    void showPermissionNotify(int type);
    void showMessageTips(Qt::TextFormat format, const QString& info);

public slots:
    void on_closeButton_Clicked();

signals:
    void labelLink_clicked(const QString& link);

private:
    QLabel* m_labelNotify;
    wizImageButton* m_buttonClose;
    QPropertyAnimation* m_animation;

    void setStyleForPermission();
    void setStyleForEditing();

    void showNotify();
    void hideNotify(bool bUseAnimation);
};

} // namespace Internal
} // namespace Core

#endif // CORE_NOTIFYBAR_H
