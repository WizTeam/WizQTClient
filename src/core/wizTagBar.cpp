#include "wizTagBar.h"
#include <QPalette>
#include <QFontMetrics>
#include "utils/stylehelper.h"

using namespace Core::Internal;

CWizTagBar::CWizTagBar(QWidget *parent) : QWidget(parent)
{
    int nHeight = Utils::StyleHelper::tagBarHeight();
    setFixedHeight(nHeight);

    setStyleSheet("font-size: 11px; color: #646464;");
    setContentsMargins(5, 0, 0, 0);
    setFocusPolicy(Qt::StrongFocus);

    QPalette pl = palette();
    pl.setBrush(QPalette::Window, QBrush(QColor("#f7f8f9")));
    setPalette(pl);
}

CWizTagBar::~CWizTagBar()
{

}



CTagItem::CTagItem(const QString text, QWidget* parent)
    : QWidget(parent)
    , m_text(text)
{

}

CTagItem::~CTagItem()
{

}

QSize CTagItem::sizeHint() const
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
