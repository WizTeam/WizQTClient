#include "wizconsoledialog.h"
#include "ui_wizconsoledialog.h"

#include <QScrollBar>
#include <QTextCodec>
#include <QFileDialog>
#include <QClipboard>
#include <QTextEdit>
#include <QTextStream>
#include <QMessageBox>

#include "wizdef.h"
#include "share/wizmisc.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"

#include "utils/logger.h"

#ifdef Q_OS_MAC
#include "mac/wizmachelper.h"
#endif

CWizConsoleDialog::CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , m_app(app)
    , m_ui(new Ui::CWizConsoleDialog)
    , m_bAutoScroll(true)
    , m_nPos(0)
{
    m_ui->setupUi(this);
    setWindowFlags(Qt::Tool);

    m_ui->editConsole->setReadOnly(true);
    m_ui->btnCopyToClipboard->setEnabled(false);

    connect(Utils::Logger::buffer(), SIGNAL(readyRead()), SLOT(onLogBufferReadyRead()));
    connect(m_ui->btnSaveAs, SIGNAL(clicked()), SLOT(onBtnSaveAsClicked()));
    connect(m_ui->editConsole, SIGNAL(copyAvailable(bool)), SLOT(onConsoleCopyAvailable(bool)));
    connect(m_ui->btnCopyToClipboard, SIGNAL(clicked()), SLOT(onBtnCopyToClipboardClicked()));
    connect(m_ui->buttonClear, SIGNAL(clicked()), SLOT(onBtnClearClicked()));

    connect(m_ui->editConsole, SIGNAL(textChanged()), SLOT(onConsoleTextChanged()));
    connect(m_ui->editConsole->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(onConsoleSliderMoved(int)));

    load();
}

CWizConsoleDialog::~CWizConsoleDialog()
{
}

void CWizConsoleDialog::showEvent(QShowEvent *event)
{
    QScrollBar* vScroll = m_ui->editConsole->verticalScrollBar();
    vScroll->setValue(vScroll->maximum());

    move(parentWidget()->geometry().center() - rect().center());

    QDialog::showEvent(event);
}

void CWizConsoleDialog::insertLog(const QByteArray& nBytes)
{
    QTextCursor cursor = m_ui->editConsole->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString::fromUtf8(nBytes.data()));

    m_nPos += nBytes.size();
}

void CWizConsoleDialog::load()
{
    QByteArray bytes = Utils::Logger::buffer()->buffer();
    insertLog(bytes);
}

void CWizConsoleDialog::onLogBufferReadyRead()
{
    QBuffer* buf = Utils::Logger::buffer();

    if (!buf->open(QIODevice::ReadOnly)) {
        return;
    }

    buf->seek(m_nPos);
    insertLog(buf->readAll());

    buf->close();
}

void CWizConsoleDialog::onConsoleSliderMoved(int value)
{
    QScrollBar* scroll = qobject_cast<QScrollBar *>(sender());
    if (value == scroll->maximum()) {
        m_bAutoScroll = true;
    } else {
        m_bAutoScroll = false;
    }
}

void CWizConsoleDialog::onConsoleTextChanged()
{
    if (m_bAutoScroll) {
        QScrollBar* vScroll = m_ui->editConsole->verticalScrollBar();
        vScroll->setValue(vScroll->maximum());
    }

    resetCount();
}

void CWizConsoleDialog::onBtnClearClicked()
{
    m_ui->editConsole->clear();

    Utils::Logger::buffer()->setBuffer(0);
    m_nPos = 0;
}

void CWizConsoleDialog::resetCount()
{
    QBuffer* buf = Utils::Logger::buffer();
    if (!buf->open(QIODevice::ReadOnly)) {
        return;
    }

    int nLines = 0;
    QTextStream ts(buf);
    do {
        ts.readLine();
        nLines++;
    } while (!ts.atEnd());

    buf->close();

    QString strCount = m_ui->labelCount->text();
    strCount = strCount.replace(QRegExp("%1|\\d+"), QString::number(nLines));
    m_ui->labelCount->setText(strCount);
}

void CWizConsoleDialog::onBtnSaveAsClicked()
{
    QString strToday = QDate::currentDate().toString(Qt::ISODate);
    QString strFileName = QString("WizNote_%1_%2.txt").arg(GetTickCount()).arg(strToday);

    QString strFilePath = QFileDialog::getSaveFileName(this, tr("Save log"), QDir::home().filePath(strFileName));
    if (strFilePath.isEmpty())
        return;

    if (QFile::exists(strFilePath)) {
        QMessageBox::warning(this, tr("Failed to save"), tr("you should select a file name which does not exists"));
        return;
    }

    QFile file(strFilePath);
    file.open(QIODevice::Truncate | QIODevice::WriteOnly);
    file.write(Utils::Logger::buffer()->data());
    file.close();
}

void CWizConsoleDialog::onConsoleCopyAvailable(bool yes)
{
    m_ui->btnCopyToClipboard->setEnabled(yes);
}

void CWizConsoleDialog::onBtnCopyToClipboardClicked()
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
