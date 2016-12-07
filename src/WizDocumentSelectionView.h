#ifndef WIZDOCUMENTSELECTIONVIEW_H
#define WIZDOCUMENTSELECTIONVIEW_H

#include <QWidget>

#include "share/WizObject.h"

class QGraphicsView;

class WizExplorerApp;
class WizDatabaseManager;

class WizDocumentSelectionView : public QWidget
{
    Q_OBJECT

public:
    explicit WizDocumentSelectionView(WizExplorerApp& app, QWidget *parent = 0);
    void requestDocuments(const CWizDocumentDataArray& arrayDocument);

protected:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QGraphicsView* m_graphicsView;

    virtual void showEvent(QShowEvent* event);

private:
    CWizDocumentDataArray m_docs;

    QPoint getThumbPosition(const WIZABSTRACT& abs);

    // generate document thumb and return
    QPixmap getDocumentPixmap(const WIZDOCUMENTDATA& doc,
                              const WIZABSTRACT& abs);

    bool isThumbNeedBuild(const WIZABSTRACT& abs);

private Q_SLOTS:
    void on_thumbCache_loaded(const WIZABSTRACT& abs);
    
};

#endif // WIZDOCUMENTSELECTIONVIEW_H
