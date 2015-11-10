#include "wizTipsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "wizPositionDelegate.h"

CWizTipsWidget::CWizTipsWidget(QWidget *parent)
    : CWizPopupWidget(parent)
    , m_hintSize(308, 82)
{
    Qt::WindowFlags flags = windowFlags();
    flags = flags & ~Qt::Popup;
    flags = flags | Qt::WindowStaysOnTopHint | Qt::Tool;
    setWindowFlags(flags);
    setContentsMargins(0,0,0,0);
    setMaximumWidth(308);
    setLeftAlign(true);

    QWidget* containerWidget = this;//new QWidget(this);
    QPalette pal = containerWidget->palette();
    pal.setBrush(QPalette::Window, QBrush(QColor("#fbfbfb")));
    containerWidget->setPalette(pal);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(12, 16, 12, 0);
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

    m_labelTitle->setStyleSheet("color:#000000; font-size:12px;");
    m_labelInfo->setStyleSheet("color:#888888; font-size:12px;");
    m_labelInfo->setWordWrap(true);
    m_labelInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_btnOK->setStyleSheet("color:#3177EE; font-size:12px; margin-top:8px; margin-bottom:8px; border:0px; background:none;");

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
        QRect rc = targetWidget->rect();
        QPoint pt = targetWidget->mapToGlobal(QPoint(rc.width()/2 + nXOff, rc.height() + nYOff));
        showAtPoint(pt);
    }
}

void CWizTipsWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    QWidget::mouseReleaseEvent(ev);

    close();

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
        return;
    m_tips.removeFirst();

    if (m_tips.isEmpty())
    {
        m_instance->deleteLater();
        m_instance = nullptr;
        return;
    }

    TipItem item = m_tips.first();
    QRect rc = item.targetWidget->rect();
    QPoint pt = item.targetWidget->mapToGlobal(QPoint(rc.width()/2 + item.nXOff, rc.height() + item.nYOff));
    item.widget->showAtPoint(pt);
}

CWizTipListManager::CWizTipListManager(QObject* parent)
    : QObject(parent)
{
}
