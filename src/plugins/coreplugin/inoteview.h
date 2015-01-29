#ifndef CORE_INOTEVIEW_H
#define CORE_INOTEVIEW_H

#include <QWidget>

class QWebEnginePage;

namespace Core {

class INoteView : public QWidget
{
    Q_OBJECT

public:
    explicit INoteView(QWidget *parent = 0);
    virtual QWebEnginePage* notePage() = 0;
    virtual bool isEditing() const = 0;
};

} // namespace Core

#endif // CORE_INOTEVIEW_H
