#ifndef WIZDOCUMENTTRANSITIONVIEW_H
#define WIZDOCUMENTTRANSITIONVIEW_H

#include <QWidget>

class QLabel;
class CWizAnimateAction;
class QToolButton;

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
    void showAsMode(const QString& strObjGUID,TransitionMode mode);

public slots:
    void onDownloadProgressChanged(QString strObjGUID, int ntotal, int nloaded);

protected:
    void hideEvent(QHideEvent* ev);

private:
    QLabel* m_labelHint;
    int m_mode;
    QString m_objGUID;
    CWizAnimateAction* m_animation;
    QToolButton* m_toolButton;
};

#endif // WIZDOCUMENTTRANSITIONVIEW_H
