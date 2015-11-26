#ifndef CORE_CELLBUTTON_H
#define CORE_CELLBUTTON_H

#include <QToolButton>
#include <QIcon>

class QSize;
class QPaintEvent;
class QString;
class QPropertyAnimation;

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

protected:
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
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;
    QString countInfo() const;
};

class RoundCellButton : public CellButton
{
    Q_OBJECT
public:
    explicit RoundCellButton(QWidget* parent = 0);

    void setNormalIcon(const QIcon& icon, const QString& text, const QString& strTips);
    void setCheckedIcon(const QIcon& icon, const QString& text, const QString& strTips);
    void setBadgeIcon(const QIcon& icon, const QString& text, const QString& strTips);

    QString text() const;
    int iconWidth() const;
    int buttonWidth() const;

public slots:
    void setState(int state);

protected:
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const;

    void applyAnimation();
protected:
    QString m_textNormal;
    QString m_textChecked;
    QString m_textBadge;

    QPropertyAnimation* m_animation;
};


}
} // namespace Core

#endif // CORE_CELLBUTTON_H
