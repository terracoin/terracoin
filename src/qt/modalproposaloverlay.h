// Copyright (c) 2018 The Terracoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_MODALPROPOSALOVERLAY_H
#define BITCOIN_QT_MODALPROPOSALOVERLAY_H

#include <QWidget>

namespace Ui {
    class ModalProposalOverlay;
}

/** Modal overlay to display information about the proposal submission */
class ModalProposalOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit ModalProposalOverlay(QWidget *parent);
    ~ModalProposalOverlay();

public Q_SLOTS:
    void updateConfirmations(int count, bool unlock, bool failed);

    void toggleVisibility(QString submitStr);
    // will show or hide the modal layer
    void showHide(bool hide = false, bool userRequested = false);
    void closeClicked();
    bool isLayerVisible() { return layerIsVisible; }

protected:
    bool eventFilter(QObject * obj, QEvent * ev);
    bool event(QEvent* ev);

private:
    Ui::ModalProposalOverlay *ui;
    bool layerIsVisible;
    bool userClosed;
};

#endif // BITCOIN_QT_MODALPROPOSALOVERLAY_H
