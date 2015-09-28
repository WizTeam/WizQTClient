#include "wizSearchWidget_mm.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QApplication>
#include <QTimer>
#include <QLabel>
#include <QPalette>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include "utils/stylehelper.h"

WizSuggestCompletionon::WizSuggestCompletionon(CWizSearchWidget *parent): QObject(parent), m_editor(parent)
{
    m_popupWgt = new CWizSuggestiongContainer;
    m_popupWgt->setWindowFlags(Qt::Popup);
    m_popupWgt->setFocusPolicy(Qt::NoFocus);
    m_popupWgt->setFocusProxy(parent);
    m_popupWgt->setMouseTracking(true);
    m_popupWgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    resetContainerSize(270, 170);

    m_treeWgt = new CWizSuggestiongList;
    m_treeWgt->setFixedHeight(134);
    m_treeWgt->setFixedWidth(270);
    m_treeWgt->setFocusPolicy(Qt::NoFocus);
//    treeWgt->setFocusProxy(popup);
    m_treeWgt->setColumnCount(1);
    m_treeWgt->setUniformRowHeights(true);
    m_treeWgt->setRootIsDecorated(false);
    m_treeWgt->setEditTriggers(QTreeWidget::NoEditTriggers);
    m_treeWgt->setSelectionBehavior(QTreeWidget::SelectRows);
    m_treeWgt->setFrameStyle(QFrame::Box | QFrame::Plain);
    m_treeWgt->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeWgt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    treeWgt->header()->hide();
    m_treeWgt->setMouseTracking(true);
    m_treeWgt->installEventFilter(this);
    m_treeWgt->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_treeWgt->setStyleSheet("QTreeWidget{ border:0px; }  QTreeView::item { padding-left:20px; }");
    m_treeWgt->header()->setStyleSheet("QHeaderView::section{ background-color:#FFFFFF; border:0px; padding-left:8px; color:#C1C1C1; }");
//    m_treeWgt->header()->setMinimumHeight(20);
//    m_treeWgt->header()->setFixedHeight(26);


    QPushButton* button = new QPushButton(m_popupWgt);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFocusProxy(m_popupWgt);
    button->setText(tr("Advanced Search"));
    button->setStyleSheet("QPushButton { background-color:#FFFFFF; border-width: 1px; \
                          padding: 0px 4px; border-style: solid; border-color: #ECECEC; \
                          border-radius: 2px; border-bottom-color:#E0E0E0; }");

    QWidget* buttonContainer = new QWidget(m_popupWgt);
    buttonContainer->setFixedHeight(35);
    buttonContainer->setStyleSheet("background-color:#F7F7F7; border-top:1px solid #E7E7E7;");
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(8, 6, 0, 6);
    buttonLayout->setSpacing(0);
    buttonContainer->setLayout(buttonLayout);
    buttonLayout->addWidget(button);
    buttonLayout->addStretch(0);

    m_infoWgt = new QWidget(m_popupWgt);
    m_infoWgt->setFixedHeight(340);
    QVBoxLayout* infoLayout = new QVBoxLayout(m_infoWgt);
    QLabel* label = new QLabel(m_infoWgt);
    label->setText("Suggestion will be showed here");
    label->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(label);

    QVBoxLayout* layout = new QVBoxLayout(m_popupWgt);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_popupWgt->setLayout(layout);
    layout->addWidget(m_treeWgt);
    layout->addWidget(m_infoWgt);
    layout->addWidget(buttonContainer);

    m_infoWgt->setVisible(false);


    connect(m_treeWgt, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(doneCompletion()));

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(500);
    connect(m_timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(m_editor, SIGNAL(textEdited(QString)), m_timer, SLOT(start()));
}

WizSuggestCompletionon::~WizSuggestCompletionon()
{
    delete m_popupWgt;
}

bool WizSuggestCompletionon::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj != m_treeWgt)
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        m_popupWgt->hide();
        m_editor->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            doneCompletion();
            consumed = true;

        case Qt::Key_Escape:
            m_editor->setFocus();
            m_popupWgt->hide();
            consumed = true;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;

        default:
            m_editor->setFocus();
            m_editor->processEvent(ev);
            m_popupWgt->hide();
            break;
        }

        return consumed;
    }

    return false;
}
//! [4]

