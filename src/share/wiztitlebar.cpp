#include "wiztitlebar.h"

#include <QPixmap>
#include <QStyle>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>



CWizTitleBar::CWizTitleBar(QWidget *parent, QWidget* window)
    : QWidget(parent)
    , m_window(window)
    , m_oldContentsMargin(10, 10, 10, 10)
{
    // 不继承父组件的背景色
    setAutoFillBackground(true);
    // 使用 Highlight 作为背景色
    //setBackgroundRole(QPalette::Highlight);

    minimize = new QToolButton(this);
    maximize = new QToolButton(this);
    close= new QToolButton(this);

    // 设置按钮图像的样式
    QPixmap pix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
    close->setIcon(pix);

    maxPix = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);
    maximize->setIcon(maxPix);

    pix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
    minimize->setIcon(pix);

    restorePix = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);

    minimize->setMinimumHeight(20);
    close->setMinimumHeight(20);
    maximize->setMinimumHeight(20);

    QLabel *label = new QLabel(this);
    label->setText("Window Title");
    m_window->setWindowTitle("Window Title");

    QHBoxLayout *hbox = new QHBoxLayout(this);

    hbox->addWidget(label);
    hbox->addWidget(minimize);
    hbox->addWidget(maximize);
    hbox->addWidget(close);

    hbox->insertStretch(1, 500);
    hbox->setSpacing(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    maxNormal = false;

    connect(close, SIGNAL( clicked() ), m_window, SLOT(close() ) );
    connect(minimize, SIGNAL( clicked() ), this, SLOT(showSmall() ) );
    connect(maximize, SIGNAL( clicked() ), this, SLOT(showMaxRestore() ) );
}

void CWizTitleBar::showSmall()
{
    m_window->showMinimized();
}

void CWizTitleBar::showMaxRestore()
{
    if (maxNormal) {
        //
        m_window->setContentsMargins(m_oldContentsMargin);
        m_window->showNormal();
        maxNormal = !maxNormal;
        maximize->setIcon(maxPix);
        //
    } else {
        //
        m_oldContentsMargin = m_window->contentsMargins();
        m_window->setContentsMargins(0, 0, 0, 0);
        m_window->showMaximized();
        maxNormal = !maxNormal;
        maximize->setIcon(restorePix);
    }
}

void CWizTitleBar::mousePressEvent(QMouseEvent *me)
{
    startPos = me->globalPos();
    clickPos = mapTo(m_window, me->pos());
}
void CWizTitleBar::mouseMoveEvent(QMouseEvent *me)
{
    if (maxNormal)
        return;
    m_window->move(me->globalPos() - clickPos);
}
