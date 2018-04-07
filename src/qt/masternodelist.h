#ifndef MASTERNODELIST_H
#define MASTERNODELIST_H

#include "primitives/transaction.h"
#include "platformstyle.h"
#include "sync.h"
#include "util.h"

#include <QMenu>
#include <QTimer>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define MY_MASTERNODELIST_UPDATE_SECONDS                 60
#define MASTERNODELIST_UPDATE_SECONDS                    15
#define MASTERNODELIST_FILTER_COOLDOWN_SECONDS            3

namespace Ui {
    class MasternodeList;
}

class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Masternode Manager page widget */
class MasternodeList : public QWidget
{
    Q_OBJECT

public:
    explicit MasternodeList(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~MasternodeList();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void StartAlias(std::string strAlias);
    void StartAll(std::string strCommand = "start-all");
    void VoteMany(std::string strCommand);

private:
    QMenu *contextMenu;
    int64_t nTimeFilterUpdated;
    bool fFilterUpdated;

public Q_SLOTS:
    void updateMyMasternodeInfo(QString strAlias, QString strAddr, const COutPoint& outpoint);
    void updateMyNodeList(bool fForce = false);
    void updateNodeList();
    void updateVoteList(bool reset = false);

    void updateProposalTotals();
    void populateStartDates();
    uint256 prepareProposal(uint256 hashParent, int nRevision, int64_t nTime, std::string strData);
    uint256 submitProposal(uint256 hashParent, int nRevision, int64_t nTime, uint256 txidFee, std::string strData);

Q_SIGNALS:
    void requestedProposalOverlay(QString submitStr);
    void requestedConfirmationUpdate(int count, bool unlock, bool failed);

private:
    QTimer *timer;
    Ui::MasternodeList *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;

    // Protects tableWidgetMasternodes
    CCriticalSection cs_mnlist;

    // Protects tableWidgetMyMasternodes
    CCriticalSection cs_mymnlist;

    QString strCurrentFilter;

    QString strCurrentName;
    double doubleCurrentAmount = 0;
    int intCurrentPayments = 1;

    void showProposalModal(QString submitStr);
    void updateProposalConfirmations(int count, bool unlock, bool failed);
    void formIsValid();

private Q_SLOTS:
    void showContextMenu(const QPoint &);
    void on_filterLineEdit_textChanged(const QString &strFilterIn);
    void on_startButton_clicked();
    void on_startAllButton_clicked();
    void on_startMissingButton_clicked();
    void on_tableWidgetMyMasternodes_itemSelectionChanged();
    void on_UpdateButton_clicked();

    void checkAvailName(QNetworkReply *NetReply);

    void on_voteManyYesButton_clicked();
    void on_voteManyNoButton_clicked();
    void on_voteManyAbstainButton_clicked();
    void on_tableWidgetVoting_itemSelectionChanged();
    void on_UpdateVotesButton_clicked();

    void on_proposalName_textChanged(const QString &strProposalName);
    void on_trcAddress_textChanged(const QString &strAddress);
    void on_paymentSlider_valueChanged(const int &intPayments);
    void on_amounttrc_valueChanged(const double &doubleAmount);
    void on_createProposal_clicked();
};
#endif // MASTERNODELIST_H
