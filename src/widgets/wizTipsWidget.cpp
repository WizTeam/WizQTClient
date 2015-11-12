#include "wizTipsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

#include "wizPositionDelegate.h"

static void emptyFunction()
{
    qDebug() << "Hello World!";
}

CWizTipsWidget::CWizTipsWidget(QWidget *parent)
    : CWizPopupWidget(parent)
    , m_hintSize(318, 82)
    , m_function(emptyFunction)
{
    Qt::WindowFlags flags = windowFlags();
    flags = flags & ~Qt::Popup;
    flags = flags | Qt::WindowStaysOnTopHint | Qt::Tool;
    setWindowFlags(flags);
    setContentsMargins(0,0,0,0);
    setMaximumWidth(318);
    setLeftAlign(true);

    QWidget* containerWidget = this;//new QWidget(this);
    QPalette pal = containerWidget->palette();
    pal.setBrush(QPalette::Window, QBrush(QColor("#F6F6F6")));
    containerWidget->setPalette(pal);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(16, 16, 16, 0);
    vLayout->setSpacing(6);
    containerWidget->setLayout(vLayout);
//    layout()->addWidget(containerWidget);

    m_labelTitle = new QLabel(this);
    m_labelInfo = new QLabel(this);
    m_btnOK = new QPushButton(this);

    vLayout->addWidget(m_labelTitle);
    vLayout->addWidget(m_labelInfo);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addStretch();
    hLayout->addWidget(m_btnOK);
    vLayout->addLayout(hLayout);

    m_labelTitle->setStyleSheet("color:#000000; font-size:14px;");
    m_labelInfo->setStyleSheet("color:#888888; font-size:12px;");
    m_labelInfo->setWordWrap(true);
    m_labelInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_btnOK->setStyleSheet("color:#3177EE; font-size:12px; line-height:18px; margin-top:8px; margin-bottom:8px; border:0px; background:none;");

    setSizeHint(m_hintSize);
}

void CWizTipsWidget::setSizeHint(const QSize& hintSize)
{
    m_hintSize = hintSize;

    int triangleMargin = (hintSize.width() - m_triangleWidth) / 2;
    setTriangleStyle(triangleMargin, m_triangleWidth, m_triangleHeight);
}

QSize CWizTipsWidget::sizeHint() const
{
    return m_hintSize;
}

void CWizTipsWidget::setText(const QString& title, const QString& info, const QString& buttonText)
{
    m_labelTitle->setText(title);
    m_labelInfo->setText(info);
    m_btnOK->setText(buttonText);
}

void CWizTipsWidget::setButtonVisible(bool visible)
{
    m_btnOK->setVisible(visible);
}

void CWizTipsWidget::addToTipListManager(QWidget* targetWidget, int nXOff, int nYOff)
{
    CWizTipListManager* manager = CWizTipListManager::instance();
    manager->addTipsWidget(this, targetWidget, nXOff, nYOff);

    if (manager->firstTipWidget() == this)
    {
        manager->displayCurrentTipWidget();
    }
}

void CWizTipsWidget::bindFunction(const std::function<void ()>& f)
{
    m_function = f;
}

void CWizTipsWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    QWidget::mouseReleaseEvent(ev);

    close();

    m_function();

    CWizPositionDelegate& delegate = CWizPositionDelegate::instance();
    delegate.removeListener(this);

    CWizTipListManager* manager = CWizTipListManager::instance();
    manager->displayNextTipWidget();
}

void CWizTipsWidget::showEvent(QShowEvent* ev)
{
    QWidget::showEvent(ev);

    CWizPositionDelegate& delegate = CWizPositionDelegate::instance();
    delegate.addListener(this);
}

CWizTipListManager* CWizTipListManager::m_instance = nullptr;

CWizTipListManager* CWizTipListManager::instance()
{
    if (m_instance)
        return m_instance;

    m_instance = new CWizTipListManager();
    return m_instance;
}

void CWizTipListManager::addTipsWidget(CWizTipsWidget* widget, QWidget* targetWidget, int nXOff, int nYOff)
{
    TipItem item;
    item.widget = widget;
    item.targetWidget = targetWidget;
    item.nXOff = nXOff;
    item.nYOff = nYOff;

    m_tips.append(item);
}

CWizTipsWidget* CWizTipListManager::firstTipWidget()
{
    if (m_tips.isEmpty())
        return nullptr;

    return m_tips.first().widget;
}

void CWizTipListManager::displayNextTipWidget()
{
    if (m_tips.isEmpty())
    {
        deleteManager();
        return;
    }
    m_tips.removeFirst();

    displayCurrentTipWidget();
}

void CWizTipListManager::displayCurrentTipWidget()
{
    if (m_tips.isEmpty())
    {
        deleteManager();
        return;
    }

    TipItem item = m_tips.first();
    QRect rc = item.targetWidget->rect();
    QPoint pt = item.targetWidget->mapToGlobal(QPoint(rc.width()/2 + item.nXOff, rc.height() + item.nYOff));
    item.widget->showAtPoint(pt);

    m_timer.start(500);
}

void CWizTipListManager::on_timerOut()
{
    if (m_tips.isEmpty())
    {
        m_timer.stop();
        deleteManager();
        return;
    }
    else if (!m_tips.first().targetWidget->isVisible())
    {
        m_timer.stop();
        m_tips.first().widget->hide();
        for (TipItem item : m_tips)
        {
            item.widget->deleteLater();
        }
        m_tips.clear();

        deleteManager();
        return;
    }
}

CWizTipListManager::CWizTipListManager(QObject* parent)
    : QObject(parent)
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));
}

void CWizTipListManager::deleteManager()
{
    m_instance->deleteLater();
    m_instance = nullptr;
}
