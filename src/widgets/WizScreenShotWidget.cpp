#include "WizScreenShotWidget.h"
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QScreen>

WizScreenShotWidget::WizScreenShotWidget(QWidget* parent) :
  QWidget(parent)
{
    tipWidth = 500;
    tipHeight = 100;
    infoWidth = 120;
    infoHeight = 50;

    initCWizScreenShotWidget();
}

void WizScreenShotWidget::active()
{
    //setWindowState(Qt::WindowActive|Qt::WindowFullScreen);
    showFullScreen();
}

void WizScreenShotWidget::initSelectedMenu()
{
    savePixmapAction = new QAction(tr("Save constituency"), this);
    cancelAction = new QAction(tr("Reselect"), this);
    quitAction = new QAction(tr("Quit"), this);
    contextMenu = new QMenu(this);

    connect(savePixmapAction, SIGNAL(triggered()), this, SLOT(savePixmap()));
    connect(cancelAction, SIGNAL(triggered()), this, SLOT(cancelSelectedRect()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
}

void WizScreenShotWidget::quit()
{
    hideWidget();
    emit shotScreenQuit();
}

void WizScreenShotWidget::savePixmap()
{
    hideWidget();
    emit finishPixmap(shotPixmap);
}

void WizScreenShotWidget::loadBackgroundPixmap(const QPixmap &bgPixmap)
{
    int width,height;
    width = QApplication::desktop()->size().width();
    height = QApplication::desktop()->size().height();

    loadBackgroundPixmap(bgPixmap, 0, 0, width, height);
}

void WizScreenShotWidget::loadBackgroundPixmap(const QPixmap &bgPixmap, int x, int y, int width, int height)
{
    loadPixmap = bgPixmap;
    screenx = x;
    screeny = y;
    screenwidth = width;
    screenheight = height;
    initCWizScreenShotWidget();
}

QPixmap WizScreenShotWidget::getFullScreenPixmap()
{
    initCWizScreenShotWidget();
    QPixmap result = QPixmap();
    QScreen *screen = QGuiApplication::primaryScreen();
    result = screen->grabWindow(0);

    return result;
}

void WizScreenShotWidget::paintEvent(QPaintEvent *event)
{
    QColor shadowColor;
    shadowColor= QColor(0, 0, 0, 100);
    painter.begin(this);

    painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::FlatCap));
    painter.drawPixmap(screenx, screeny, loadPixmap);
    painter.fillRect(screenx, screeny, screenwidth, screenheight, shadowColor);

    switch (currentShotState)
    {
    case initShot:
        drawTipsText();
        break;
    case beginShot:
    case finishShot:
        selectedRect = getRect(beginPoint,endPoint); // get selected range
        drawSelectedPixmap();
        break;
    case beginMoveShot:
    case finishMoveShot:
        selectedRect = getMoveAllSelectedRect();  // get selected range
        drawSelectedPixmap();
        break;
    case beginControl:
    case finishControl:
        selectedRect = getMoveControlSelectedRect();
        drawSelectedPixmap();
        break;
    default:
        break;
    }
    drawXYWHInfo(); //draw point info
    painter.end();

    if (currentShotState == finishMoveShot || currentShotState == finishControl)
    {
        updateBeginEndPointValue(selectedRect); //prepare for next move
    }

}

void WizScreenShotWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        initCWizScreenShotWidget();
        hideWidget();
    }
}

void WizScreenShotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && currentShotState == initShot)
    {
        currentShotState = beginShot;
        beginPoint = event->pos();
    }

    if (event->button() == Qt::LeftButton && isInSelectedRect(event->pos()) &&
            getMoveControlState(event->pos()) == moveControl0)
    {
        currentShotState = beginMoveShot;
        moveBeginPoint = event->pos();
    }

    if (event->button() == Qt::LeftButton && getMoveControlState(event->pos()) != moveControl0)
    {
        currentShotState = beginControl;
        controlValue = getMoveControlState(event->pos());
        moveBeginPoint = event->pos();
    }
}

void WizScreenShotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && currentShotState == beginShot)
    {
        currentShotState = finishShot;
        endPoint = event->pos();
        update();
    }

    if (event->button() == Qt::LeftButton && currentShotState == beginMoveShot)
    {
        currentShotState = finishMoveShot;
        moveEndPoint = event->pos();
        update();
    }

    if (event->button() == Qt::LeftButton && currentShotState == beginControl)
    {
        currentShotState = finishControl;
        moveEndPoint = event->pos();
        update();
    }
}

void WizScreenShotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (currentShotState == beginShot)
    {
        endPoint = event->pos();
        update();
    }

    if (currentShotState == beginMoveShot || currentShotState == beginControl)
    {
        moveEndPoint = event->pos();
        update();
    }

    updateMouseShape(event->pos());
    setMouseTracking(true);
}

void WizScreenShotWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (currentShotState == finishShot || currentShotState == finishMoveShot || currentShotState == finishControl)
    {
        QRect rcSelect = getMoveControlSelectedRect();
        hideWidget();
        if (rcSelect.contains(event->pos()))
            emit finishPixmap(shotPixmap);
    }
}

QRect WizScreenShotWidget::getRect(const QPoint &beginPoint, const QPoint &endPoint)
{
    int x, y, width, height;
    width = qAbs(beginPoint.x() - endPoint.x());
    height = qAbs(beginPoint.y() - endPoint.y());
    x = beginPoint.x() < endPoint.x() ? beginPoint.x() : endPoint.x();
    y = beginPoint.y() < endPoint.y() ? beginPoint.y() : endPoint.y();

    return QRect(x,y,width,height);
}

void WizScreenShotWidget::initCWizScreenShotWidget()
{
    currentShotState = initShot;
    controlValue = moveControl0;
    beginPoint = QPoint(0, 0);
    endPoint = QPoint(0, 0);
    moveBeginPoint = QPoint(0, 0);
    moveEndPoint = QPoint(0, 0);

    tlRect = QRect(0, 0, 0, 0);
    trRect = QRect(0, 0, 0, 0);
    blRect = QRect(0, 0, 0, 0);
    brRect = QRect(0, 0, 0, 0);
    tcRect = QRect(0, 0, 0, 0);
    bcRect = QRect(0, 0, 0, 0);
    lcRect = QRect(0, 0, 0, 0);
    rcRect = QRect(0, 0, 0, 0);

    setCursor(Qt::CrossCursor);
    update();
}

bool WizScreenShotWidget::isInSelectedRect(const QPoint &point)
{
    int x, y;
    QRect selectedRect;
    if (currentShotState == initShot || currentShotState == beginShot)
        return false;

    selectedRect = getSelectedRect();
    x = point.x();
    y = point.y();

    return selectedRect.contains(x, y);
}

void WizScreenShotWidget::cancelSelectedRect()
{
    initCWizScreenShotWidget();
    update();
}

void WizScreenShotWidget::contextMenuEvent(QContextMenuEvent *event)
{
    initSelectedMenu();

    if (isInSelectedRect(event->pos()))
    {
        contextMenu->addAction(savePixmapAction);
    }
    else
    {
        contextMenu->addAction(cancelAction);
        contextMenu->addAction(quitAction);
    }

    contextMenu->exec(event->pos());
}

void WizScreenShotWidget::hideWidget()
{
    //setWindowState(Qt::WindowMinimized);
    hide();
}

void WizScreenShotWidget::drawTipsText()
{
    int x = (screenwidth - tipWidth) / 2;
    int y = (screenheight - tipHeight) / 2;

    QColor color = QColor(100, 100, 100, 200);
    QRect rect = QRect(x, y, tipWidth, tipHeight);
    QString strTipsText = QString(tr("Tips \n mouse dragging screenshots; area right in the screenshots saved; \n screenshots area right outside cancellation; ESC to exit."));

    painter.fillRect(rect, color);
    painter.setPen(QPen(Qt::white));
    painter.drawText(rect, Qt::AlignCenter, strTipsText);

}

QRect WizScreenShotWidget::getSelectedRect()
{
    if (currentShotState == beginMoveShot)
    {
        return getMoveAllSelectedRect();
    }
    else if (currentShotState == beginControl)
    {
        return getMoveControlSelectedRect();
    }
    else
    {
        return getRect(beginPoint, endPoint);
    }
}

void WizScreenShotWidget::updateBeginEndPointValue(const QRect &rect)
{
    beginPoint = rect.topLeft();
    endPoint = rect.bottomRight();

    moveBeginPoint = QPoint(0, 0);
    moveEndPoint = QPoint(0, 0);
}

void WizScreenShotWidget::checkMoveEndPoint()
{
    int x,y;

    QRect selectedRect = getRect(beginPoint, endPoint);
    QPoint bottomRightPoint = selectedRect.bottomRight();

    x = moveEndPoint.x() - moveBeginPoint.x();
    y = moveEndPoint.y() - moveBeginPoint.y();

    if (x + selectedRect.x() < 0)
    { //当移动后X坐标小于零时，则出现选区丢失，则计算出moveEndPoint的X最大坐标值，进行赋值
        moveEndPoint.setX(qAbs(selectedRect.x() - moveBeginPoint.x()));
    }

    if (y + selectedRect.y() < 0)
    { //当移动后Y坐标小于零时，则出现选区丢失，则计算出moveEndPoint的Y最大坐标值，进行赋值
        moveEndPoint.setY(qAbs(selectedRect.y() - moveBeginPoint.y()));
    }

    if (x + bottomRightPoint.x() > screenwidth)
    { //当移动选区后，出现超出整个屏幕的右面时，设置moveEndPoint的X的最大坐标
        moveEndPoint.setX(screenwidth - bottomRightPoint.x() + moveBeginPoint.x());
    }

    if (y + bottomRightPoint.y() > screenheight)
    { //当移动选区后，出现超出整个屏幕的下面时，设置moveEndPoint的Y的最大坐标值
        moveEndPoint.setY(screenheight - bottomRightPoint.y() + moveBeginPoint.y());
    }
}

