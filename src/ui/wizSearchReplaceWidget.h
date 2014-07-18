#ifndef WIZSEARCHREPLACEWIDGET_H
#define WIZSEARCHREPLACEWIDGET_H

#include <QWidget>

namespace Ui {
class CWizSearchReplaceWidget;
}

class CWizSearchReplaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CWizSearchReplaceWidget(QWidget *parent = 0);
    ~CWizSearchReplaceWidget();

private:
    Ui::CWizSearchReplaceWidget *ui;
};

#endif // WIZSEARCHREPLACEWIDGET_H
