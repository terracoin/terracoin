#include <QLabel>
#include <QScrollArea>

#include <QGridLayout>

#include "tooltipdialog.h"

ToolTipDialog::ToolTipDialog(QWidget *parent, const QString &title, const QString &content, const QString &url) : QDialog(parent)
{
    setWindowTitle(title);
    setMinimumSize(600, 600);

    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *contentWidget = new QWidget(this);
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout *l = new QVBoxLayout(contentWidget);
    contentWidget->setLayout(l);

    QLabel *proposalUrl = new QLabel("<a href=\"" + url + "\">" + tr("Open in Browser") + "</a>");
    proposalUrl->setOpenExternalLinks(true);

    QLabel *proposalContent = new QLabel(content);
    proposalContent->setWordWrap(true);
    l->addWidget(proposalContent);

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(proposalUrl);
    dialogLayout->addWidget(scrollArea);

    setLayout(dialogLayout);
}