void WizScreenShotWidget::draw8ControlPoint(const QRect &rect)
{
    int x, y;
    QColor color= QColor(0, 0, 255);
    QPoint tlPoint = rect.topLeft();
    QPoint trPoint = rect.topRight();
    QPoint blPoint = rect.bottomLeft();
    QPoint brPoint = rect.bottomRight();

    x = (tlPoint.x() + trPoint.x()) / 2;
    y = tlPoint.y();
    QPoint tcPoint = QPoint(x, y);

    x = (blPoint.x() + brPoint.x()) / 2;
    y = blPoint.y();
    QPoint bcPoint = QPoint(x, y);

    x = tlPoint.x();
    y = (tlPoint.y() + blPoint.y()) / 2;
    QPoint lcPoint = QPoint(x, y);

    x = trPoint.x();
    y = (trPoint.y() + brPoint.y()) / 2;
    QPoint rcPoint = QPoint(x, y);

    tlRect = QRect(tlPoint.x() - 2, tlPoint.y() - 2, 6, 6);
    trRect = QRect(trPoint.x() - 2, trPoint.y() - 2, 6, 6);
    blRect = QRect(blPoint.x() - 2, blPoint.y() - 2, 6, 6);
    brRect = QRect(brPoint.x() - 2, brPoint.y() - 2, 6, 6);
    tcRect = QRect(tcPoint.x() - 2, tcPoint.y() - 2, 6, 6);
    bcRect = QRect(bcPoint.x() - 2, bcPoint.y() - 2, 6, 6);
    lcRect = QRect(lcPoint.x() - 2, lcPoint.y() - 2, 6, 6);
    rcRect = QRect(rcPoint.x() - 2, rcPoint.y() - 2, 6, 6);

    painter.fillRect(tlRect, color);
    painter.fillRect(trRect, color);
    painter.fillRect(blRect, color);
    painter.fillRect(brRect, color);
    painter.fillRect(tcRect, color);
    painter.fillRect(bcRect, color);
    painter.fillRect(lcRect, color);
    painter.fillRect(rcRect, color);
}

