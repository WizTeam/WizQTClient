#include "WizMultiLineListWidget.h"

#include <QStyledItemDelegate>
#include "share/WizQtHelper.h"



class CWizMultiLineListWidgetDelegate : public QStyledItemDelegate
{
    int m_lineCount;
public:
    CWizMultiLineListWidgetDelegate(int lineCount, QWidget*parent)
        : QStyledItemDelegate(parent)
        , m_lineCount(lineCount)
    {
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
    {
        QSize sz = QStyledItemDelegate::sizeHint(option, index);
        //
        sz.setHeight((option.fontMetrics.height() + WizSmartScaleUI(2)) * m_lineCount + WizSmartScaleUI(8));
        return sz;
    }
    //
    int lineCount() const
    {
        return m_lineCount;
    }
};


WizMultiLineListWidget::WizMultiLineListWidget(int lineCount, QWidget* parent)
    : QListWidget(parent)
{
    setItemDelegate(new CWizMultiLineListWidgetDelegate(lineCount, this));
}

int WizMultiLineListWidget::wrapTextLineIndex() const
{
    return 1;
}

bool WizMultiLineListWidget::imageAlignLeft() const
{
    return true;
}
int WizMultiLineListWidget::imageWidth() const
{
    return WizSmartScaleUI(32);
}

QString WizMultiLineListWidget::itemText(const QModelIndex& index, int line) const
{
    if (line == 0)
    {
        return itemFromIndex(index)->text();
    }
    //
    return QString();
}

QColor WizMultiLineListWidget::itemTextColor(const QModelIndex& index, int line, bool selected, QColor defColor) const
{
    Q_UNUSED(index);
    Q_UNUSED(line);
    Q_UNUSED(selected);
    //
    return defColor;
}

QPixmap WizMultiLineListWidget::itemImage(const QModelIndex& index) const
{
    Q_UNUSED(index);
    //
    return QPixmap();
}

bool WizMultiLineListWidget::itemExtraImage(const QModelIndex& index, const QRect& itemBound,
                                              QRect& rcImage, QPixmap& extraPix) const
{
    Q_UNUSED(index);
    Q_UNUSED(itemBound);
    Q_UNUSED(rcImage);
    Q_UNUSED(extraPix);

    return false;
}

int WizMultiLineListWidget::lineCount() const
{
    if (const CWizMultiLineListWidgetDelegate* delegate = dynamic_cast<const CWizMultiLineListWidgetDelegate*>(itemDelegate()))
    {
        return delegate->lineCount();
    }
    return 2;
}

