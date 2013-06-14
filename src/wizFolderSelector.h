#ifndef WIZFOLDERSELECTOR_H
#define WIZFOLDERSELECTOR_H

#include <QDialog>

namespace Ui {
class CWizFolderSelector;
}

class CWizFolderSelector : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWizFolderSelector(QWidget *parent = 0);
    ~CWizFolderSelector();

    virtual void showEvent(QShowEvent* event);

    QString folder() { return m_strFolder; }

protected:
    void initFolders();
    
private:
    Ui::CWizFolderSelector *ui;
    QString m_strFolder;
};

#endif // WIZFOLDERSELECTOR_H
