#ifndef WIZCONSOLEDIALOG_H
#define WIZCONSOLEDIALOG_H

#include <QDialog>

class CWizExplorerApp;
class QByteArray;

namespace Ui {
    class CWizConsoleDialog;
}

class CWizConsoleDialog: public QDialog
{
    Q_OBJECT

public:
    CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent = 0);
    ~CWizConsoleDialog();

protected:
    virtual void showEvent(QShowEvent *event);

private:
    CWizExplorerApp& m_app;
    Ui::CWizConsoleDialog* m_ui;

    bool m_bAutoScroll;
    qint64 m_nPos;

    void load();
    void insertLog(const QString& text);
    void resetCount();

public Q_SLOTS:
    void onConsoleTextChanged();
    void onConsoleSliderMoved(int value);
    void onConsoleCopyAvailable(bool yes);
    void onBtnClearClicked();
    void onLogBufferReadyRead();
    void onBtnSaveAsClicked();
    void onBtnCopyToClipboardClicked();

};

#endif // WIZCONSOLEDIALOG_H
