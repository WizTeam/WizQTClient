#ifndef WIZIMAGEBUTTON_H
#define WIZIMAGEBUTTON_H

#include <QPushButton>
#include <QString>

class QPaintEvent;
class QMouseEvent;

class WizImageButton : public QPushButton
{
    Q_OBJECT
public:
    explicit WizImageButton(QWidget* parent = 0);

    void setIcon(const QIcon& icon);
    void setIconNormal(const QPixmap& icoFile);
    void setIconHot(const QPixmap& icoFile);
    void setIconDown(const QPixmap& icoFile);

    void setLockNormalStatus(bool lock);

    void setStatusHot();
    void setStatusNormal();
    void setStatusDown();
    //
    QSize sizeHint() const override;

signals:

public slots:

protected slots:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent*  event);
    void	mouseReleaseEvent(QMouseEvent* event);

private:
    QPixmap m_normalIcon;
    QPixmap m_hotIcon;
    QPixmap m_downIcon;
    QPixmap m_currentIcon;
    QPixmap m_oldIcon;

    bool m_lockNormalStatus;
};

#endif // WIZIMAGEBUTTON_H
