#include "wizScreenShotWidget.h"
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>

CWizScreenShotWidget::CWizScreenShotWidget(QWidget* parent) :
    isSave(false)
  , QWidget(parent)
{
    tipWidth = 300; //温馨提示框的宽度
    tipHeight = 100; //温馨提示框的高度
    infoWidth = 100; //坐标信息框的宽度
    infoHeight = 50; //坐标信息框的高度

    initCWizScreenShotWidget();
}

void CWizScreenShotWidget::active()
{
    setWindowState(Qt::WindowActive|Qt::WindowFullScreen);
}

void CWizScreenShotWidget::initSelectedMenu()
{
    savePixmapAction = new QAction(tr("Save constituency"), this);
    cancelAction = new QAction(tr("Reselect"), this);
    quitAction = new QAction(tr("Quit"), this);
    contextMenu = new QMenu(this);

    connect(savePixmapAction, SIGNAL(triggered()), this, SLOT(savePixmap()));
    connect(cancelAction, SIGNAL(triggered()), this, SLOT(cancelSelectedRect()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
}

void CWizScreenShotWidget::quit()
{
    hideWidget();
    emit shotScreenQuit();
}

void CWizScreenShotWidget::savePixmap()
{
    hideWidget();
//    mainPixmap = shotPixmap;
//    isSave = true;
//    emit setNewPixmap(shotPixmap);
    emit finishPixmap(shotPixmap);
}

bool CWizScreenShotWidget::getIsSave()
{
    return isSave;
}

void CWizScreenShotWidget::setIsSave(bool is)
{
    isSave = is;
}

QPixmap CWizScreenShotWidget::getMainPixmap()
{
    return this->mainPixmap;
}

void CWizScreenShotWidget::loadBackgroundPixmap(const QPixmap &bgPixmap)
{
    int width,height;
    width = QApplication::desktop()->size().width();
    height = QApplication::desktop()->size().height();

    loadBackgroundPixmap(bgPixmap, 0, 0, width, height);
}

void CWizScreenShotWidget::loadBackgroundPixmap(const QPixmap &bgPixmap, int x, int y, int width, int height)
{
    loadPixmap = bgPixmap;
    screenx = x;
    screeny = y;
    screenwidth = width;
    screenheight = height;
    initCWizScreenShotWidget();
}

QPixmap CWizScreenShotWidget::getFullScreenPixmap()
{
    initCWizScreenShotWidget();
    QPixmap result = QPixmap();
    result = QPixmap::grabWindow(QApplication::desktop()->winId()); //抓取当前屏幕的图片

    return result;
}

void CWizScreenShotWidget::paintEvent(QPaintEvent *event)
{
    QColor shadowColor;
    shadowColor= QColor(0, 0, 0, 100); //阴影颜色设置
    painter.begin(this); //进行重绘

    painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::FlatCap));//设置画笔
    painter.drawPixmap(screenx, screeny, loadPixmap); //将背景图片画到窗体上
    painter.fillRect(screenx, screeny, screenwidth, screenheight, shadowColor); //画影罩效果

    switch (currentShotState)
    {
    case initShot:
        drawTipsText();
        break;
    case beginShot:
    case finishShot:
        selectedRect = getRect(beginPoint,endPoint); //获取选区
        drawSelectedPixmap();
        break;
    case beginMoveShot:
    case finishMoveShot:
        selectedRect = getMoveAllSelectedRect(); //获取选区
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
    drawXYWHInfo(); //打印坐标信息
    painter.end();  //重绘结束

    if (currentShotState == finishMoveShot || currentShotState == finishControl)
    {
        updateBeginEndPointValue(selectedRect); //当移动完选区后,更新beginPoint,endPoint,为下一次移动做准备工作
    }

}

void CWizScreenShotWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        initCWizScreenShotWidget();
        hideWidget();
    }
}

void CWizScreenShotWidget::mousePressEvent(QMouseEvent *event)
{
    //当开始进行拖动选择区域时,确定开始选取的beginPoint坐标
    if (event->button() == Qt::LeftButton && currentShotState == initShot)
    {
        currentShotState = beginShot; //设置当前状态为beginShot状态
        beginPoint = event->pos();
    }

    //移动选区改变选区的所在位置
    if (event->button() == Qt::LeftButton && isInSelectedRect(event->pos()) &&
            getMoveControlState(event->pos()) == moveControl0)
    {
        currentShotState = beginMoveShot; //启用开始移动选取选项,beginMoveShot状态
        moveBeginPoint = event->pos();
    }
    //移动控制点改变选区大小
    if (event->button() == Qt::LeftButton && getMoveControlState(event->pos()) != moveControl0)
    {
        currentShotState = beginControl; //开始移动控制点
        controlValue = getMoveControlState(event->pos());
        moveBeginPoint = event->pos();
    }
}

void CWizScreenShotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //当结束拖动选择区域时,确定结束选取的endPoint坐标
    if (event->button() == Qt::LeftButton && currentShotState == beginShot)
    {
        currentShotState = finishShot;
        endPoint = event->pos();
        update();
    }

    //当结束移动选区改变选区的所在位置时，确定结束选取的moveEndPoint坐标
    if (event->button() == Qt::LeftButton && currentShotState == beginMoveShot)
    {
        currentShotState = finishMoveShot;
        moveEndPoint = event->pos();
        update();
    }

    //当前状态为beginControl状态时，设置状态为finishControl并确定moveEndPoint的坐标
    if (event->button() == Qt::LeftButton && currentShotState == beginControl)
    {
        currentShotState = finishControl;
        moveEndPoint = event->pos();
        update();
    }
}

void CWizScreenShotWidget::mouseMoveEvent(QMouseEvent *event)
{
    //当拖动时，动态的更新所选择的区域
    if (currentShotState == beginShot)
    {
        endPoint = event->pos();
        update();
    }

    //当确定选区后，对选区进行移动操作
    if (currentShotState == beginMoveShot || currentShotState == beginControl)
    {
        moveEndPoint = event->pos();
        update();
    }

    updateMouseShape(event->pos()); //修改鼠标的形状
    setMouseTracking(true);
}

void CWizScreenShotWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (currentShotState == finishShot || currentShotState == finishMoveShot || currentShotState == finishControl)
    {
        QRect rcSelect = getMoveControlSelectedRect();
        hideWidget();
        if (rcSelect.contains(event->pos()))
            emit finishPixmap(shotPixmap);
    }
}

QRect CWizScreenShotWidget::getRect(const QPoint &beginPoint, const QPoint &endPoint)
{
    int x, y, width, height;
    width = qAbs(beginPoint.x() - endPoint.x());
    height = qAbs(beginPoint.y() - endPoint.y());
    x = beginPoint.x() < endPoint.x() ? beginPoint.x() : endPoint.x();
    y = beginPoint.y() < endPoint.y() ? beginPoint.y() : endPoint.y();

    return QRect(x,y,width,height);
}

void CWizScreenShotWidget::initCWizScreenShotWidget()
{
    currentShotState = initShot;
    controlValue = moveControl0;
    beginPoint = QPoint(0, 0);
    endPoint = QPoint(0, 0);
    moveBeginPoint = QPoint(0, 0);
    moveEndPoint = QPoint(0, 0);

    tlRect = QRect(0, 0, 0, 0); //左上点
    trRect = QRect(0, 0, 0, 0); //上右点
    blRect = QRect(0, 0, 0, 0); //左下点
    brRect = QRect(0, 0, 0, 0); //右下点
    tcRect = QRect(0, 0, 0, 0); //上中点
    bcRect = QRect(0, 0, 0, 0); //下中点
    lcRect = QRect(0, 0, 0, 0); //左中点
    rcRect = QRect(0, 0, 0, 0); //右中点

    setCursor(Qt::CrossCursor);
    update();
}

bool CWizScreenShotWidget::isInSelectedRect(const QPoint &point)
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

void CWizScreenShotWidget::cancelSelectedRect()
{
    initCWizScreenShotWidget();
    update(); //进行重绘，将选取区域去掉
}

void CWizScreenShotWidget::contextMenuEvent(QContextMenuEvent *event)
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

void CWizScreenShotWidget::hideWidget()
{
    setWindowState(Qt::WindowMinimized);
    hide();
}

void CWizScreenShotWidget::drawTipsText()
{
    int x = (screenwidth - tipWidth) / 2;
    int y = (screenheight - tipHeight) / 2;

    QColor color = QColor(100, 100, 100, 200);
    QRect rect = QRect(x, y, tipWidth, tipHeight);
    QString strTipsText = QString(tr("Tips \n mouse dragging screenshots; area right in the screenshots saved; \n screenshots area right outside cancellation; ESC to exit;"));

    painter.fillRect(rect, color);
    painter.setPen(QPen(Qt::white));//设置画笔的颜色为白色
    painter.drawText(rect, Qt::AlignCenter, strTipsText);

}

QRect CWizScreenShotWidget::getSelectedRect()
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

void CWizScreenShotWidget::updateBeginEndPointValue(const QRect &rect)
{
    beginPoint = rect.topLeft();
    endPoint = rect.bottomRight();

    moveBeginPoint = QPoint(0, 0);
    moveEndPoint = QPoint(0, 0);
}

void CWizScreenShotWidget::checkMoveEndPoint()
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

