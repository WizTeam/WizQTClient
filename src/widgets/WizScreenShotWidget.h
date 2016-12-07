#ifndef WIZSCREENSHOTWIDGET_H
#define WIZSCREENSHOTWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QtGui>
#include <QPainter>
#include <QBrush>

class QMenu;
class MainPixmap;
class WizScreenShotWidget : public QWidget
{
    Q_OBJECT
public:
    WizScreenShotWidget(QWidget* parent = 0);

    void active();

    enum shotState{initShot, beginShot, finishShot, endShot,
                   beginMoveShot, finishMoveShot, beginControl, finishControl};
    //move control point, default value is 0
    enum controlPointEnum{moveControl0, moveControl1, moveControl2, moveControl3, moveControl4,
                          moveControl5, moveControl6, moveControl7, moveControl8};

    QPixmap getFullScreenPixmap();

public slots:
    void loadBackgroundPixmap(const QPixmap &bgPixmap);
    void loadBackgroundPixmap(const QPixmap &bgPixmap, int x, int y, int width, int height);
    void cancelSelectedRect();
    void savePixmap();
    void quit();

signals:
    void finishPixmap(QPixmap finishPixmap);
    void shotScreenQuit();

private:

    //border control rect
    QRect tlRect; //topLeft
    QRect trRect; //topRight
    QRect blRect;
    QRect brRect;
    QRect tcRect;
    QRect bcRect;
    QRect lcRect;
    QRect rcRect;

    QPainter painter;
    QPoint beginPoint, endPoint, moveBeginPoint, moveEndPoint;
    QRect selectedRect;
    QPixmap loadPixmap, shotPixmap;
    shotState currentShotState;
    controlPointEnum controlValue;
    QAction *savePixmapAction; // action save image
    QAction *cancelAction; //action reselect rect
    QAction *quitAction; //action quit
    QMenu *contextMenu;

    int screenwidth;
    int screenheight;
    int screenx;
    int screeny;
    int tipWidth, tipHeight, infoWidth, infoHeight;

    QRect getSelectedRect();
    QRect getRect(const QPoint &beginPoint, const QPoint &endPoint); //get selected range by two point

    void initCWizScreenShotWidget();
    bool isInSelectedRect(const QPoint &point);
    void initSelectedMenu();
    void drawTipsText();
    void drawSelectedPixmap(void);
    void updateBeginEndPointValue(const QRect &rect);
    void checkMoveEndPoint();
    void draw8ControlPoint(const QRect &rect);
    void updateMouseShape(const QPoint &point);
    void updateMoveControlMouseShape(controlPointEnum controlValue);

    controlPointEnum getMoveControlState(const QPoint &point);
    QRect getMoveAllSelectedRect(void);
    QRect getMoveControlSelectedRect(void);

    int getMinValue(int num1, int num2);
    void drawXYWHInfo(void);


    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

    void hideWidget();
};


class WizScreenShotHelper : public QObject
{
    Q_OBJECT
public:
    WizScreenShotHelper();
    ~WizScreenShotHelper();

public slots:
    void startScreenShot();

signals:
    void shotScreenQuit();
    void screenShotCaptured(QPixmap pix);

private:
    WizScreenShotWidget *m_widget;
};


#endif // WIZSCREENSHOTWIDGET_H
