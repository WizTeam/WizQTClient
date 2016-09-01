#include "wizTipsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

static void emptyFunction()
{
}

QSet<QString> CWizTipsWidget::m_tipsList = QSet<QString>();

CWizTipsWidget::CWizTipsWidget(const QString& id, QWidget *parent)
    : CWizPopupWidget(parent)
    , m_hintSize(318, 82)
    , m_showFunction(emptyFunction)
    , m_hideFunction(emptyFunction)
    , m_closeFunction(emptyFunction)
    , m_id(id)
    , m_autoAdjustPosition(false)
    , m_targetWidget(nullptr)
    , m_xOff(0)
    , m_yOff(0)
{
    Qt::WindowFlags flags = windowFlags();
    flags = flags & ~Qt::Popup;
    flags = flags | Qt::WindowStaysOnTopHint | Qt::Tool;
    setWindowFlags(flags);
    setContentsMargins(0,0,0,0);
    setMaximumWidth(318);
    setLeftAlign(true);

    m_tipsList.insert(m_id);

    QWidget* containerWidget = this;//new QWidget(this);
    QPalette pal = containerWidget->palette();
    pal.setBrush(QPalette::Window, QBrush(QColor("#F6F6F6")));
    containerWidget->setPalette(pal);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(16, 16, 16, 0);
    vLayout->setSpacing(6);
    containerWidget->setLayout(vLayout);

    m_labelTitle = new QLabel(this);
    m_labelInfo = new QLabel(this);
    m_btnOK = new QPushButton(this);

    vLayout->addWidget(m_labelTitle);
    vLayout->addWidget(m_labelInfo);

    QHBoxLayout* hLayout = new QHBoxLayout();
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

    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));

    setSizeHint(m_hintSize);
}

CWizTipsWidget::~CWizTipsWidget()
{
    m_tipsList.remove(m_id);
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

bool CWizTipsWidget::isTipsExists(const QString& id)
{
    return m_tipsList.contains(id);
}

void CWizTipsWidget::setAutoAdjustPosition(bool autoAdjust)
{
    m_autoAdjustPosition = autoAdjust;
}

bool CWizTipsWidget::isAutoAdjustPosition() const
{
    return m_autoAdjustPosition;
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

bool CWizTipsWidget::bindTargetWidget(QWidget* targetWidget, int nXOff, int nYOff)
{
    if (!targetWidget)
        return false;

    m_targetWidget = targetWidget;
    m_xOff = nXOff;
    m_yOff = nYOff;

    m_autoAdjustPosition = true;

    return true;
}

void CWizTipsWidget::bindShowFunction(const std::function<void ()>& f)
{
    m_showFunction = f;
}

void CWizTipsWidget::bindHideFunction(const std::function<void ()>& f)
{
    m_hideFunction = f;
}

void CWizTipsWidget::bindCloseFunction(const std::function<void ()>& f)
{
    m_closeFunction = f;
}

void CWizTipsWidget::hide()
{
    m_hideFunction();

    QWidget::hide();

    deleteLater();
}

void CWizTipsWidget::on_targetWidgetClicked()
{
    closeTip();
}

void CWizTipsWidget::on_timerOut()
{
    if (m_targetWidget->isVisible())
    {
        on_showRequest();
    }
    else if (isVisible())
    {
        hide();
        m_timer.stop();
    }
}

void CWizTipsWidget::on_showRequest()
{
    if (!m_targetWidget->isVisible())
        return;

    QRect rc = m_targetWidget->rect();
    QPoint pt = m_targetWidget->mapToGlobal(QPoint(rc.width()/2 + m_xOff, rc.height() + m_yOff));
    showAtPoint(pt);
}

void CWizTipsWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    m_timer.stop();
    QWidget::mouseReleaseEvent(ev);
    closeTip();
}

void CWizTipsWidget::showEvent(QShowEvent* ev)
{
    m_showFunction();

    QWidget::showEvent(ev);

    if (m_autoAdjustPosition)
    {
        m_timer.start(500);
    }
}

void CWizTipsWidget::closeTip()
{
    m_closeFunction();

    close();

    emit finished();
}

