#include "WizDocumentTransitionView.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>

#include "share/WizAnimateAction.h"
#include "share/WizUIBase.h"

WizDocumentTransitionView::WizDocumentTransitionView(QWidget *parent) :
    QWidget(parent)
  , m_mode(-1)
  , m_objGUID(QString())
{
#ifndef Q_OS_MAC
    if (isDarkMode()) {
        setStyleSheet("color:#e9e9e9");
    }
#endif

    m_labelHint = new QLabel(this);

    m_toolButton = new QToolButton(this);
    m_toolButton->setFixedSize(50,36);

    m_animation = new WizAnimateAction(this);
    m_animation->setToolButton(m_toolButton);

    m_animation->setSingleIcons("transitionViewDownloading");
    m_toolButton->setIconSize(QSize(50,36));

    m_toolButton->setVisible(false);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addStretch(1);
    hLayout->addWidget(m_toolButton);
    hLayout->addWidget(m_labelHint);
    hLayout->addStretch(1);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addStretch(1);      
    layout->addLayout(hLayout);
    layout->addStretch(1);
    layout->setAlignment(m_labelHint, Qt::AlignCenter);
    setLayout(layout);
    //
}

void WizDocumentTransitionView::showAsMode(const QString& strObjGUID, TransitionMode mode)
{
    m_mode = mode;
    if (m_mode == Downloading) {
        m_labelHint->setText(tr("Downloading note from cloud server..."));
        m_toolButton->setVisible(true);
        m_animation->startPlay();
    } else if (m_mode == ErrorOccured) {
        m_labelHint->setText(tr("Error occured while loading note."));
        m_animation->stopPlay();
        m_toolButton->setVisible(false);
    }

    m_objGUID = strObjGUID;

    show();
}

void WizDocumentTransitionView::onDownloadProgressChanged(QString strObjGUID, int ntotal, int nloaded)
{
    if (isVisible() && strObjGUID == m_objGUID && m_mode == Downloading)
    {
        int nPercent = nloaded * 100 / (ntotal + 1);
        m_labelHint->setText(QString(tr("Downloading note from cloud server( %1 %)...").arg(nPercent)));
    }
}

void WizDocumentTransitionView::hideEvent(QHideEvent* ev)
{
    m_animation->stopPlay();
    m_toolButton->setVisible(false);

    QWidget::hideEvent(ev);

}
