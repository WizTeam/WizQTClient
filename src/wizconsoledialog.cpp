#include "wizconsoledialog.h"
#include "ui_wizconsoledialog.h"

#include <QTextCodec>
#include <QMessageBox>

#include "share/wizmisc.h"

CWizConsoleDialog::CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , m_app(app)
    , m_ui(new Ui::CWizConsoleDialog)
{
    setWindowFlags(Qt::Tool);

    m_ui->setupUi(this);
    vScroll = m_ui->editConsole->verticalScrollBar();

    connect(m_ui->editConsole, SIGNAL(textChanged()), SLOT(on_editConsole_textChanged()));
    connect(m_ui->buttonClear, SIGNAL(clicked()), SLOT(on_buttonClear_clicked()));
    connect(::WizGlobal()->bufferLog(), SIGNAL(readyRead()), SLOT(bufferLog_readyRead()));

    m_ui->buttonSync->setDown(true);

    m_ui->buttonAll->setEnabled(false);
    m_ui->buttonWarn->setEnabled(false);
    m_ui->buttonError->setEnabled(false);

    load();
}

void CWizConsoleDialog::load()
{
    QString strLogFileName = ::WizGetLogFileName();
    QFile file(strLogFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = file.readAll();
    file.close();

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString strLogData = codec->toUnicode(data);

    m_data = strLogData;

    m_ui->editConsole->setText(m_data);
}

void CWizConsoleDialog::on_editConsole_textChanged()
{
    vScroll->setValue(vScroll->maximum());
}

void CWizConsoleDialog::on_buttonClear_clicked()
{
    QString strLogFileName = ::WizGetLogFileName();
    QFile file(strLogFileName);
    file.open(QIODevice::Truncate | QIODevice::WriteOnly);
    file.close();

    m_ui->editConsole->clear();
}

void CWizConsoleDialog::bufferLog_readyRead()
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
