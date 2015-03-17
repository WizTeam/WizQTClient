#ifndef CORE_INOTEVIEW_H
#define CORE_INOTEVIEW_H

#include <QWidget>

class QWebEnginePage;
class QWebFrame;

namespace Core {

class INoteView : public QWidget
{
    Q_OBJECT

public:
    explicit INoteView(QWidget *parent = 0);
#ifdef USEWEBENGINE
    virtual QWebEnginePage* notePage() = 0;
#else
    virtual QWebFrame* noteFrame() = 0;
#endif
    virtual bool isEditing() const = 0;
};

} // namespace Core

#endif // CORE_INOTEVIEW_H
