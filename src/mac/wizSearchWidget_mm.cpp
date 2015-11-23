#include "wizSearchWidget_mm.h"

#ifdef USECOCOATOOLBAR

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
#include <QStyledItemDelegate>
#include <QPainter>
#include <QtConcurrent>
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "utils/stylehelper.h"
#include "share/wizsettings.h"
#include "wizPositionDelegate.h"


class WizSuggestionItemDelegate :  public QStyledItemDelegate
{
public:
    WizSuggestionItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent)
    {}

    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        return QSize(QStyledItemDelegate::sizeHint(option, index).width(), 20);
    }

    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);


        painter->save();
        painter->setPen(Qt::black);
        if ((option.state & QStyle::State_Selected))
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor("#5990EF"));
            painter->fillRect(option.rect, painter->brush());
            painter->setPen(Qt::white);
        }

        QRect rcText = opt.rect.adjusted(20, 0, -20, 0);
        QFontMetrics fm(painter->font());
        QString text = fm.elidedText(opt.text, Qt::ElideRight, rcText.width());
        painter->drawText(rcText, Qt::AlignLeft | Qt::AlignVCenter, text);
        painter->restore();
    }
};



WizSuggestCompletionon::WizSuggestCompletionon(CWizSearchWidget *parent)
    : QObject(parent)
    , m_editor(parent)
    , m_popupWgtWidth(WizIsHighPixel() ? HIGHPIXSEARCHWIDGETWIDTH : NORMALSEARCHWIDGETWIDTH)
    , m_usable(true)
{
    m_popupWgt = new QWidget;
    m_popupWgt->setWindowFlags(Qt::Popup);
    m_popupWgt->setFocusPolicy(Qt::NoFocus);
    m_popupWgt->setFocusProxy(parent);
    m_popupWgt->setMouseTracking(true);
    m_popupWgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    resetContainerSize(m_popupWgtWidth, 170);

    m_treeWgt = new CWizSuggestiongList;
    m_treeWgt->setFixedHeight(134);
    m_treeWgt->setFixedWidth(m_popupWgtWidth);
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
    m_treeWgt->setStyleSheet("QTreeWidget{ border:0px; outline:0; }  "
                             "QTreeView::item { color:#AAAAAA; border-left:20px solid #FFFFFF;  margin-left:20px; }");
    m_treeWgt->header()->setStyleSheet("QHeaderView::section{ background-color:#FFFFFF; height:20px; "
                                       "border:0px; padding-left:8px; color:#C1C1C1; font-size:12px; }");
//    m_treeWgt->header()->setMinimumHeight(20);
//    m_treeWgt->header()->setFixedHeight(26);
    WizSuggestionItemDelegate* suggestionDelegate = new WizSuggestionItemDelegate(m_treeWgt);
    m_treeWgt->setItemDelegate(suggestionDelegate);


    QPushButton* button = new QPushButton(m_popupWgt);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFocusProxy(m_popupWgt);
    button->setText(tr("Advanced Search"));
    button->setStyleSheet("QPushButton { background-color:#FFFFFF; border-width: 1px; \
                          padding: 0px 4px; border-style: solid; border-color: #ECECEC; \
                          border-radius: 2px; border-bottom-color:#E0E0E0; }");

    connect(button, SIGNAL(clicked(bool)), parent, SLOT(on_advanced_buttonClicked()));

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
    m_infoWgt->setFixedHeight(135);
    m_infoWgt->setStyleSheet("background-color:#FFFFFF;");
    QVBoxLayout* infoLayout = new QVBoxLayout(m_infoWgt);
    QLabel* label = new QLabel(m_infoWgt);
    label->setText(tr("Suggestion will be showed here"));
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
    m_timer->setInterval(200);
    connect(m_timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(m_editor, SIGNAL(textEdited(QString)), m_timer, SLOT(start()));

    CWizPositionDelegate& delegate = CWizPositionDelegate::instance();
    delegate.addListener(m_popupWgt);
}

WizSuggestCompletionon::~WizSuggestCompletionon()
{
    delete m_popupWgt;
}

void WizSuggestCompletionon::setUserSettings(CWizUserSettings* settings)
{
    m_settings = settings;
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

void WizSuggestCompletionon::setUsable(bool usable)
{
    m_usable = usable;
}
//! [4]

//! [5]
void WizSuggestCompletionon::showCompletion(const QStringList &choices, bool isRecentSearches)
{

    if (choices.isEmpty())
    {
        m_treeWgt->setVisible(false);
        m_infoWgt->setVisible(true);
        m_popupWgt->setFixedHeight(170);
    }
    else
    {
        m_infoWgt->setVisible(false);
        m_treeWgt->setVisible(true);
        m_treeWgt->setUpdatesEnabled(false);
        m_treeWgt->clear();

        m_treeWgt->setHeaderLabel(isRecentSearches? tr("Recent Searches") : tr("Suggestions"));

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

        m_treeWgt->setFixedWidth(m_popupWgtWidth);
        resetContainerSize(m_popupWgtWidth, treeWgtHeight + 35);
    }

    QPoint bottomLeft(m_popupOffset.width(), -10); // = m_editor->geometry().bottomLeft();

    if (qApp->activeWindow() == nullptr)
        return;

    m_popupWgt->move(qApp->activeWindow()->mapToGlobal(bottomLeft));
    m_popupWgt->setFocus();
    m_treeWgt->setFocus();
    m_treeWgt->setCurrentItem(nullptr);
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

void WizSuggestCompletionon::setPopupOffset(int popupWgtWidth, const QSize& offset)
{
    m_popupWgtWidth = popupWgtWidth;
    m_popupOffset = offset;
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
        QMetaObject::invokeMethod(m_editor, "on_search_editFinished", Q_ARG(QString, item->text(0)));
        connect(m_editor, SIGNAL(textEdited(QString)), m_timer, SLOT(start()));
    }
}

void searchTitleFromDB(CWizDatabase& db, const QString& title, QStringList& suggestions)
{
    CWizStdStringArray arrayTitle;
    if (db.GetDocumentTitleStartWith(title, 5, arrayTitle))
    {
        for (CString title : arrayTitle)
        {
            if (suggestions.count() >= 5)
                break;

            suggestions.append(title);
        }
    }
}

void WizSuggestCompletionon::autoSuggest()
{
    if (!m_editor->isEditing() || !m_usable)
        return;

    QString inputedText = m_editor->currentText();

    if (inputedText.isEmpty())
    {
        QStringList recentSearches = m_settings->getRecentSearches(true);
        m_treeWgt->setHeaderLabel(tr("Recent Searches"));
        showCompletion(recentSearches, true);
    }
    else
    {
        QtConcurrent::run([this, inputedText](){
            QStringList suggestions;
            CWizDatabaseManager* manager = CWizDatabaseManager::instance();

            CWizDatabase& db = manager->db();
            searchTitleFromDB(db, inputedText, suggestions);

            for (int i = 0; i < manager->count(); i++)
            {
                if (suggestions.count() >= 5)
                    break;

                CWizDatabase& db = manager->at(i);
                searchTitleFromDB(db, inputedText, suggestions);
            }
            QMetaObject::invokeMethod(this, "showCompletion", Qt::QueuedConnection,
                                      Q_ARG(QStringList, suggestions), Q_ARG(bool, false));
        });
    }
}

void WizSuggestCompletionon::resetContainerSize(int width, int height)
{
    m_popupWgt->setFixedSize(m_editor->sizeHint().width(), height);
    QRect rect(0, 0, m_editor->sizeHint().width(), height); //= geometry();    
    QRegion region = Utils::StyleHelper::borderRadiusRegion(rect);

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

void CWizSuggestiongList::leaveEvent(QEvent* ev)
{
    QTreeWidget::leaveEvent(ev);
    setCurrentItem(nullptr);
    update();
}

#endif
