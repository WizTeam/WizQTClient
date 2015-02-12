#include "wizWebEngineInjectObject.h"
#include "widgets/WizCodeEditorDialog.h"
#include <QDesktopServices>


CWizCodeExternal::CWizCodeExternal(WizCodeEditorDialog* editor, QObject* parent)
    : m_editor(editor)
    , QObject(parent)
{
}

void CWizCodeExternal::accept()
{
    m_editor->accept();
}

void CWizCodeExternal::reject()
{
    m_editor->reject();
}

void CWizCodeExternal::insertHtml(const QString& strResult)
{
    m_editor->insertHtml(strResult);
}

QString CWizCodeExternal::getLastCodeType()
{
    return m_editor->getLastCodeType();
}

void CWizCodeExternal::saveLastCodeType(const QString& codeType)
{
    m_editor->saveLastCodeType(codeType);
}



CWizCommentsExternal::CWizCommentsExternal(QObject* parent)
    : QObject(parent)
{
}

void CWizCommentsExternal::openUrl(const QString& strUrl)
{
    QDesktopServices::openUrl(strUrl);
}
