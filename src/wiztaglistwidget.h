#ifndef WIZTAGLISTWIDGET_H
#define WIZTAGLISTWIDGET_H

#include "share/wizpopupwidget.h"
#include "share/wizDatabaseManager.h"

class QLineEdit;
class QListWidget;
class QListWidgetItem;

class CWizTagListWidget : public CWizPopupWidget
{
    Q_OBJECT
public:
    CWizTagListWidget(CWizDatabaseManager& db, QWidget* parent);

    void setDocument(const WIZDOCUMENTDATA& data);

private:
    CWizDatabaseManager& m_dbMgr;
    WIZDOCUMENTDATA m_document;

    QPointer<QLineEdit> m_tagsEdit;
    QPointer<QListWidget> m_list;

    void updateTagsText();

public Q_SLOTS:
    void on_list_itemClicked(QListWidgetItem* item);
    void on_tagsEdit_editingFinished();
};

#endif // WIZTAGLISTWIDGET_H
