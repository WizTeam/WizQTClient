#include "wizDocumentTransitionView.h"

#include <QLabel>
#include <QVBoxLayout>

CWizDocumentTransitionView::CWizDocumentTransitionView(QWidget *parent) :
    QWidget(parent)
  , m_mode(-1)
{
    m_labelHint = new QLabel(this);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addStretch(1);
    layout->addWidget(m_labelHint);
    layout->addStretch(1);
    layout->setAlignment(m_labelHint, Qt::AlignCenter);
    setLayout(layout);

}

void CWizDocumentTransitionView::showAsMode(TransitionMode mode)
{
    m_mode = mode;
    if (m_mode == Downloading) {
        m_labelHint->setText(tr("Downloading note from cloud server..."));
    } else if (m_mode == ErrorOccured) {
        m_labelHint->setText(tr("Error occured while loading note."));
    }

    show();
}

void CWizDocumentTransitionView::onDownloadProgressChanged(QString strObjGUID, int ntotal, int nloaded)
{
    if (isVisible() && m_mode == Downloading)
    {
        int nPercent = nloaded * 100 / (ntotal + 1);
        m_labelHint->setText(QString(tr("Downloading note from cloud server( %1 %)...").arg(nPercent)));
    }
}
