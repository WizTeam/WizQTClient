#include "wizconsoledialog.h"

#include "ui_wizconsoledialog.h"

#include <QTextCodec>
#include <QScrollBar>

CWizConsoleDialog::CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::CWizConsoleDialog)
    , m_app(app)
{
    m_ui->setupUi(this);
    setWindowTitle(tr("Console"));

    load();

    connect(m_ui->editConsole, SIGNAL(textChanged()), SLOT(on_editConsole_textChanged()));

    m_ui->editConsole->setText(m_data);
}

void CWizConsoleDialog::load()
{
    QString strLogFileName = ::WizGetLogFileName();
    QFile file(strLogFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QByteArray data = file.readAll();
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    m_data = codec->toUnicode(data);
    file.close();
}

void CWizConsoleDialog::on_editConsole_textChanged()
{
    QScrollBar *sb = m_ui->editConsole->verticalScrollBar();
    sb->setValue(sb->maximum());
}
