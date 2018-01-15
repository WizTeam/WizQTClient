#ifndef WizCombineNotesDialog_H
#define WizCombineNotesDialog_H

#include <QDialog>
#include <QLineEdit>

#include "share/WizObject.h"
#include <QListWidget>

namespace Ui {
    class WizCombineNotesDialog;
}

class QTimer;
class WizDatabaseManager;

class WizCombineNotesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizCombineNotesDialog(WizDatabaseManager& dbMgr, const CWizDocumentDataArray& documents, QWidget *parent = 0);
    ~WizCombineNotesDialog();

    CWizDocumentDataArray m_documents;
    QTimer* m_timer;
    WizDatabaseManager& m_dbMgr;
private:
    CWizDocumentDataArray getResultDocuments();
public:
    virtual void accept();
signals:

private slots:
    void checkButtonStatusTimeout();
    void currentRowChanged(int currentRow);
    void preview();
    void moveUp();
    void moveDown();

private:
    Ui::WizCombineNotesDialog *ui;
};

#endif // WizCombineNotesDialog_H
