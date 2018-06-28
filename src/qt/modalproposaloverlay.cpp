// Copyright (c) 2018 The Terracoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "modalproposaloverlay.h"
#include "ui_modalproposaloverlay.h"

#include "guiutil.h"

#include "chainparams.h"

#include "governance-object.h"

#include <QResizeEvent>
#include <QPropertyAnimation>

ModalProposalOverlay::ModalProposalOverlay(QWidget *parent) :
QWidget(parent),
ui(new Ui::ModalProposalOverlay),
layerIsVisible(false),
userClosed(false)
{
    ui->setupUi(this);
    ui->progressBar->reset();
    ui->label_total->setText(QString::number(GOVERNANCE_FEE_CONFIRMATIONS));
    ui->progressBar->setRange(0, 100);
    ui->label_time->setText("This should take about " + QString::number(GOVERNANCE_FEE_CONFIRMATIONS * (Params().GetConsensus().nPowTargetSpacing / 60)) + " minutes");
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(closeClicked()));
    if (parent) {
        parent->installEventFilter(this);
        raise();
    }

    setVisible(false);
}

ModalProposalOverlay::~ModalProposalOverlay()
{
    delete ui;
}

bool ModalProposalOverlay::eventFilter(QObject * obj, QEvent * ev) {
    if (obj == parent()) {
        if (ev->type() == QEvent::Resize) {
            QResizeEvent * rev = static_cast<QResizeEvent*>(ev);
            resize(rev->size());
            if (!layerIsVisible)
                setGeometry(0, height(), width(), height());

        }
        else if (ev->type() == QEvent::ChildAdded) {
            raise();
        }
    }
    return QWidget::eventFilter(obj, ev);
}

//! Tracks parent widget changes
bool ModalProposalOverlay::event(QEvent* ev) {
    if (ev->type() == QEvent::ParentAboutToChange) {
        if (parent()) parent()->removeEventFilter(this);
    }
    else if (ev->type() == QEvent::ParentChange) {
        if (parent()) {
            parent()->installEventFilter(this);
            raise();
        }
    }
    return QWidget::event(ev);
}

void ModalProposalOverlay::updateConfirmations(int count, bool unlock, bool failed)
{
    if (failed) {
        ui->label_failed->setText("<strong>Submit failed please copy this submit statement and try to run it manually in the debug console.</strong>");
    }

    int percentage = (count * 100 ) / GOVERNANCE_FEE_CONFIRMATIONS;
    if (percentage > 100)
        percentage = 100;

    // show confirms
    ui->label_count->setText(QString::number(count));

    // show the percentage done
    ui->progressBar->setValue(percentage);

    if (count >= GOVERNANCE_FEE_CONFIRMATIONS || unlock) {
        if (count >= GOVERNANCE_FEE_CONFIRMATIONS) {
            ui->label_failed->setText("<strong>SUBMITTED</strong>");
        } else {
            ui->label_failed->setText("<strong>Confirmations are taking too long. You can continue to wait, or copy the command below and run it by hand once the confirmations have occurred.</strong>");
        }
        ui->closeButton->setEnabled(true);
    }
}

void ModalProposalOverlay::toggleVisibility(QString submitStr)
{
    ui->label_submitStr->setText("<strong>Debug Console Submit Command:</strong><br><br>" + submitStr);

    showHide(layerIsVisible, true);
    if (!layerIsVisible)
        userClosed = true;
}

void ModalProposalOverlay::showHide(bool hide, bool userRequested)
{
    if ( (layerIsVisible && !hide) || (!layerIsVisible && hide) || (!hide && userClosed && !userRequested))
        return;

    if (!isVisible() && !hide)
        setVisible(true);

    setGeometry(0, hide ? 0 : height(), width(), height());

    QPropertyAnimation* animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(300);
    animation->setStartValue(QPoint(0, hide ? 0 : this->height()));
    animation->setEndValue(QPoint(0, hide ? this->height() : 0));
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    layerIsVisible = !hide;
}

void ModalProposalOverlay::closeClicked()
{
    showHide(true);

    ui->closeButton->setEnabled(false);
    ui->label_count->setText(QString::number(0));
    ui->progressBar->reset();
    ui->label_submitStr->setText("");
    ui->label_failed->setText("");

    userClosed = true;
}
