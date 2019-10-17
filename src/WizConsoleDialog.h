#ifndef WIZCONSOLEDIALOG_H
#define WIZCONSOLEDIALOG_H

#include <QDialog>

class WizExplorerApp;
class QByteArray;

namespace Ui {
    class WizConsoleDialog;
}

class WizConsoleDialog: public QDialog
{
    Q_OBJECT

public:
    WizConsoleDialog(WizExplorerApp& app, QWidget* parent = 0);
    ~WizConsoleDialog();

private:
    WizExplorerApp& m_app;
    Ui::WizConsoleDialog* m_ui;

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
