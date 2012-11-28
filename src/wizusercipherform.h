#ifndef WIZUSERCIPHERFORM_H
#define WIZUSERCIPHERFORM_H

#include <QFrame>

#include "wizdef.h"

namespace Ui {
class CWizUserCipherForm;
}

class CWizUserCipherForm : public QFrame
{
    Q_OBJECT
    
public:
    explicit CWizUserCipherForm(CWizExplorerApp& app, QWidget *parent);
    ~CWizUserCipherForm();

    void setHint(const QString& strHint);

    const QString& userCipher() const { return m_userCipher; }
    bool isSaveForSession() const { return m_bSaveForSession; }

    void sheetShow();

protected:
    virtual QSize sizeHint();
    virtual void showEvent(QShowEvent* event);
    
private:
    Ui::CWizUserCipherForm *ui;
    CWizExplorerApp& m_app;

    bool m_bSaveForSession;
    QString m_userCipher;

Q_SIGNALS:
    void accepted();

public Q_SLOTS:
    void accept();
    void onCheckSave_stateChanged(int state);

};

#endif // WIZUSERCIPHERFORM_H
