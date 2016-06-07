#include "wizTableSelector.h"
#include <QGridLayout>

WizTableItemWidget::WizTableItemWidget(int row, int col, QWidget* parent)
    : QWidget(parent)
    , m_selected(true)
    , m_row(row)
    , m_col(col)
{
    setFixedSize(QSize(16, 16));
    //
    setAutoFillBackground(true);
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
        setPalette(QPalette(QColor(0xff, 0, 0)));
    }
    else
    {
        setPalette(QPalette(QColor(0, 0xff, 0)));
    }
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
    if (underMouse())
    {
        emit mousePressed(m_row, m_col);
    }
    //
    QWidget::mousePressEvent(e);
}

WizTableSelectorWidget::WizTableSelectorWidget(QWidget* parent)
    : QWidget(parent)
{
    QGridLayout* grid = new QGridLayout();
    setLayout(grid);
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
