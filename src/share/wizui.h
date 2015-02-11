#ifndef WIZUI_H
#define WIZUI_H

#include "wizqthelper.h"
#include <QImage>
#include <QPushButton>

class QPainter;
class QLineEdit;
class QLabel;

class CWizSkin9GridImage
{
protected:
    QImage m_img;
    QRect m_arrayImageGrid[9];
    //
    BOOL Clear();
public:
    static BOOL SplitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount);
    BOOL SetImage(const CString& strImageFileName, QPoint ptTopLeft);
    //
    void Draw(QPainter* p, QRect rc, int nAlpha) const;
    void DrawBorder(QPainter* p, QRect rc) const;
    BOOL Valid() const;
    //
    QSize actualSize() const { return m_img.size(); }
};


class CWizIconLineEditContainer : public QWidget
{
    Q_OBJECT
public:
    CWizIconLineEditContainer(QWidget* parent);
    ~CWizIconLineEditContainer();
private:
    CWizSkin9GridImage* m_background;
    QLayout* m_layout;
    QLineEdit* m_edit;
    QLabel* m_leftIcon;
    QLabel* m_rightIcon;
public:
    void setBackgroundImage(QString fileName, QPoint pt);
    void setLeftIcon(QString fileName);
    void setRightIcon(QString fileName);
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

class CWizImageButton : public QPushButton
{
public:
    CWizImageButton(QWidget* parent);
public:
    void setButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                        const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                        const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor);

};



void WizInitWidgetMargins(const QString& skinName, QWidget* widget, const QString& name);
QWidget* WizInitWidgetMarginsEx(const QString& skinName, QWidget* widget, const QString& name);

#endif // WIZUI_H
