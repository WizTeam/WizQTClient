#include "notecomments.h"
#include "notecomments_p.h"

#include <QDebug>

#include "wizDocumentView.h"
#include "share/wizobject.h"
#include "sync/token.h"
#include "sync/apientry.h"

static NoteCommentsPrivate* g_comments = 0;

NoteCommentsPrivate::NoteCommentsPrivate()
{
    //connect(WizGlobal::instance(), SIGNAL(viewNoteLoaded(CWizDocumentView*, const WIZDOCUMENTDATA&)),
    //        SLOT(onViewNoteLoaded(CWizDocumentView*, const WIZDOCUMENTDATA&)));
    //connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)));
}

NoteCommentsPrivate::~NoteCommentsPrivate()
{
    g_comments = 0;
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



NoteComments::NoteComments()
{
}

void NoteComments::init()
{
    if (!g_comments) {
        g_comments = new NoteCommentsPrivate();
    }
}


