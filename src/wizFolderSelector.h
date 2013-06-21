#ifndef WIZFOLDERSELECTOR_H
#define WIZFOLDERSELECTOR_H

#include <QDialog>
#include <QPointer>

class CWizExplorerApp;
class CWizFolderView;

class CWizFolderSelector : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWizFolderSelector(const QString& strTitle, CWizExplorerApp& app, QWidget *parent = 0);

    void setCopyStyle();
    void setAcceptRoot(bool b) { m_bAcceptRoot = b; }
    QString selectedFolder();

protected:
    CWizExplorerApp& m_app;

private:
    QPointer<CWizFolderView> m_folderView;
    bool m_bAcceptRoot;
    bool m_bKeepTime;
    bool m_bKeepTags;

private Q_SLOTS:
    void on_accept();
    void on_checkKeepTime_stateChanged(int state);
    void on_checkKeepTags_stateChanged(int state);
};

#endif // WIZFOLDERSELECTOR_H
