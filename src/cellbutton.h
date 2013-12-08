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
    enum Position {
        Left,
        Center,
        Right
    };

    enum State {
        Normal,
        Checked,
        Badge
    };

    explicit CellButton(Position pos, QWidget* parent);
    void setNormalIcon(const QIcon& icon, const QString& strTips);
    void setCheckedIcon(const QIcon& icon, const QString& strTips);
    void setBadgeIcon(const QIcon& icon, const QString& strTips);
    void setState(int state);
    int state() const { return m_state; }

private:
    Position m_pos;
    int m_state;
    QIcon m_iconNomal;
    QIcon m_iconChecked;
    QIcon m_iconBadge;
    QString m_strTipsNormal;
    QString m_strTipsChecked;
    QString m_strTipsBagde;

    QIcon m_backgroundIcon;

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual QSize sizeHint() const;

};

}
} // namespace Core

#endif // CORE_CELLBUTTON_H
