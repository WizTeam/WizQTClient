#ifndef WIZIMAGEBUTTON_H
#define WIZIMAGEBUTTON_H

#include <QPushButton>
#include <QString>

class QPaintEvent;
class QMouseEvent;

class wizImageButton : public QPushButton
{
    Q_OBJECT
public:
    explicit wizImageButton(QWidget* parent = 0);

    void setIconNormal(const QString& icoFile);
    void setIconHot(const QString& icoFile);
    void setIconDown(const QString& icoFile);

    void setLockNormalStatus(bool lock);

    void setStatusHot();
    void setStatusNormal();
    void setStatusDown();

signals:

public slots:

protected slots:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent*  event);
    void	mouseReleaseEvent(QMouseEvent* event);

private:
    QString m_normalIcon;
    QString m_hotIcon;
    QString m_downIcon;
    QString m_currentIcon;

    bool m_lockNormalStatus;
};

#endif // WIZIMAGEBUTTON_H
