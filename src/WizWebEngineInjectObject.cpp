#include "WizWebEngineInjectObject.h"
#include "widgets/WizCodeEditorDialog.h"
#include <QDesktopServices>


WizCodeExternal::WizCodeExternal(WizCodeEditorDialog* editor, QObject* parent)
    : m_editor(editor)
    , QObject(parent)
{
}

void WizCodeExternal::accept()
{
    m_editor->accept();
}

void WizCodeExternal::reject()
{
    m_editor->reject();
}

void WizCodeExternal::insertHtml(const QString& strResult)
{
    m_editor->insertHtml(strResult);
}

QString WizCodeExternal::getLastCodeType()
{
    return m_editor->getLastCodeType();
}

void WizCodeExternal::saveLastCodeType(const QString& codeType)
{
    m_editor->saveLastCodeType(codeType);
}



WizCommentsExternal::WizCommentsExternal(QObject* parent)
    : QObject(parent)
{
}

void WizCommentsExternal::openUrl(const QString& strUrl)
{
    QDesktopServices::openUrl(strUrl);
}
