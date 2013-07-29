#ifndef WIZDOCUMENTTRANSITIONVIEW_H
#define WIZDOCUMENTTRANSITIONVIEW_H

#include <QWidget>

class QLabel;

class CWizDocumentTransitionView : public QWidget
{
    Q_OBJECT

public:
    enum TransitionMode {
        Downloading,
        Loading,
        Saving,
        ErrorOccured
    };

    explicit CWizDocumentTransitionView(QWidget *parent = 0);
    void showAsMode(TransitionMode mode);

private:
    QLabel* m_labelHint;
};

#endif // WIZDOCUMENTTRANSITIONVIEW_H
