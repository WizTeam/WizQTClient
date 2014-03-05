#ifndef WIZUSERCIPHERFORM_H
#define WIZUSERCIPHERFORM_H

#include <QWidget>

#include "wizdef.h"

namespace Ui {
class CWizUserCipherForm;
}

class QPropertyAnimation;
class CWizUserCipherForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit CWizUserCipherForm(CWizExplorerApp& app, QWidget *parent);
    ~CWizUserCipherForm();

    void setHint(const QString& strHint);

    const QString& userCipher() const { return m_userCipher; }
    bool isSaveForSession() const { return m_bSaveForSession; }

//    void sheetShow();
    void setCipherEditorFocus();

    void cipherError();
    void cipherCorrect();

protected:
    virtual QSize sizeHint();
    virtual void showEvent(QShowEvent* event);
    
private:
    Ui::CWizUserCipherForm *ui;
    CWizExplorerApp& m_app;

    bool m_bSaveForSession;
    QString m_userCipher;
    QPropertyAnimation* m_animation;

Q_SIGNALS:
    void cipherCheckRequest();

public Q_SLOTS:
    void onButtonOK_clicked();
    void onCheckSave_stateChanged(int state);
    void onCipher_changed(const QString& text);

};

#endif // WIZUSERCIPHERFORM_H
