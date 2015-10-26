#ifndef CORE_CELLBUTTON_H
#define CORE_CELLBUTTON_H

#include <QToolButton>
#include <QIcon>

class QSize;
class QPaintEvent;
class QString;

namespace Core {
namespace Internal {


class CellButton : public QToolButton
{
    Q_OBJECT

public:
    enum ButtonType {
        ImageOnly,
        WithCountInfo
    };

    enum State {
        Normal,
        Checked,
        Badge
    };

    explicit CellButton(ButtonType type, QWidget* parent);
    void setNormalIcon(const QIcon& icon, const QString& strTips);
    void setCheckedIcon(const QIcon& icon, const QString& strTips);
    void setBadgeIcon(const QIcon& icon, const QString& strTips);
    int state() const { return m_state; }

public slots:
    void setState(int state);
    void setCount(int count);

private:
    ButtonType m_buttonType;
    int m_state;
    int m_count;
    QSize m_iconSize;
    QIcon m_iconNomal;
    QIcon m_iconChecked;
    QIcon m_iconBadge;
    QString m_strTipsNormal;
    QString m_strTipsChecked;
    QString m_strTipsBagde;

    //QIcon m_backgroundIcon;

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual QSize sizeHint() const;
    QString countInfo() const;
};

}
} // namespace Core

#endif // CORE_CELLBUTTON_H
