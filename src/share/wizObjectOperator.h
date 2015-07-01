#ifndef CWIZOBJECTOPERATOR_H
#define CWIZOBJECTOPERATOR_H

#include <QObject>
#include "wizobject.h"

class CWizProgressDialog;
class CWizDatabaseManager;
class CWizObjectDataDownloaderHost;

class CWizDocumentOperator : public QObject
{
    Q_OBJECT
public:
    CWizDocumentOperator(CWizDatabaseManager& dbMgr, QObject* parent = 0);
    ~CWizDocumentOperator();

    /**
      IMPORTANT!!! :   All of these copy or move functions could only call once, and the operator object will destroy automatic
     */
    void copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                     bool keepDocTime, bool keepTag, CWizObjectDataDownloaderHost* downloader);
    void copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                     bool keepDocTime, CWizObjectDataDownloaderHost* downloader);
    void moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                      CWizObjectDataDownloaderHost* downloader);
    void moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                      CWizObjectDataDownloaderHost* downloader);
    //
    void copyFolderToPersonalDB();




    void bindSignalsToProgressDialog(CWizProgressDialog* progress);

public slots:
    void stop();

signals:
    void progress(int max, int value);
    void newAction(const QString& action);
    void finished();

private slots:
    void copyDocumentToPersonalFolder();
    void copyDocumentToGroupFolder();
    void moveDocumentToPersonalFolder();
    void moveDocumentToGroupFolder();

private:
    void copyDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc);
    void copyDocumentToGroupFolder(const WIZDOCUMENTDATA& doc);
    void moveDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc);
    void moveDocumentToGroupFolder(const WIZDOCUMENTDATA& doc);

protected:
    CWizDocumentDataArray m_arrayDocument;
    QString m_targetFolder;
    WIZTAGDATA m_targetTag;
    bool m_keepDocTime;
    bool m_keepTag;
    CWizDatabaseManager& m_dbMgr;
    CWizObjectDataDownloaderHost* m_downloader;

    //
    QThread* m_thread;
    bool m_stop;
};



#endif // CWIZOBJECTOPERATOR_H
