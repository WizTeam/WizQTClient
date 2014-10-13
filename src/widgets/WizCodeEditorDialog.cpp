#include "WizCodeEditorDialog.h"
#include "wizdef.h"
#include "utils/pathresolve.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QWebPage>
#include <QWebFrame>
#include <QWebView>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QDir>
#include <QPlainTextEdit>
#include <QEvent>
#include <QDebug>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <coreplugin/icore.h>

WizCodeEditorDialog::WizCodeEditorDialog(QWidget *parent) :
    QDialog(parent)
  , m_codeType(new QComboBox(this))
  , m_codeEditor(new QPlainTextEdit(this))
  , m_codeBrowser(new QWebView(this))
{

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowState(windowState() & ~Qt::WindowFullScreen);
    resize(650, 550);
    //
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    QLabel *labelType = new QLabel(tr("Code Type : "), this);
    horizontalLayout->addWidget(labelType);

    horizontalLayout->addWidget(m_codeType);

    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);

    m_codeEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    //m_codeEditor->page()->setContentEditable(true);

    QWidget* wgtPre = new QWidget(this);
    QVBoxLayout *layoutPre = new QVBoxLayout(wgtPre);
    layoutPre->setContentsMargins(0, 0, 0, 0);
    QLabel *labelPreview = new QLabel(tr("Priview : "), this);
    verticalLayout->addWidget(labelPreview);
    m_codeBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_codeBrowser->setHtml("");
    layoutPre->addWidget(labelPreview);
    layoutPre->addWidget(m_codeBrowser);

    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(m_codeEditor);
    splitter->addWidget(wgtPre);


    QPushButton *btnOK = new QPushButton(this);
    btnOK->setText(tr("OK"));
    QPushButton *btnCancel = new QPushButton(this);
    btnCancel->setText(tr("Cancel"));
    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnCancel);
    layoutButton->addWidget(btnOK);


    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->addWidget(splitter);
    verticalLayout->addLayout(layoutButton);


    initCodeTypeCombox();
    //
    //connect(m_codeEditor->page(), SIGNAL(contentsChanged()), SLOT(renderCodeToHtml()));
    connect(m_codeEditor, SIGNAL(textChanged()), SLOT(renderCodeToHtml()));
    connect(m_codeType, SIGNAL(currentIndexChanged(int)), SLOT(renderCodeToHtml()));
    connect(btnOK, SIGNAL(clicked()), SLOT(onButtonOKClicked()));
    connect(btnCancel, SIGNAL(clicked()), SLOT(onButtonCancelClicked()));
}

void WizCodeEditorDialog::setCode(const QString& strCode)
{
    if (!strCode.isEmpty())
    {
        //m_codeEditor->page()->mainFrame()->setHtml(strCode);
        m_codeEditor->setPlainText(strCode);
        renderCodeToHtml();
    }
}

void WizCodeEditorDialog::renderCodeToHtml()
{
    QWebFrame *frame = m_codeBrowser->page()->mainFrame();
    QString codeText = m_codeEditor->toPlainText();//->page()->mainFrame()->toHtml();
#if QT_VERSION > 0x050000
    codeText = codeText.toHtmlEscaped();
#else
    codeText.replace("<", "&lt;");
    codeText.replace(">", "&gt;");
#endif
    // 需要将纯文本中的空格转换为Html中的空格。直接将文本中的空格写入Html中，会被忽略。
    codeText.replace(" ", "åß∂ƒ");
    codeText.replace("\n", "<br />");

    m_codeBrowser->setUpdatesEnabled(false);
    frame->setHtml(QString("<p>``` %1</p>%2<p>```</p>").arg(m_codeType->currentText()).
                   arg(codeText));
    Core::ICore::instance()->emitFrameRenderRequested(frame, true);
    m_codeBrowser->setUpdatesEnabled(true);
    codeText = frame->toHtml();
    codeText.replace("åß∂ƒ", "&nbsp;");
    frame->setHtml(codeText);
}

void WizCodeEditorDialog::onButtonOKClicked()
{
    QString strHtml = m_codeBrowser->page()->mainFrame()->toHtml();
    //插入Html时，处理特殊字符。
    strHtml.replace("\\n", "\\\\n");
    strHtml.replace("\'", "\\\'");
    insertHtmlRequest(strHtml);
    close();
}

void WizCodeEditorDialog::onButtonCancelClicked()
{
    close();
}

void WizCodeEditorDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}

void WizCodeEditorDialog::initCodeTypeCombox()
{
    QStringList strList;
    strList << "c" << "cpp" << "java" << "js" << "perl" << "sh" << "py" << "Basic" << "CSS" << "Go" << "Lua" << "Pascal" << "SQL" << "Visual Basic"
               "htm" << "cc" << "bsh" << "cs" << "csh" << "cyc" << "cv" << "m" << "mxml" << "html" << "xml"
               "pl" << "pm" << "rb" << "xhtml" << "xsl" << "Apollo" <<  "Clojure" << "Dart" << "Erlang" <<
               "Haskell" << "Lisp" << "Scheme" << "Llvm" << "Matlab" <<  "Mumps" << "Nemerle" <<
               "Protocol buffers" << "R, S" << "RD" << "Scala" << "TCL" << "Latek" << "CHDL" << "Wiki" << "XQ" << "YAML";
    strList.sort();
    foreach (QString str, strList) {
        m_codeType->addItem(str);
    }
#if QT_VERSION < 0x050000
    int index = m_codeType->findText("c");
    m_codeType->setCurrentIndex(index);
#else
    m_codeType->setCurrentText("c");
#endif
}




