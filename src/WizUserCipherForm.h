#ifndef WIZUSERCIPHERFORM_H
#define WIZUSERCIPHERFORM_H

#include <QWidget>
#include "WizDef.h"

namespace Ui {
class WizUserCipherForm;
}

class QPropertyAnimation;
class WizUserCipherForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit WizUserCipherForm(WizExplorerApp& app, QWidget *parent);

    void setPasswordText(const QString& strPasswordText);
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
    Ui::WizUserCipherForm *ui;
    WizExplorerApp& m_app;

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
