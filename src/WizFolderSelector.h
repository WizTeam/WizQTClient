#ifndef WIZFOLDERSELECTOR_H
#define WIZFOLDERSELECTOR_H

#include <QDialog>
#include <QPointer>

class WizExplorerApp;
class WizFolderView;
struct WIZTAGDATA;

class WizFolderSelector : public QDialog
{
    Q_OBJECT
    
public:
    explicit WizFolderSelector(const QString& strTitle, WizExplorerApp& app, unsigned int nPermission,QWidget *parent = 0);

    void setCopyStyle(bool showKeepTagsOption);
    void setAcceptRoot(bool b) { m_bAcceptRoot = b; }
    //
    bool isKeepTime() const;
    bool isKeepTag() const;
    //
    bool isSelectPersonalFolder();
    QString selectedFolder();
    bool isSelectGroupFolder();
    WIZTAGDATA selectedGroupFolder();

protected:
    WizExplorerApp& m_app;

private:
    QPointer<WizFolderView> m_folderView;
    bool m_bAcceptRoot;
    bool m_bKeepTime;
    bool m_bKeepTags;
    unsigned int m_nMinPermission;

private Q_SLOTS:
    void on_accept();
    void on_checkKeepTime_stateChanged(int state);
    void on_checkKeepTags_stateChanged(int state);
};

#endif // WIZFOLDERSELECTOR_H