void WizScreenShotWidget::updateMouseShape(const QPoint &point)
{
    switch (currentShotState)
    {
    case initShot:
    case beginShot:
        setCursor(Qt::CrossCursor);
        break;
    case beginMoveShot:
        setCursor(Qt::OpenHandCursor);
        break;
    case finishShot:
    case finishMoveShot:
    case finishControl:
        if (getSelectedRect().contains(point))
            setCursor(Qt::OpenHandCursor);
        else
            updateMoveControlMouseShape(getMoveControlState(point));
        break;
    case beginControl:
        updateMoveControlMouseShape(controlValue);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void WizScreenShotWidget::updateMoveControlMouseShape(controlPointEnum controlValue){
    switch (controlValue)
    {
    case moveControl1:
    case moveControl5:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case moveControl2:
    case moveControl6:
        setCursor(Qt::SizeVerCursor);
        break;
    case moveControl3:
    case moveControl7:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case moveControl4:
    case moveControl8:
        setCursor(Qt::SizeHorCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

WizScreenShotWidget::controlPointEnum WizScreenShotWidget::getMoveControlState(const QPoint &point)
{
    WizScreenShotWidget::controlPointEnum result = moveControl0;
    if (currentShotState == initShot || currentShotState == beginShot)
    {
        result = moveControl0;
    }
    else if (tlRect.contains(point))
    {
        result = moveControl1;
    }
    else if (tcRect.contains(point))
    {
        result = moveControl2;
    }
    else if (trRect.contains(point))
    {
        result = moveControl3;
    }
    else if (rcRect.contains(point))
    {
        result = moveControl4;
    }
    else if (brRect.contains(point))
    {
        result = moveControl5;
    }
    else if (bcRect.contains(point))
    {
        result = moveControl6;
    }
    else if (blRect.contains(point))
    {
        result = moveControl7;
    }
    else if (lcRect.contains(point))
    {
        result = moveControl8;
    }
    else
    {
        result = moveControl0;
    }

    return result;
}

QRect WizScreenShotWidget::getMoveAllSelectedRect(void)
{
    QRect result;
    QPoint tmpBeginPoint, tmpEndPoint;
    int moveX, moveY;
    checkMoveEndPoint();
    moveX = moveEndPoint.x() - moveBeginPoint.x();
    moveY = moveEndPoint.y() - moveBeginPoint.y();
    tmpBeginPoint.setX(beginPoint.x() + moveX);
    tmpBeginPoint.setY(beginPoint.y() + moveY);
    tmpEndPoint.setX(endPoint.x() + moveX);
    tmpEndPoint.setY(endPoint.y() + moveY);

    result = getRect(tmpBeginPoint, tmpEndPoint);
    return result;
}

QRect WizScreenShotWidget::getMoveControlSelectedRect(void)
{
    int x, y, w, h;
    QRect rect = getRect(beginPoint,endPoint);
    QRect result;
    switch (controlValue)
    {
    case moveControl1:
        result = getRect(rect.bottomRight(), moveEndPoint);
        return result;
        break;
    case moveControl2:
        x = rect.x();
        y = getMinValue(moveEndPoint.y(), rect.bottomLeft().y());
        w = rect.width();
        h = qAbs(moveEndPoint.y() - rect.bottomRight().y());
        break;
    case moveControl3:
        result = getRect(rect.bottomLeft(), moveEndPoint);
        return result;
        break;
    case moveControl4:
        x = getMinValue(rect.x(), moveEndPoint.x());
        y = rect.y();
        w = qAbs(rect.bottomLeft().x() - moveEndPoint.x());
        h = rect.height();
        break;
    case moveControl5:
        result = getRect(rect.topLeft(), moveEndPoint);
        return result;
        break;
    case moveControl6:
        x = rect.x();
        y = getMinValue(rect.y(), moveEndPoint.y());
        w = rect.width();
        h = qAbs(moveEndPoint.y() - rect.topLeft().y());
        break;
    case moveControl7:
        result = getRect(moveEndPoint, rect.topRight());
        return result;
        break;
    case moveControl8:
        x = getMinValue(moveEndPoint.x(), rect.bottomRight().x());
        y = rect.y();
        w = qAbs(rect.bottomRight().x() - moveEndPoint.x());
        h = rect.height();
        break;
    default:
        result = getRect(beginPoint, endPoint);
        return result;
        break;
    }

    return QRect(x, y, w, h);
}

int WizScreenShotWidget::getMinValue(int num1, int num2)
{
    return num1 < num2 ? num1 : num2;
}

void WizScreenShotWidget::drawSelectedPixmap(void)
{
    painter.drawRect(selectedRect);
    shotPixmap = loadPixmap.copy(selectedRect);
    if (selectedRect.width() > 0 && selectedRect.height())
    {
        painter.drawPixmap(selectedRect.topLeft(), shotPixmap);
    }
    draw8ControlPoint(selectedRect);
}

void WizScreenShotWidget::drawXYWHInfo(void)
{
    int x, y;
    QColor color = QColor(239, 234, 228, 200);
    QRect rect;
    QString strTipsText;
    switch (currentShotState)
    {
    case beginShot:
    case finishShot:
    case beginMoveShot:
    case finishMoveShot:
    case beginControl:
    case finishControl:
        x = selectedRect.x() + 5;
        y = selectedRect.y() > infoHeight ? selectedRect.y() - infoHeight:selectedRect.y();
        rect = QRect(x,y,infoWidth,infoHeight);
        strTipsText = QString(tr("Coordinate information \n x: %1 y: %2 \n w: %3 h: %4")).arg(selectedRect.x(), 4).arg(selectedRect.y(), 4)
                .arg(selectedRect.width(),4).arg(selectedRect.height(), 4);
        painter.fillRect(rect, color);
        painter.setPen(QPen(Qt::black));
        painter.drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, strTipsText);
        break;
    default:
        break;
    }
}


WizScreenShotHelper::WizScreenShotHelper()
{
    m_widget = new WizScreenShotWidget();
    connect(m_widget, SIGNAL(finishPixmap(QPixmap)), SIGNAL(screenShotCaptured(QPixmap)));
    connect(m_widget, SIGNAL(shotScreenQuit()), SIGNAL(shotScreenQuit()));
}

WizScreenShotHelper::~WizScreenShotHelper()
{
    if (m_widget)
        delete m_widget;
}

void WizScreenShotHelper::startScreenShot()
{
    QPixmap pixmap = m_widget->getFullScreenPixmap();
    m_widget->loadBackgroundPixmap(pixmap);
    m_widget->show();
    m_widget->active();
}

