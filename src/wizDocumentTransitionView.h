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

public slots:
    void onDownloadProgressChanged(QString strObjGUID, int ntotal, int nloaded);

private:
    QLabel* m_labelHint;
    int m_mode;
};

#endif // WIZDOCUMENTTRANSITIONVIEW_H
