#ifndef WIZNOTESTYLE_H
#define WIZNOTESTYLE_H

#include <QString>
#include <QStyle>
#include <QProxyStyle>
#include <QListWidget>
#include <QListWidgetItem>

QStyle* WizGetStyle(const QString& skinName);

QStyle* WizGetImageButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                               const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                               const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor);



template <typename T>
class CWizListItemStyle : public QProxyStyle
{
public:
    CWizListItemStyle(QStyle *style = 0) : QProxyStyle(style) {}
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        switch (element)
        {
        case CE_ItemViewItem:
            {
                const QStyleOptionViewItemV4 *vopt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(option);
                Q_ASSERT(vopt);

                if (const QListWidget* view = dynamic_cast<const QListWidget*>(widget))
                {
                    QListWidgetItem *item = view->item(vopt->index.row());
                    if (item)
                    {
                        if (T* templateItem = dynamic_cast<T*>(item))
                        {
                            templateItem->draw(painter, vopt);
                        }
                    }
                }
                break;
            }
        default:
            QProxyStyle::drawControl(element, option, painter, widget);
            break;
        }
    }
};

//class CWizAppStyle : public QProxyStyle
//{

//};

#endif // WIZNOTESTYLE_H
