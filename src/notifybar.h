#ifndef CORE_NOTIFYBAR_H
#define CORE_NOTIFYBAR_H

#include <QWidget>

class QLabel;
class wizImageButton;

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
        PermissionLack
    };

    explicit NotifyBar(QWidget *parent);
    void showPermissionNotify(int type);
    void showEditingNotify(const QString& editor);

public slots:
    void on_closeButton_Clicked();

private:
    QLabel* m_labelNotify;
    wizImageButton* m_buttonClose;

    void setStyleForPermission();
    void setStyleForEditing();
};

} // namespace Internal
} // namespace Core

#endif // CORE_NOTIFYBAR_H
