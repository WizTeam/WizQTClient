#include "wizconsoledialog.h"

#include "ui_wizconsoledialog.h"

#include <QTextCodec>
#include <QScrollBar>

#include <QMessageBox>

CWizConsoleDialog::CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::CWizConsoleDialog)
    , m_app(app)
{
    m_ui->setupUi(this);
    setWindowTitle(tr("Console"));

    connect(m_ui->editConsole, SIGNAL(textChanged()), SLOT(on_editConsole_textChanged()));
    connect(::WizGlobal()->bufferLog(), SIGNAL(readyRead()), SLOT(on_bufferLogReady()));

    m_ui->buttonSync->setDown(true);
    load();
}

void CWizConsoleDialog::load()
{
    QString strLogFileName = ::WizGetLogFileName();
    QFile file(strLogFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QByteArray data = file.readAll();
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString strLogData = codec->toUnicode(data);
    file.close();

    m_data = strLogData;

    m_ui->editConsole->setText(m_data);
}

void CWizConsoleDialog::on_editConsole_textChanged()
{
    QScrollBar *sb = m_ui->editConsole->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void CWizConsoleDialog::on_bufferLogReady()
{
    QBuffer* buffer = WizGlobal()->bufferLog();
    buffer->open(QIODevice::ReadWrite);
    QByteArray data = buffer->readAll();
    buffer->close();

    buffer->setData("");

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString strLog = codec->toUnicode(data);

    m_data += strLog;

    appendLogs(strLog);
}

void CWizConsoleDialog::appendLogs(const QString& strLog)
{
    m_ui->editConsole->moveCursor(QTextCursor::End);
    m_ui->editConsole->insertPlainText(strLog);
}
