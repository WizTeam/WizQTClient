#ifndef WIZEMAILSHAREDIALOG_H
#define WIZEMAILSHAREDIALOG_H

#include <QDialog>
#include "WizDef.h"
#include "share/WizObject.h"

namespace Ui {
class WizEmailShareDialog;
}

class QListWidgetItem;
class QListWidget;
class QNetworkReply;
class QNetworkAccessManager;
class WizEmailShareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizEmailShareDialog(WizExplorerApp& app, QWidget *parent = 0);
    ~WizEmailShareDialog();

    void setNote(const WIZDOCUMENTDATA& note, const QString& sendTo = "");

    bool isInsertCommentToNote() const;
    QString getCommentsText() const;

signals:
    void insertCommentToNoteRequest(const QString& docGUID, const QString& comment);

private slots:
    void on_toolButton_send_clicked();

    void on_toolButton_contacts_clicked();

    void on_contactsListItemClicked(QListWidgetItem *item);

    void on_networkFinished(QNetworkReply* reply);

    void on_networkError(const QString& errorMsg);

    void on_mailShare_finished(int nCode, const QString& returnMessage);

    void on_toolButton_settings_clicked();

    void signature_text_edit_finished();
    void autoInsert_state_changed(bool checked);

private:
    QString getExInfo();    
    void processReturnMessage(const QString& returnMessage, int& nCode, QString& message);
    void saveContacts();
    void updateContactList();

    void sendEmails();

private:
    Ui::WizEmailShareDialog *ui;
    WIZDOCUMENTDATA m_note;
    WizExplorerApp& m_app;
    QDialog* m_contactDialog;
    QListWidget* m_contactList;
    QNetworkAccessManager* m_net;
};

#endif // WIZEMAILSHAREDIALOG_H
