#include "wizconsoledialog.h"
#include "ui_wizconsoledialog.h"

#include <QtGui>

#include "wizdef.h"
#include "share/wizmisc.h"
#include "share/wizDatabaseManager.h"

#ifdef Q_WS_MAC
#include "mac/wizmachelper.h"
#endif

#define MAXIMUM_LOG_ENTRIES 10000


CWizConsoleDialog::CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , m_app(app)
    , m_ui(new Ui::CWizConsoleDialog)
    , m_nEntries(0)
    , m_bAutoScroll(true)
{
    m_ui->setupUi(this);
    setWindowFlags(Qt::Tool);

    m_ui->editConsole->setReadOnly(true);
    m_ui->btnCopyToClipboard->setEnabled(false);
    m_codec = QTextCodec::codecForName("UTF-8");

    connect(::WizGlobal()->bufferLog(), SIGNAL(readyRead()), SLOT(bufferLog_readyRead()));
    connect(m_ui->btnSaveAs, SIGNAL(clicked()), SLOT(on_btnSaveAs_clicked()));
    connect(m_ui->editConsole, SIGNAL(copyAvailable(bool)), SLOT(on_editConsole_copyAvailable(bool)));
    connect(m_ui->btnCopyToClipboard, SIGNAL(clicked()), SLOT(on_btnCopyToClipboard_clicked()));
    connect(m_ui->buttonClear, SIGNAL(clicked()), SLOT(on_buttonClear_clicked()));

    connect(m_ui->editConsole, SIGNAL(textChanged()), SLOT(on_editConsole_textChanged()));
    connect(m_ui->editConsole->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(onEditConsole_sliderMoved(int)));

    load();
}

CWizConsoleDialog::~CWizConsoleDialog()
{
    if (m_nEntries < MAXIMUM_LOG_ENTRIES) {
        return;
    }

    QString strLogFileName = ::WizGetLogFileName();
    QFile file(strLogFileName);
    file.open(QIODevice::Truncate | QIODevice::WriteOnly);

    int i = 0;
    QBuffer buffer;
    buffer.setData(m_data.toUtf8());
    buffer.open(QIODevice::ReadOnly);
    while(i < m_nEntries) {
        QByteArray bufLine = buffer.readLine();
        if (i >= m_nEntries - MAXIMUM_LOG_ENTRIES) { // skip old entries
            file.write(bufLine);
        }

        i++;
    }

    buffer.close();
    file.close();
}

void CWizConsoleDialog::showEvent(QShowEvent *event)
{
    QScrollBar* vScroll = m_ui->editConsole->verticalScrollBar();
    vScroll->setValue(vScroll->maximum());

    move(parentWidget()->geometry().center() - rect().center());

    QDialog::showEvent(event);
}

void CWizConsoleDialog::load()
{
    QString strLogFileName = ::WizGetLogFileName();
    QFile file(strLogFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    readByLine(&file);
    file.close();

    resetCount();
}

void CWizConsoleDialog::onEditConsole_sliderMoved(int value)
{
    QScrollBar* scroll = qobject_cast<QScrollBar *>(sender());
    if (value == scroll->maximum()) {
        m_bAutoScroll = true;
    } else {
        m_bAutoScroll = false;
    }
}

void CWizConsoleDialog::on_editConsole_textChanged()
{
    if (m_bAutoScroll) {
        QScrollBar* vScroll = m_ui->editConsole->verticalScrollBar();
        vScroll->setValue(vScroll->maximum());
    }
}

void CWizConsoleDialog::on_buttonClear_clicked()
{
    QString strLogFileName = ::WizGetLogFileName();
    QFile file(strLogFileName);
    file.open(QIODevice::Truncate | QIODevice::WriteOnly);
    file.close();

    m_ui->editConsole->clear();
    m_data.clear();
    m_nEntries = 0;
    resetCount();
}

void CWizConsoleDialog::bufferLog_readyRead()
{
    QBuffer* buffer = WizGlobal()->bufferLog();
    buffer->open(QIODevice::ReadWrite);
    // read all, to avoid write on read issue.
    QByteArray bytesLog = buffer->readAll();
    buffer->close();
    buffer->setData("");

    QBuffer bufLog(&bytesLog);
    bufLog.open(QIODevice::ReadOnly);
    readByLine(&bufLog);
    bufLog.close();

    resetCount();
}

void CWizConsoleDialog::readByLine(QIODevice* dev)
{
    while (1) {
        QByteArray data = dev->readLine();
        if (data.isEmpty()) {
            break;
        }

        m_data += QString::fromUtf8(data);
        QString strText = m_codec->toUnicode(data);

        // use copyed cursor instead
        QTextCursor cursor = m_ui->editConsole->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(strText);

        m_nEntries++;
    }
}

void CWizConsoleDialog::resetCount()
{
    QString strCount = m_ui->labelCount->text();
    strCount = strCount.replace(QRegExp("%1|\\d+"), QString::number(m_nEntries));
    m_ui->labelCount->setText(strCount);
}

void CWizConsoleDialog::on_btnSaveAs_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    QString strToday = QDate::currentDate().toString(Qt::ISODate);
    QString strFileName = QString("WizNote_%1_%2.txt").arg(GetTickCount()).arg(strToday);
    fileDialog.selectFile(strFileName);

    if (fileDialog.exec() == QDialog::Accepted) {
        QStringList files = fileDialog.selectedFiles();
        QString selected;
        if (files.isEmpty()) {
            return;
        }

        selected = files[0];
        QFile file(selected);
        file.open(QIODevice::Truncate | QIODevice::WriteOnly);
        file.write(m_data.toUtf8());
        file.close();
    }
}

void CWizConsoleDialog::on_editConsole_copyAvailable(bool yes)
{
    m_ui->btnCopyToClipboard->setEnabled(yes);
}

void CWizConsoleDialog::on_btnCopyToClipboard_clicked()
{
    m_ui->editConsole->copy();

    QClipboard* clipboard = QApplication::clipboard();
    QString strText = clipboard->text();

    QString strOutput;
    strOutput += QString("Version: WizNote %1 %2\n").arg(WIZ_CLIENT_TYPE).arg(WIZ_CLIENT_VERSION);
#ifdef Q_WS_MAC
    strOutput += QString("OS: %1\n").arg(WizMacGetOSVersion());
#elif defined(Q_WS_X11)
    // FIXME: add distribution, release number, etc..
#endif
    strOutput += QString("Username: %1\n").arg(m_app.databaseManager().db().getUserId());
    strOutput += QString("Time: %1 %2\n").arg(QDate::currentDate().toString(Qt::ISODate)).arg(QTime::currentTime().toString(Qt::ISODate));
    strOutput += "\n";
    strOutput += strText;

    clipboard->setText(strOutput);

}
