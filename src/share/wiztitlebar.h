#ifndef WIZTITLEBAR_H
#define WIZTITLEBAR_H

#include <QWidget>

class QToolButton;

class CWizTitleBar : public QWidget
{
    Q_OBJECT
public:
    CWizTitleBar(QWidget *parent, QWidget* window, QWidget* shadowContainerWidget);
private:
    QWidget* m_window;
    QWidget* m_shadowContainerWidget;
    QMargins m_oldContentsMargin;
public slots:
    void showSmall();
    void showMaxRestore();
protected:
    void mousePressEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);
    void mouseDoubleClickEvent ( QMouseEvent * event );
private:
    QToolButton *minimize;
    QToolButton *maximize;
    QToolButton *close;
    QPixmap restorePix, maxPix;
    bool maxNormal;
    QPoint startPos;
    QPoint clickPos;
};



#endif // WIZTITLEBAR_H
