#ifndef CORE_NOTIFYBAR_H
#define CORE_NOTIFYBAR_H

#include <QWidget>

class QLabel;

namespace Core {
namespace Internal {

class NotifyBar : public QWidget
{
public:
    enum NotifyType
    {
        Locked,
        Deleted,
        PermissionLack
    };

    explicit NotifyBar(QWidget *parent);
    void showNotify(int type);

private:
    QLabel* m_labelNotify;
};

} // namespace Internal
} // namespace Core

#endif // CORE_NOTIFYBAR_H
