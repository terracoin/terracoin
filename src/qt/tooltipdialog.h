#ifndef TOOLTIPDIALOG_H
#define TOOLTIPDIALOG_H

#include <QDialog>

class ToolTipDialog : public QDialog
{
    Q_OBJECT

public:
    ToolTipDialog(QWidget *parent=0, const QString &title=QString(""), const QString &content=QString(""), const QString &url=QString(""));
};

#endif // TOOLTIPDIALOG_H
