#ifndef WIZUI_H
#define WIZUI_H

#include "WizQtHelper.h"
#include <QImage>
#include <QPushButton>

class QPainter;
class QLineEdit;
class QLabel;

class WizSkin9GridImage
{
protected:
    QImage m_img;
    QRect m_arrayImageGrid[9];
    //
    BOOL clear();
public:
    static BOOL splitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount);
    BOOL setImage(const CString& strImageFileName, QPoint ptTopLeft, QColor darkColor = Qt::transparent);
    BOOL setImage(const QImage& image, QPoint ptTopLeft, QColor darkColor = Qt::transparent);
    //
    void draw(QPainter* p, QRect rc, int nAlpha) const;
    void drawBorder(QPainter* p, QRect rc) const;
    BOOL valid() const;
    //
    QSize actualSize() const { return m_img.size(); }
};


class WizIconLineEditContainer : public QWidget
{
    Q_OBJECT
public:
    WizIconLineEditContainer(QWidget* parent);
    ~WizIconLineEditContainer();
private:
    WizSkin9GridImage* m_background;
    QLayout* m_layout;
    QLineEdit* m_edit;
    QLabel* m_leftIcon;
    QLabel* m_rightIcon;
public:
    void setBackgroundImage(QString fileName, QPoint pt, QColor darkColor = Qt::transparent);
    void setLeftIcon(QString fileName);
    void setRightIcon(QString fileName);
    void setRightIcon(const QIcon& icon);
    void setPlaceholderText(const QString& strText);
    void setAutoClearRightIcon(bool bAutoClean);
    //
    QLineEdit* edit() const { return m_edit; }

signals:
    void rightIconClicked();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent* event);

private slots:
    void cleanRightIcon();
};

class WizStyleButton : public QPushButton
{
public:
    WizStyleButton(QWidget* parent);
public:
    void setButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                        const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                        const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor);

};



void WizInitWidgetMargins(const QString& skinName, QWidget* widget, const QString& name);
QWidget* WizInitWidgetMarginsEx(const QString& skinName, QWidget* widget, const QString& name);

#endif // WIZUI_H
