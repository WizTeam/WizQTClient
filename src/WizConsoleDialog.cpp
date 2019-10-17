#include "WizConsoleDialog.h"
#include "ui_WizConsoleDialog.h"

#include <QScrollBar>
#include <QTextCodec>
#include <QFileDialog>
#include <QClipboard>
#include <QTextEdit>
#include <QTextStream>
#include <QMessageBox>

#include "WizDef.h"
#include "share/WizMisc.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "share/WizUIBase.h"
#include "utils/WizLogger.h"

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#endif

WizConsoleDialog::WizConsoleDialog(WizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , m_app(app)
    , m_ui(new Ui::WizConsoleDialog)
    , m_bAutoScroll(true)
    , m_nPos(0)
{
    if (isDarkMode()) {
#ifndef Q_OS_MAC
        QString darkStyleSheet = QString("color:#a6a6a6").arg(WizColorLineEditorBackground.name());
        setStyleSheet(darkStyleSheet);
#endif
    }

    m_ui->setupUi(this);
    //setWindowFlags(Qt::Tool);

    m_ui->editConsole->setReadOnly(true);
    m_ui->btnCopyToClipboard->setEnabled(false);

    connect(Utils::WizLogger::logger(), SIGNAL(readyRead()), SLOT(onLogBufferReadyRead()));
    connect(m_ui->btnSaveAs, SIGNAL(clicked()), SLOT(onBtnSaveAsClicked()));
    connect(m_ui->editConsole, SIGNAL(copyAvailable(bool)), SLOT(onConsoleCopyAvailable(bool)));
    connect(m_ui->btnCopyToClipboard, SIGNAL(clicked()), SLOT(onBtnCopyToClipboardClicked()));
    connect(m_ui->buttonClear, SIGNAL(clicked()), SLOT(onBtnClearClicked()));

    connect(m_ui->editConsole, SIGNAL(textChanged()), SLOT(onConsoleTextChanged()));
    connect(m_ui->editConsole->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(onConsoleSliderMoved(int)));
    //
    //
    load();
    //
    if (isDarkMode()) {
        QString darkStyleSheet = QString("background-color:%1").arg(WizColorLineEditorBackground.name());
        m_ui->editConsole->setStyleSheet(darkStyleSheet);
    }
}

WizConsoleDialog::~WizConsoleDialog()
{
}

void WizConsoleDialog::insertLog(const QString& text)
{
    QTextCursor cursor = m_ui->editConsole->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text);
}

void WizConsoleDialog::load()
{
    QString text;
    Utils::WizLogger::getAllLogs(text);
    insertLog(text);
}

void WizConsoleDialog::onLogBufferReadyRead()
{
    load();
}

void WizConsoleDialog::onConsoleSliderMoved(int value)
{
    QScrollBar* scroll = qobject_cast<QScrollBar *>(sender());
    if (value == scroll->maximum()) {
        m_bAutoScroll = true;
    } else {
        m_bAutoScroll = false;
    }
}

void WizConsoleDialog::onConsoleTextChanged()
{
    if (m_bAutoScroll) {
        QScrollBar* vScroll = m_ui->editConsole->verticalScrollBar();
        vScroll->setValue(vScroll->maximum());
    }

    resetCount();
}

void WizConsoleDialog::onBtnClearClicked()
{
    m_ui->editConsole->clear();
}

void WizConsoleDialog::resetCount()
{
}

void WizConsoleDialog::onBtnSaveAsClicked()
{
    QString strToday = QDate::currentDate().toString(Qt::ISODate);
    QString strFileName = QString("WizNote_%1_%2.txt").arg(WizGetTickCount()).arg(strToday);

    QString strFilePath = QFileDialog::getSaveFileName(this, tr("Save log"), QDir::home().filePath(strFileName));
    if (strFilePath.isEmpty())
        return;

    if (QFile::exists(strFilePath)) {
        QMessageBox::warning(this, tr("Failed to save"), tr("you should select a file name which does not exists"));
        return;
    }
    //
    QString text = m_ui->editConsole->document()->toPlainText();

    QFile file(strFilePath);
    file.open(QIODevice::Truncate | QIODevice::WriteOnly);
    file.write(text.toUtf8());
    file.close();
}

void WizConsoleDialog::onConsoleCopyAvailable(bool yes)
{
    m_ui->btnCopyToClipboard->setEnabled(yes);
}

void WizConsoleDialog::onBtnCopyToClipboardClicked()
{
    m_ui->editConsole->copy();

    QClipboard* clipboard = QApplication::clipboard();
    QString strText = clipboard->text();

    QString strOutput;
    strOutput += QString("Version: WizNote %1 %2\n").arg(WIZ_CLIENT_TYPE).arg(WIZ_CLIENT_VERSION);
#ifdef Q_OS_MAC
    strOutput += QString("OS: %1\n").arg(WizMacGetOSVersion());
#elif defined(Q_OS_LINUX)
    // FIXME: add distribution, release number, etc..
#endif
    strOutput += QString("Username: %1\n").arg(m_app.databaseManager().db().getUserId());
    strOutput += QString("Time: %1 %2\n").arg(QDate::currentDate().toString(Qt::ISODate)).arg(QTime::currentTime().toString(Qt::ISODate));
    strOutput += "\n";
    strOutput += strText;

    clipboard->setText(strOutput);
}