void CWizScreenShotWidget::draw8ControlPoint(const QRect &rect)
{
    int x, y;
    QColor color= QColor(0, 0, 255); //画点的颜色设置
    QPoint tlPoint = rect.topLeft(); //左上点
    QPoint trPoint = rect.topRight(); //右上点
    QPoint blPoint = rect.bottomLeft(); //左下点
    QPoint brPoint = rect.bottomRight(); //右下点

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

    tlRect = QRect(tlPoint.x() - 2, tlPoint.y() - 2, 6, 6); //左上点
    trRect = QRect(trPoint.x() - 2, trPoint.y() - 2, 6, 6); //右上点
    blRect = QRect(blPoint.x() - 2, blPoint.y() - 2, 6, 6); //左下点
    brRect = QRect(brPoint.x() - 2, brPoint.y() - 2, 6, 6); //右下点
    tcRect = QRect(tcPoint.x() - 2, tcPoint.y() - 2, 6, 6); //上中点
    bcRect = QRect(bcPoint.x() - 2, bcPoint.y() - 2, 6, 6); //下中点
    lcRect = QRect(lcPoint.x() - 2, lcPoint.y() - 2, 6, 6);//左中点
    rcRect = QRect(rcPoint.x() - 2, rcPoint.y() - 2, 6, 6); //右中点

    painter.fillRect(tlRect, color);
    painter.fillRect(trRect, color);
    painter.fillRect(blRect, color);
    painter.fillRect(brRect, color);
    painter.fillRect(tcRect, color);
    painter.fillRect(bcRect, color);
    painter.fillRect(lcRect, color);
    painter.fillRect(rcRect, color);
}

void CWizScreenShotWidget::updateMouseShape(const QPoint &point)
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
        updateMoveControlMouseShape(controlValue); //调用函数对移动8个控制点进行鼠标状态的改变
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void CWizScreenShotWidget::updateMoveControlMouseShape(controlPointEnum controlValue){
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

CWizScreenShotWidget::controlPointEnum CWizScreenShotWidget::getMoveControlState(const QPoint &point)
{
    CWizScreenShotWidget::controlPointEnum result = moveControl0;
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

QRect CWizScreenShotWidget::getMoveAllSelectedRect(void)
{
    QRect result;
    QPoint tmpBeginPoint, tmpEndPoint;
    int moveX, moveY;
    checkMoveEndPoint(); //对移动选区进行判断，当移动的选区超出边界，则停止移动
    moveX = moveEndPoint.x() - moveBeginPoint.x();
    moveY = moveEndPoint.y() - moveBeginPoint.y();
    tmpBeginPoint.setX(beginPoint.x() + moveX);
    tmpBeginPoint.setY(beginPoint.y() + moveY);
    tmpEndPoint.setX(endPoint.x() + moveX);
    tmpEndPoint.setY(endPoint.y() + moveY);

    result = getRect(tmpBeginPoint, tmpEndPoint);
    return result;
}

QRect CWizScreenShotWidget::getMoveControlSelectedRect(void)
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

int CWizScreenShotWidget::getMinValue(int num1, int num2)
{
    return num1 < num2 ? num1 : num2;
}

void CWizScreenShotWidget::drawSelectedPixmap(void)
{
    painter.drawRect(selectedRect); //画选中的矩形框
    shotPixmap = loadPixmap.copy(selectedRect);  //更新选区的Pixmap
    if (selectedRect.width() > 0 && selectedRect.height())
    {
        painter.drawPixmap(selectedRect.topLeft(), shotPixmap); //画选中区域的图片
    }
    draw8ControlPoint(selectedRect); //画出选区的8个控制点
}

void CWizScreenShotWidget::drawXYWHInfo(void)
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
        strTipsText = QString(tr("Coordinate information \n x:% 1 y:% 2 \n w:% 3 h:% 4")).arg(selectedRect.x(), 4).arg(selectedRect.y(), 4)
                .arg(selectedRect.width(),4).arg(selectedRect.height(), 4);
        painter.fillRect(rect, color);
        painter.setPen(QPen(Qt::black));//设置画笔的颜色为黑色
        painter.drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, strTipsText);
        break;
    default:
        break;
    }
}


CWizScreenShotHelper::CWizScreenShotHelper()
{
    m_widget = new CWizScreenShotWidget();
    connect(m_widget, SIGNAL(finishPixmap(QPixmap)), SIGNAL(screenShotCaptured(QPixmap)));
    connect(m_widget, SIGNAL(shotScreenQuit()), SIGNAL(shotScreenQuit()));
}

CWizScreenShotHelper::~CWizScreenShotHelper()
{
    if (m_widget)
        delete m_widget;
}

void CWizScreenShotHelper::startScreenShot()
{
    QPixmap pixmap = m_widget->getFullScreenPixmap();
    m_widget->loadBackgroundPixmap(pixmap);
    m_widget->show();
    m_widget->active();
}

