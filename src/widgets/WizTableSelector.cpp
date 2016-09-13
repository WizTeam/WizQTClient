#include "WizTableSelector.h"
#include <QGridLayout>
#include <QMenu>
#include <QApplication>
#include "share/WizThreads.h"
#include <QStyleOption>
#include <QPainter>
#include <QStyle>

WizTableItemWidget::WizTableItemWidget(int row, int col, QWidget* parent)
    : QWidget(parent)
    , m_selected(true)
    , m_row(row)
    , m_col(col)
{
    setFixedSize(QSize(15, 15));
    //
    //setAutoFillBackground(true);
    setSelected(false);
    //
    setMouseTracking(true);
}
//
void WizTableItemWidget::setSelected(bool selected)
{
    if (m_selected == selected)
        return;
    //
    m_selected = selected;
    if (m_selected)
    {
        setStyleSheet("WizTableItemWidget {border: 1px solid #99448aff; background-color:#4c448aff}");
    }
    else
    {
        setStyleSheet("WizTableItemWidget {border: 1px solid #cccccc; background-color:#ffffff}");
    }
}
//
void WizTableItemWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
//
void WizTableItemWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (underMouse())
    {
        emit mouseTracked(m_row, m_col);
    }
    //
    QWidget::mouseMoveEvent(e);
}
void WizTableItemWidget::mousePressEvent(QMouseEvent * e)
{
    bool under = underMouse();
    //
    QWidget::mousePressEvent(e);
    //
    if (under)
    {
        emit mousePressed(m_row + 1, m_col + 1);
    }
}

WizTableSelectorWidget::WizTableSelectorWidget(QWidget* parent)
    : QWidget(parent)
{
    QGridLayout* grid = new QGridLayout();
    setLayout(grid);
    grid->setSpacing(4);
    //
    for (int row = 0; row < ROW_COUNT; row++)
    {
        for (int col = 0; col < COL_COUNT; col++)
        {
            WizTableItemWidget* item = new WizTableItemWidget(row, col, this);
            grid->addWidget(item, row, col);
            //
            connect(item, SIGNAL(mouseTracked(int,int)), this, SLOT(onItemMouseTracked(int,int)));
            connect(item, SIGNAL(mousePressed(int,int)), this, SIGNAL(itemSelected(int,int)));
        }
    }
    //
    setMouseTracking(true);
}

void WizTableSelectorWidget::onItemMouseTracked(int currentRow, int currentCol)
{
    QGridLayout* grid =  dynamic_cast<QGridLayout*>(layout());
    if (!grid)
        return;
    //
    for (int row = 0; row < ROW_COUNT; row++)
    {
        for (int col = 0; col < COL_COUNT; col++)
        {
            QLayoutItem* layoutItem = grid->itemAtPosition(row, col);
            if (!layoutItem)
                continue;
            WizTableItemWidget* item = dynamic_cast<WizTableItemWidget *>(layoutItem->widget());
            if (!item)
                continue;
            //
            bool selected = row <= currentRow && col <= currentCol;
            item->setSelected(selected);
        }
    }
}
