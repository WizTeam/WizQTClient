#include "wizDocumentTransitionView.h"

#include <QLabel>
#include <QVBoxLayout>

CWizDocumentTransitionView::CWizDocumentTransitionView(QWidget *parent) :
    QWidget(parent)
{
    m_labelHint = new QLabel(this);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addStretch(1);
    layout->addWidget(m_labelHint);
    layout->addStretch(1);
    setLayout(layout);
}

void CWizDocumentTransitionView::showAsMode(TransitionMode mode)
{
    Q_UNUSED(mode);

    if (mode == Downloading) {
        m_labelHint->setText(tr("Downloading document from cloud server..."));
    } else if (mode == ErrorOccured) {
        m_labelHint->setText(tr("Error occured while loading document."));
    }

    show();
}