//! [5]
void WizSuggestCompletionon::showCompletion(const QStringList &choices)
{

    if (choices.isEmpty())
    {
        m_treeWgt->setVisible(false);
        m_infoWgt->setVisible(true);
    }
    else
    {
        m_infoWgt->setVisible(false);
        m_treeWgt->setVisible(true);
        m_treeWgt->setUpdatesEnabled(false);
        m_treeWgt->clear();
        for (int i = 0; i < choices.count(); ++i)
        {
            QTreeWidgetItem * item;
            item = new QTreeWidgetItem(m_treeWgt);
            item->setText(0, choices[i]);
        }
        int treeWgtHeight = (choices.count() + 1) * 20 + 6;
        m_treeWgt->setFixedHeight(treeWgtHeight);
        m_treeWgt->adjustSize();
        m_treeWgt->setUpdatesEnabled(true);

        resetContainerSize(270, treeWgtHeight + 35);
        }
    //    int h = treeWgt->sizeHintForRow(0) * qMin(7, choices.count()) + 3;
//    popup->resize(treeWgt->width(), h);

    QPoint bottomLeft(280, -5); // = m_editor->geometry().bottomLeft();

    qApp->activeWindow()->mapToGlobal(bottomLeft);
    m_popupWgt->move(qApp->activeWindow()->mapToGlobal(bottomLeft));
    m_popupWgt->setFocus();
//    editor->setFocus();
    m_treeWgt->setFocus();
    m_treeWgt->setCurrentItem(nullptr);
//    popup->setFixedHeight(100);
    m_popupWgt->show();
}

bool WizSuggestCompletionon::isVisible()
{
    return m_popupWgt->isVisible();
}

void WizSuggestCompletionon::hide()
{
    m_popupWgt->hide();
}

void WizSuggestCompletionon::selectSuggestItem(bool up)
{
    QModelIndex curIndex = m_treeWgt->currentIndex();
    int newIndex = curIndex.row() + (up ? -1 : 1);
    if (newIndex < 0)
        newIndex += m_treeWgt->topLevelItemCount();
    else if (newIndex >= m_treeWgt->topLevelItemCount())
        newIndex -= m_treeWgt->topLevelItemCount();

    m_treeWgt->setCurrentItem(m_treeWgt->topLevelItem(newIndex));
}

QString WizSuggestCompletionon::getCurrentText()
{
    if (m_treeWgt->currentItem())
        return m_treeWgt->currentItem()->text(0);

    return QString();
}

void WizSuggestCompletionon::doneCompletion()
{
    m_timer->stop();
    m_popupWgt->hide();
    m_editor->setFocus();
    QTreeWidgetItem *item = m_treeWgt->currentItem();
    if (item) {
        m_editor->setText(item->text(0));
        disconnect(m_editor, SIGNAL(textEdited(QString)), m_timer, SLOT(start()));
        QMetaObject::invokeMethod(m_editor, "doSearch", Q_ARG(QString, item->text(0)));
        connect(m_editor, SIGNAL(textEdited(QString)), m_timer, SLOT(start()));
    }
}

void WizSuggestCompletionon::autoSuggest()
{
    if (!m_editor->isEditing())
        return;

    QStringList choices;

    choices << "choices1" << "choices2" << "choices3";

    showCompletion(choices);
}

void WizSuggestCompletionon::resetContainerSize(int width, int height)
{
    m_popupWgt->setFixedSize(width, height);
    QPainterPath path;
    QRectF rect(0, 0, width, height); //= geometry();
    path.addRoundRect(rect, 5, 5);
    QPolygon polygon= path.toFillPolygon().toPolygon();
    QRegion region(polygon);
    m_popupWgt->setMask(region);
}

void WizSuggestCompletionon::preventSuggest()
{
    m_timer->stop();
}

CWizSuggestiongList::CWizSuggestiongList(QWidget* parent)
    : QTreeWidget(parent)
{
    setMouseTracking(true);
}

void CWizSuggestiongList::mouseMoveEvent(QMouseEvent* event)
{
    QTreeWidget::mouseMoveEvent(event);
    QTreeWidgetItem* item = itemAt(event->pos());
    setCurrentItem(item);
    update();
}



CWizSuggestiongContainer::CWizSuggestiongContainer(QWidget* parent)
    : QWidget(parent)
{

}

//void CWizSuggestiongContainer::showEvent(QShowEvent* ev)
//{
//    clearMask();
//    QWidget::showEvent(ev);

//    QPainterPath path;
//    QRectF rect = geometry();
//    path.addRoundRect(rect, 4, 4);
//    QPolygon polygon= path.toFillPolygon().toPolygon();
//    QRegion region(polygon);
//    setMask(region);
//}

//void CWizSuggestiongContainer::paintEvent(QPaintEvent* ev)
//{
//    //
//    QPainter pt(this);

//    pt.setCompositionMode( QPainter::CompositionMode_Clear );
//    pt.fillRect(rect(), Qt::SolidPattern );

////    QWidget::paintEvent(event);
//}
