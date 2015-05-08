#include "wizTagBar.h"
#include "utils/stylehelper.h"

using namespace Core::Internal;

CWizTagBar::CWizTagBar(QWidget *parent) : QWidget(parent)
{
    int nHeight = Utils::StyleHelper::tagBarHeight();
    setFixedHeight(nHeight);

    setStyleSheet("font-size: 11px; color: #646464;");
    setContentsMargins(5, 0, 0, 0);
    setFocusPolicy(Qt::StrongFocus);
}

CWizTagBar::~CWizTagBar()
{

}



CTagItem::CTagItem(QWidget* parent) : QWidget(parent)
{

}

CTagItem::~CTagItem()
{

}

void CTagItem::paintEvent(QPaintEvent* event)
{

}

void CTagItem::focusInEvent(QFocusEvent* event)
{
    m_focused = true;
    QWidget::focusInEvent(event);
}

void CTagItem::focusOutEvent(QFocusEvent* event)
{
    m_focused = false;
    QWidget::focusOutEvent(event);
}

void CTagItem::mousePressEvent(QMouseEvent* event)
{
    if (m_focused)
    {

    }
    QWidget::mousePressEvent(event);
}
