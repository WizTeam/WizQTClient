#include "WizDocumentSelectionView.h"

#include <QGraphicsView>
#include <QGraphicsObject>
#include <QVBoxLayout>

#include "WizDef.h"
#include "WizMainWindow.h"
#include "share/WizDatabaseManager.h"


#define WIZ_SELECTION_ITEM_MAX 5
#define WIZ_SELECTION_ITEM_MARGIN_Y 200
#define WIZ_SELECTION_ITEM_OFFSET_X 50
#define WIZ_SELECTION_ITEM_OFFSET_Y 50

class CWizDocumentGraphicsPixmapItem : public QGraphicsObject
{
public:
    CWizDocumentGraphicsPixmapItem(const WIZDOCUMENTDATA& data, const QPixmap& pixmap, QGraphicsItem* parent = 0)
        : QGraphicsObject(parent)
        , m_doc(data)
        , m_pixmap(pixmap)
    {
    }

   virtual QRectF boundingRect() const
   {
        if (m_pixmap.isNull())
            return QRectF();
        if (flags() & QGraphicsItem::ItemIsSelectable) {
            qreal pw = 1.0;
            return QRectF(pos(), m_pixmap.size()).adjusted(-pw/2, -pw/2, pw/2, pw/2);
        } else {
            return QRectF(pos(), m_pixmap.size());
        }
   }


    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(pos(), m_pixmap);

        //if (option->state & QStyle::State_Selected)
        //    qt_graphicsItem_highlightSelected(this, painter, option);
    }

    const WIZDOCUMENTDATA& document() const { return m_doc; }

private:
    WIZDOCUMENTDATA m_doc;
    QPixmap m_pixmap;
};


WizDocumentSelectionView::WizDocumentSelectionView(WizExplorerApp& app, QWidget *parent)
    : m_app(app)
    , m_dbMgr(app.databaseManager())
    , QWidget(parent)
{
//    MainWindow* mainWindow = qobject_cast<MainWindow*>(app.mainWindow());
//    CWizDocumentListView* listView = qobject_cast<CWizDocumentListView*>(mainWindow->DocumentsCtrl());
    //m_thumbCache = listView->thumbCache();
    //connect(m_thumbCache, SIGNAL(loaded(const WIZABSTRACT&)), SLOT(on_thumbCache_loaded(const WIZABSTRACT&)));

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //scene->setSceneRect(-100, -100, 500, 500);

    m_graphicsView = new QGraphicsView(scene, this);
    m_graphicsView->setBackgroundBrush(WizGetLeftViewBrush());
    m_graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    layout->addWidget(m_graphicsView);
}

void WizDocumentSelectionView::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    m_graphicsView->scene()->clear();
}

void WizDocumentSelectionView::requestDocuments(const CWizDocumentDataArray& arrayDocument)
{
    int nTotal = qMin((int)arrayDocument.size(), WIZ_SELECTION_ITEM_MAX);

    m_docs.clear();
    CWizDocumentDataArray::const_reverse_iterator rit = arrayDocument.rbegin();
    m_docs.insert(m_docs.begin(), rit, rit + nTotal);

    CWizDocumentDataArray::iterator it;
    for(it = m_docs.begin(); it != m_docs.end(); it++) {
        const WIZDOCUMENTDATAEX& doc = *it;
        //m_thumbCache->load(doc.strKbGUID, doc.strGUID);
    }
}

void WizDocumentSelectionView::on_thumbCache_loaded(const WIZABSTRACT& abs)
{
    if (!isThumbNeedBuild(abs)) {
        return;
    }

    WIZDOCUMENTDATA data;
    m_dbMgr.db(abs.strKbGUID).documentFromGuid(abs.guid, data);
    QPixmap pixmap  = getDocumentPixmap(data, abs);

    CWizDocumentGraphicsPixmapItem* item = new CWizDocumentGraphicsPixmapItem(data, pixmap);
    m_graphicsView->scene()->addItem(item);
    //m_graphicsView->scene()->advance();

    QPropertyAnimation* animation = new QPropertyAnimation(item, "pos");
    animation->setEasingCurve(QEasingCurve::OutBounce);
    animation->setDuration(500);
    animation->setStartValue(QPoint(0, 500));
    animation->setEndValue(getThumbPosition(abs));
    animation->start();
}

bool WizDocumentSelectionView::isThumbNeedBuild(const WIZABSTRACT& abs)
{
    bool bFound = false;
    WIZDOCUMENTDATA data;

    CWizDocumentDataArray::iterator it;
    for(it = m_docs.begin(); it != m_docs.end(); it++) {
        const WIZDOCUMENTDATAEX& doc = *it;
        if (abs.strKbGUID == doc.strKbGUID && abs.guid == doc.strKbGUID) {
            data = doc;
            bFound = true;
            break;
        }
    }

    if (bFound) {
        return true;
    }

    // search on item on screen


    return true;
}

QPoint WizDocumentSelectionView::getThumbPosition(const WIZABSTRACT& abs)
{
    for (int i = 0; i < m_docs.size(); i++) {
        const WIZDOCUMENTDATAEX& doc = m_docs.at(i);
        if (abs.strKbGUID == doc.strKbGUID && abs.guid == doc.strKbGUID) {
            int x = (size().width() - (WIZ_SELECTION_ITEM_MAX - 1) * WIZ_SELECTION_ITEM_OFFSET_X) / 2 + WIZ_SELECTION_ITEM_OFFSET_X * i;
            int y =  WIZ_SELECTION_ITEM_MARGIN_Y + WIZ_SELECTION_ITEM_OFFSET_Y * qAbs(i - (WIZ_SELECTION_ITEM_MAX -1) / 2);
            return QPoint(x, y);
        }
    }

    return QPoint(0, 0);
}

QPixmap WizDocumentSelectionView::getDocumentPixmap(const WIZDOCUMENTDATA& doc,
                                                     const WIZABSTRACT& abs)
{
    // default values
    QSize szPixmap(200, 200);
    int nMarginX = 10, nMarginY = 10;

    QPixmap pixmap(szPixmap);
    pixmap.fill(Qt::white);

    QPainter p(&pixmap);

    // draw title
    QFont font = p.font();
    font.setPointSize(11);
    font.setBold(true);
    p.setFont(font);

    int titleHeight = p.fontMetrics().height();
    QString strTitle = p.fontMetrics().elidedText(doc.strTitle, Qt::ElideRight, pixmap.width() - nMarginX * 2);
    p.drawText(nMarginX, titleHeight, strTitle);

    // draw seperate line
    p.drawLine(QPoint(nMarginX, titleHeight + nMarginY), QPoint(pixmap.width() - nMarginX, titleHeight + nMarginY));

    // draw text
    font.setPointSize(10);
    font.setBold(false);
    p.setFont(font);

    // if thumb image exist, draw restricted rect text
    QRect textRect;
    if (!abs.image.isNull()) {
        textRect = QRect(nMarginX, titleHeight + nMarginY * 2, pixmap.width() - nMarginX * 2, titleHeight * 2); // two lines
    } else {
        textRect = QRect(nMarginX, titleHeight + nMarginY * 2, pixmap.width() - nMarginX * 2, pixmap.height() - titleHeight - nMarginY * 3);
    }

    p.drawText(textRect, Qt::TextWrapAnywhere, abs.text);

    // draw thumb image
    const QImage& img = abs.image;
    if (!img.isNull()) {
        QRect imageRect(0, textRect.y() + textRect.height(), szPixmap.width(), szPixmap.height() - textRect.y());
        p.drawImage(imageRect, img, QRect(0, 0, 0, 0));
    }

    return pixmap;
}
