#include "notecomments.h"
#include "notecomments_p.h"

#include <QDebug>

#include <coreplugin/icore.h>

#include "wizDocumentView.h"
#include "share/wizobject.h"
#include "sync/token.h"
#include "sync/apientry.h"

using namespace Core;

namespace WizService {
namespace Internal {

static NoteCommentsPrivate* m_comments = 0;

NoteCommentsPrivate::NoteCommentsPrivate()
{
    //connect(ICore::instance(), SIGNAL(viewNoteLoaded(Core::CWizDocumentView*, const WIZDOCUMENTDATA&)),
    //        SLOT(onViewNoteLoaded(Core::CWizDocumentView*, const WIZDOCUMENTDATA&)));
    //connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)));
}

NoteCommentsPrivate::~NoteCommentsPrivate()
{
    m_comments = 0;
}

void NoteCommentsPrivate::onViewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& note)
{
    m_map.insert(view, note);

    Token::requestToken();
}

void NoteCommentsPrivate::onTokenAcquired(const QString& strToken)
{
    if (strToken.isEmpty())
        return;

    loadComments(strToken);
}

void NoteCommentsPrivate::onCommentsButtonClicked()
{
}

void NoteCommentsPrivate::loadComments(const QString& strToken)
{
    QMap<CWizDocumentView*, WIZDOCUMENTDATA>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++) {
        qDebug() << CommonApiEntry::commentUrl(strToken, it.value().strKbGUID, it.value().strGUID);
    }
}


} // namespace Internal


NoteComments::NoteComments()
{
}

void NoteComments::init()
{
    if (!Internal::m_comments) {
        Internal::m_comments = new Internal::NoteCommentsPrivate();
    }
}


} // namespace WizService
