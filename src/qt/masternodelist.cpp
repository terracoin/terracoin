#include "masternodelist.h"
#include "ui_masternodelist.h"

#include "activemasternode.h"
#include "clientmodel.h"
#include "init.h"
#include "guiutil.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "sync.h"
#include "wallet/wallet.h"
#include "walletmodel.h"

#include "messagesigner.h"
#include "governance.h"
#include "governance-object.h"
#include "governance-vote.h"
#include "governance-classes.h"
#include "governance-validators.h"

#include <QTimer>
#include <QMessageBox>

int GetOffsetFromUtc()
{
#if QT_VERSION < 0x050200
    const QDateTime dateTime1 = QDateTime::currentDateTime();
    const QDateTime dateTime2 = QDateTime(dateTime1.date(), dateTime1.time(), Qt::UTC);
    return dateTime1.secsTo(dateTime2);
#else
    return QDateTime::currentDateTime().offsetFromUtc();
#endif
}

MasternodeList::MasternodeList(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MasternodeList),
    clientModel(0),
    walletModel(0)
{
    ui->setupUi(this);

    ui->startButton->setEnabled(false);
    ui->voteManyYesButton->setEnabled(false);
    ui->voteManyNoButton->setEnabled(false);

    int columnNameWidth = 200;
    int columnCreatedWidth = 90;
    int columnStartWidth = 90;
    int columnEndWidth = 90;
    int columnYesWidth = 45;
    int columnNoWidth = 45;
    int columnAbstainWidth = 45;
    int columnMonthlyWidth = 80;
    int columnPaymentsWidth = 60;
    int columnRemainingWidth = 60;
    int columnTotalWidth = 80;
    int columnPassingWidth = 45;
    int columnPayeeWidth = 200;
    int columnHashWidth = 200;

    ui->tableWidgetVoting->setColumnWidth(0, columnNameWidth);
    ui->tableWidgetVoting->setColumnWidth(1, columnCreatedWidth);
    ui->tableWidgetVoting->setColumnWidth(2, columnStartWidth);
    ui->tableWidgetVoting->setColumnWidth(3, columnEndWidth);
    ui->tableWidgetVoting->setColumnWidth(4, columnYesWidth);
    ui->tableWidgetVoting->setColumnWidth(5, columnNoWidth);
    ui->tableWidgetVoting->setColumnWidth(6, columnAbstainWidth);
    ui->tableWidgetVoting->setColumnWidth(7, columnMonthlyWidth);
    ui->tableWidgetVoting->setColumnWidth(8, columnPaymentsWidth);
    ui->tableWidgetVoting->setColumnWidth(9, columnRemainingWidth);
    ui->tableWidgetVoting->setColumnWidth(10, columnTotalWidth);
    ui->tableWidgetVoting->setColumnWidth(11, columnPassingWidth);
    ui->tableWidgetVoting->setColumnWidth(12, columnPayeeWidth);
    ui->tableWidgetVoting->setColumnWidth(13, columnHashWidth);

    int columnAliasWidth = 100;
    int columnAddressWidth = 200;
    int columnProtocolWidth = 60;
    int columnStatusWidth = 80;
    int columnActiveWidth = 130;
    int columnLastSeenWidth = 130;

    ui->tableWidgetMyMasternodes->setColumnWidth(0, columnAliasWidth);
    ui->tableWidgetMyMasternodes->setColumnWidth(1, columnAddressWidth);
    ui->tableWidgetMyMasternodes->setColumnWidth(2, columnProtocolWidth);
    ui->tableWidgetMyMasternodes->setColumnWidth(3, columnStatusWidth);
    ui->tableWidgetMyMasternodes->setColumnWidth(4, columnActiveWidth);
    ui->tableWidgetMyMasternodes->setColumnWidth(5, columnLastSeenWidth);

    ui->tableWidgetMasternodes->setColumnWidth(0, columnAddressWidth);
    ui->tableWidgetMasternodes->setColumnWidth(1, columnProtocolWidth);
    ui->tableWidgetMasternodes->setColumnWidth(2, columnStatusWidth);
    ui->tableWidgetMasternodes->setColumnWidth(3, columnActiveWidth);
    ui->tableWidgetMasternodes->setColumnWidth(4, columnLastSeenWidth);

    ui->tableWidgetMyMasternodes->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *startAliasAction = new QAction(tr("Start alias"), this);
    contextMenu = new QMenu();
    contextMenu->addAction(startAliasAction);
    connect(ui->tableWidgetMyMasternodes, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(startAliasAction, SIGNAL(triggered()), this, SLOT(on_startButton_clicked()));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNodeList()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateVoteList()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateMyNodeList()));
    timer->start(1000);

    fFilterUpdated = false;
    nTimeFilterUpdated = GetTime();
    updateNodeList();
    updateVoteList();

    // Compute last/next superblock
    int nLastSuperblock, nNextSuperblock;

    // Get current block height
    int nBlockHeight = 0;
    {   
        LOCK(cs_main); 
        nBlockHeight = (int)chainActive.Height();
    }

    // Get chain parameters
    int nSuperblockStartBlock = Params().GetConsensus().nSuperblockStartBlock;
    int nSuperblockCycle = Params().GetConsensus().nSuperblockCycle;

    // Get first superblock
    int nFirstSuperblockOffset = (nSuperblockCycle - nSuperblockStartBlock % nSuperblockCycle) % nSuperblockCycle;
    int nFirstSuperblock = nSuperblockStartBlock + nFirstSuperblockOffset;

    if(nBlockHeight < nFirstSuperblock){
        nLastSuperblock = 0;
        nNextSuperblock = nFirstSuperblock;
    } else {
        nLastSuperblock = nBlockHeight - nBlockHeight % nSuperblockCycle;
        nNextSuperblock = nLastSuperblock + nSuperblockCycle;
    }
    ui->superblockLabel->setText(QString::number(nNextSuperblock));
}

MasternodeList::~MasternodeList()
{
    delete ui;
}

void MasternodeList::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model) {
        // try to update list when masternode count changes
        connect(clientModel, SIGNAL(strMasternodesChanged(QString)), this, SLOT(updateNodeList()));
    }
}

void MasternodeList::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void MasternodeList::showContextMenu(const QPoint &point)
{
    QTableWidgetItem *item = ui->tableWidgetMyMasternodes->itemAt(point);
    if(item) contextMenu->exec(QCursor::pos());
}

void MasternodeList::StartAlias(std::string strAlias)
{
    std::string strStatusHtml;
    strStatusHtml += "<center>Alias: " + strAlias;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        if(mne.getAlias() == strAlias) {
            std::string strError;
            CMasternodeBroadcast mnb;

            bool fSuccess = CMasternodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb);

            if(fSuccess) {
                strStatusHtml += "<br>Successfully started masternode.";
                mnodeman.UpdateMasternodeList(mnb, *g_connman);
                mnb.Relay(*g_connman);
                mnodeman.NotifyMasternodeUpdates(*g_connman);
            } else {
                strStatusHtml += "<br>Failed to start masternode.<br>Error: " + strError;
            }
            break;
        }
    }
    strStatusHtml += "</center>";

    QMessageBox msg;
    msg.setText(QString::fromStdString(strStatusHtml));
    msg.exec();

    updateMyNodeList(true);
}

void MasternodeList::StartAll(std::string strCommand)
{
    int nCountSuccessful = 0;
    int nCountFailed = 0;
    std::string strFailedHtml;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        std::string strError;
        CMasternodeBroadcast mnb;

        int32_t nOutputIndex = 0;
        if(!ParseInt32(mne.getOutputIndex(), &nOutputIndex)) {
            continue;
        }

        COutPoint outpoint = COutPoint(uint256S(mne.getTxHash()), nOutputIndex);

        if(strCommand == "start-missing" && mnodeman.Has(outpoint)) continue;

        bool fSuccess = CMasternodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb);

        if(fSuccess) {
            nCountSuccessful++;
            mnodeman.UpdateMasternodeList(mnb, *g_connman);
            mnb.Relay(*g_connman);
            mnodeman.NotifyMasternodeUpdates(*g_connman);
        } else {
            nCountFailed++;
            strFailedHtml += "\nFailed to start " + mne.getAlias() + ". Error: " + strError;
        }
    }
    pwalletMain->Lock();

    std::string returnObj;
    returnObj = strprintf("Successfully started %d masternodes, failed to start %d, total %d", nCountSuccessful, nCountFailed, nCountFailed + nCountSuccessful);
    if (nCountFailed > 0) {
        returnObj += strFailedHtml;
    }

    QMessageBox msg;
    msg.setText(QString::fromStdString(returnObj));
    msg.exec();

    updateMyNodeList(true);
}

void MasternodeList::updateMyMasternodeInfo(QString strAlias, QString strAddr, const COutPoint& outpoint)
{
    bool fOldRowFound = false;
    int nNewRow = 0;

    for(int i = 0; i < ui->tableWidgetMyMasternodes->rowCount(); i++) {
        if(ui->tableWidgetMyMasternodes->item(i, 0)->text() == strAlias) {
            fOldRowFound = true;
            nNewRow = i;
            break;
        }
    }

    if(nNewRow == 0 && !fOldRowFound) {
        nNewRow = ui->tableWidgetMyMasternodes->rowCount();
        ui->tableWidgetMyMasternodes->insertRow(nNewRow);
    }

    masternode_info_t infoMn;
    bool fFound = mnodeman.GetMasternodeInfo(outpoint, infoMn);

    QTableWidgetItem *aliasItem = new QTableWidgetItem(strAlias);
    QTableWidgetItem *addrItem = new QTableWidgetItem(fFound ? QString::fromStdString(infoMn.addr.ToString()) : strAddr);
    QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(fFound ? infoMn.nProtocolVersion : -1));
    QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(fFound ? CMasternode::StateToString(infoMn.nActiveState) : "MISSING"));
    QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(fFound ? (infoMn.nTimeLastPing - infoMn.sigTime) : 0)));
    QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M",
                                                                                                   fFound ? infoMn.nTimeLastPing + GetOffsetFromUtc() : 0)));
    QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(fFound ? CBitcoinAddress(infoMn.pubKeyCollateralAddress.GetID()).ToString() : ""));

    ui->tableWidgetMyMasternodes->setItem(nNewRow, 0, aliasItem);
    ui->tableWidgetMyMasternodes->setItem(nNewRow, 1, addrItem);
    ui->tableWidgetMyMasternodes->setItem(nNewRow, 2, protocolItem);
    ui->tableWidgetMyMasternodes->setItem(nNewRow, 3, statusItem);
    ui->tableWidgetMyMasternodes->setItem(nNewRow, 4, activeSecondsItem);
    ui->tableWidgetMyMasternodes->setItem(nNewRow, 5, lastSeenItem);
    ui->tableWidgetMyMasternodes->setItem(nNewRow, 6, pubkeyItem);
}

void MasternodeList::updateMyNodeList(bool fForce)
{
    TRY_LOCK(cs_mymnlist, fLockAcquired);
    if(!fLockAcquired) {
        return;
    }
    static int64_t nTimeMyListUpdated = 0;

    // automatically update my masternode list only once in MY_MASTERNODELIST_UPDATE_SECONDS seconds,
    // this update still can be triggered manually at any time via button click
    int64_t nSecondsTillUpdate = nTimeMyListUpdated + MY_MASTERNODELIST_UPDATE_SECONDS - GetTime();
    ui->secondsLabel->setText(QString::number(nSecondsTillUpdate));

    if(nSecondsTillUpdate > 0 && !fForce) return;
    nTimeMyListUpdated = GetTime();

    ui->tableWidgetMasternodes->setSortingEnabled(false);
    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        int32_t nOutputIndex = 0;
        if(!ParseInt32(mne.getOutputIndex(), &nOutputIndex)) {
            continue;
        }

        updateMyMasternodeInfo(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), COutPoint(uint256S(mne.getTxHash()), nOutputIndex));
    }
    ui->tableWidgetMasternodes->setSortingEnabled(true);

    // reset "timer"
    ui->secondsLabel->setText("0");
}

void MasternodeList::updateNodeList()
{
    TRY_LOCK(cs_mnlist, fLockAcquired);
    if(!fLockAcquired) {
        return;
    }

    static int64_t nTimeListUpdated = GetTime();

    // to prevent high cpu usage update only once in MASTERNODELIST_UPDATE_SECONDS seconds
    // or MASTERNODELIST_FILTER_COOLDOWN_SECONDS seconds after filter was last changed
    int64_t nSecondsToWait = fFilterUpdated
                            ? nTimeFilterUpdated - GetTime() + MASTERNODELIST_FILTER_COOLDOWN_SECONDS
                            : nTimeListUpdated - GetTime() + MASTERNODELIST_UPDATE_SECONDS;

    if(fFilterUpdated) ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", nSecondsToWait)));
    if(nSecondsToWait > 0) return;

    nTimeListUpdated = GetTime();
    fFilterUpdated = false;

    QString strToFilter;
    ui->countLabel->setText("Updating...");
    ui->tableWidgetMasternodes->setSortingEnabled(false);
    ui->tableWidgetMasternodes->clearContents();
    ui->tableWidgetMasternodes->setRowCount(0);
    std::map<COutPoint, CMasternode> mapMasternodes = mnodeman.GetFullMasternodeMap();
    int offsetFromUtc = GetOffsetFromUtc();

    for(auto& mnpair : mapMasternodes)
    {
        CMasternode mn = mnpair.second;
        // populate list
        // Address, Protocol, Status, Active Seconds, Last Seen, Pub Key
        QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(mn.addr.ToString()));
        QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(mn.nProtocolVersion));
        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(mn.GetStatus()));
        QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(mn.lastPing.sigTime - mn.sigTime)));
        QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", mn.lastPing.sigTime + offsetFromUtc)));
        QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(CBitcoinAddress(mn.pubKeyCollateralAddress.GetID()).ToString()));

        if (strCurrentFilter != "")
        {
            strToFilter =   addressItem->text() + " " +
                            protocolItem->text() + " " +
                            statusItem->text() + " " +
                            activeSecondsItem->text() + " " +
                            lastSeenItem->text() + " " +
                            pubkeyItem->text();
            if (!strToFilter.contains(strCurrentFilter)) continue;
        }

        ui->tableWidgetMasternodes->insertRow(0);
        ui->tableWidgetMasternodes->setItem(0, 0, addressItem);
        ui->tableWidgetMasternodes->setItem(0, 1, protocolItem);
        ui->tableWidgetMasternodes->setItem(0, 2, statusItem);
        ui->tableWidgetMasternodes->setItem(0, 3, activeSecondsItem);
        ui->tableWidgetMasternodes->setItem(0, 4, lastSeenItem);
        ui->tableWidgetMasternodes->setItem(0, 5, pubkeyItem);
    }

    ui->countLabel->setText(QString::number(ui->tableWidgetMasternodes->rowCount()));
    ui->tableWidgetMasternodes->setSortingEnabled(true);
}

void MasternodeList::on_filterLineEdit_textChanged(const QString &strFilterIn)
{
    strCurrentFilter = strFilterIn;
    nTimeFilterUpdated = GetTime();
    fFilterUpdated = true;
    ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", MASTERNODELIST_FILTER_COOLDOWN_SECONDS)));
}

void MasternodeList::on_startButton_clicked()
{
    std::string strAlias;
    {
        LOCK(cs_mymnlist);
        // Find selected node alias
        QItemSelectionModel* selectionModel = ui->tableWidgetMyMasternodes->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();

        if(selected.count() == 0) return;

        QModelIndex index = selected.at(0);
        int nSelectedRow = index.row();
        strAlias = ui->tableWidgetMyMasternodes->item(nSelectedRow, 0)->text().toStdString();
    }

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm masternode start"),
        tr("Are you sure you want to start masternode %1?").arg(QString::fromStdString(strAlias)),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if(!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAlias(strAlias);
        return;
    }

    StartAlias(strAlias);
}

void MasternodeList::on_startAllButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm all masternodes start"),
        tr("Are you sure you want to start ALL masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if(!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAll();
        return;
    }

    StartAll();
}

void MasternodeList::on_startMissingButton_clicked()
{

    if(!masternodeSync.IsMasternodeListSynced()) {
        QMessageBox::critical(this, tr("Command is not available right now"),
            tr("You can't use this command until masternode list is synced"));
        return;
    }

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this,
        tr("Confirm missing masternodes start"),
        tr("Are you sure you want to start MISSING masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if(!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAll("start-missing");
        return;
    }

    StartAll("start-missing");
}

void MasternodeList::on_tableWidgetMyMasternodes_itemSelectionChanged()
{
    if(ui->tableWidgetMyMasternodes->selectedItems().count() > 0) {
        ui->startButton->setEnabled(true);
    }
}

void MasternodeList::on_UpdateButton_clicked()
{
    updateMyNodeList(true);
}

void MasternodeList::on_UpdateVotesButton_clicked()
{
    updateVoteList(true);
}

void MasternodeList::updateVoteList(bool reset)
{

    static int64_t lastVoteListUpdate = 0;
    int nMnCount = mnodeman.CountEnabled();

    // CALCULATE THE MINUMUM VOTE COUNT REQUIRED FOR FULL SIGNAL

    int nAbsVoteReq;
    if(Params().NetworkIDString() == CBaseChainParams::MAIN)
    {
        nAbsVoteReq = std::max(Params().GetConsensus().nGovernanceMinQuorum, nMnCount / 10);
    }
    else
    {
        nAbsVoteReq = Params().GetConsensus().nGovernanceMinQuorum;
    }

    // automatically update my masternode list only once in MY_MASTERNODELIST_UPDATE_SECONDS seconds,
    // this update still can be triggered manually at any time via button click
    int64_t timeTillUpdate = lastVoteListUpdate + MY_MASTERNODELIST_UPDATE_SECONDS - GetTime();
    ui->voteSecondsLabel->setText(QString::number(timeTillUpdate));

    if(timeTillUpdate > 0 && !reset) return;
    lastVoteListUpdate = GetTime();

    QString strToFilter;
    ui->tableWidgetVoting->setSortingEnabled(false);
    ui->tableWidgetVoting->clearContents();
    ui->tableWidgetVoting->setRowCount(0);

    // Compute last/next superblock
    int nLastSuperblock, nNextSuperblock;

    // Get current block height
    int nBlockHeight = 0;
    {   
        LOCK(cs_main); 
        nBlockHeight = (int)chainActive.Height();
    }

    // Get chain parameters
    int nSuperblockStartBlock = Params().GetConsensus().nSuperblockStartBlock;
    int nSuperblockCycle = Params().GetConsensus().nSuperblockCycle;
    int nPowTargetSpacing = Params().GetConsensus().nPowTargetSpacing;

    // Get first superblock
    int nFirstSuperblockOffset = (nSuperblockCycle - nSuperblockStartBlock % nSuperblockCycle) % nSuperblockCycle;
    int nFirstSuperblock = nSuperblockStartBlock + nFirstSuperblockOffset;

    if(nBlockHeight < nFirstSuperblock){
        nLastSuperblock = 0;
        nNextSuperblock = nFirstSuperblock;
    } else {
        nLastSuperblock = nBlockHeight - nBlockHeight % nSuperblockCycle;
        nNextSuperblock = nLastSuperblock + nSuperblockCycle;
    }

    int64_t nNextTime = GetAdjustedTime() + ((nNextSuperblock - nBlockHeight) * nPowTargetSpacing);
    int64_t nSuperblockCycleSeconds = nSuperblockCycle * nPowTargetSpacing;
    int64_t nSuperblockValue = round((int64_t)CSuperblock::GetPaymentsLimit(nNextSuperblock) / 100000000);
    int64_t nTotalAllotted = 0;

    std::vector<CGovernanceObject*> objs = governance.GetAllNewerThan(0);
    BOOST_FOREACH(CGovernanceObject* pGovObj, objs)
    {
        if(!pGovObj->IsSetCachedValid()) continue;
        if(pGovObj->GetObjectType() != GOVERNANCE_OBJECT_PROPOSAL) continue;

        UniValue obj(UniValue::VOBJ);
        obj.read(pGovObj->GetDataAsString());
        std::vector<UniValue> arr1 = obj.getValues();
        std::vector<UniValue> arr2 = arr1.at(0).getValues();
        UniValue objJSON = arr2.at(1);

        int64_t nStartEpoch = 0;
        const UniValue uValueSE = objJSON["start_epoch"];
        switch(uValueSE.getType()) {
            case UniValue::VNUM:
                nStartEpoch = uValueSE.get_int64();
                break;
            default:
                std::istringstream ss(uValueSE.get_str());
                ss >> nStartEpoch;
                break;
        }

        int64_t nEndEpoch = 0;
        const UniValue uValueEE = objJSON["end_epoch"];
        switch(uValueEE.getType()) {
            case UniValue::VNUM:
                nEndEpoch = uValueEE.get_int64();
                break;
            default:
                std::istringstream ss(uValueEE.get_str());
                ss >> nEndEpoch;
                break;
        }

        int64_t nAmount = 0;
        const UniValue uValuePA = objJSON["payment_amount"];
        switch(uValuePA.getType()) {
            case UniValue::VNUM:
                nAmount = uValuePA.get_int64();
                break;
            default:
                std::istringstream ss(uValuePA.get_str());
                ss >> nAmount;
                break;
        }

        // Don't display past proposals
	if (nEndEpoch < nNextTime) continue;

        int nPayments = floor((nEndEpoch - nStartEpoch) / nSuperblockCycleSeconds);
        int nStart = nStartEpoch;
        if (GetAdjustedTime() > nStartEpoch) {
            nStart = GetAdjustedTime();
        }
        int nRemaining = nPayments - floor((nStart - nStartEpoch) / nSuperblockCycleSeconds);

        CBitcoinAddress address2(objJSON["payment_address"].get_str());

        // populate list
        QLabel *nameItem = new QLabel("<a href=\"" + QString::fromStdString(objJSON["url"].get_str()) + "\">" + QString::fromStdString(objJSON["name"].get_str()) + "</a>");
        nameItem->setOpenExternalLinks(true);
        nameItem->setStyleSheet("background-color: transparent; padding: 0 1em;");
        QTableWidgetItem *hashItem = new QTableWidgetItem(QString::fromStdString(pGovObj->GetHash().ToString()));
        QTableWidgetItem *blockCreatedItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d", (int64_t)pGovObj->GetCreationTime() + GetOffsetFromUtc())));
        QTableWidgetItem *blockStartItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d", nStartEpoch + GetOffsetFromUtc())));
        QTableWidgetItem *blockEndItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d", nEndEpoch + GetOffsetFromUtc())));
        QTableWidgetItem *yesVotesItem = new QTableWidgetItem(QString::number((int64_t)pGovObj->GetYesCount(VOTE_SIGNAL_FUNDING)));
        QTableWidgetItem *noVotesItem = new QTableWidgetItem(QString::number((int64_t)pGovObj->GetNoCount(VOTE_SIGNAL_FUNDING)));
        QTableWidgetItem *abstainVotesItem = new QTableWidgetItem(QString::number((int64_t)pGovObj->GetAbstainCount(VOTE_SIGNAL_FUNDING)));
        QTableWidgetItem *monthlyPaymentItem = new QTableWidgetItem(QString::number(nAmount));
        QTableWidgetItem *paymentsItem = new QTableWidgetItem(QString::number(nPayments));
        QTableWidgetItem *remainingPaymentsItem = new QTableWidgetItem(QString::number(nRemaining));
        QTableWidgetItem *totalPaymentItem = new QTableWidgetItem(QString::number(nAmount * nPayments));
        QTableWidgetItem *AddressItem = new QTableWidgetItem(QString::fromStdString(address2.ToString()));

        std::string projected;            
        if ((int64_t)pGovObj->GetAbsoluteYesCount(VOTE_SIGNAL_FUNDING) >= nAbsVoteReq){
            nTotalAllotted += nAmount;
            projected = "Yes";
        } else {
            projected = "No";
        }
        QTableWidgetItem *projectedItem = new QTableWidgetItem(QString::fromStdString(projected));

        ui->tableWidgetVoting->insertRow(0);
// FIXME if projected == No Red, Yes Green
        ui->tableWidgetVoting->setCellWidget(0, 0, nameItem);
        ui->tableWidgetVoting->setItem(0, 1, blockCreatedItem);
        ui->tableWidgetVoting->setItem(0, 2, blockStartItem);
        ui->tableWidgetVoting->setItem(0, 3, blockEndItem);
        ui->tableWidgetVoting->setItem(0, 4, yesVotesItem);
        ui->tableWidgetVoting->setItem(0, 5, noVotesItem);
        ui->tableWidgetVoting->setItem(0, 6, abstainVotesItem);
        ui->tableWidgetVoting->setItem(0, 7, monthlyPaymentItem);
        ui->tableWidgetVoting->setItem(0, 8, paymentsItem);
        ui->tableWidgetVoting->setItem(0, 9, remainingPaymentsItem);
        ui->tableWidgetVoting->setItem(0, 10, totalPaymentItem);
        ui->tableWidgetVoting->setItem(0, 11, projectedItem);
        ui->tableWidgetVoting->setItem(0, 12, AddressItem);
        ui->tableWidgetVoting->setItem(0, 13, hashItem);
    }

    ui->superblockLabel->setText(QString::number(nNextSuperblock));
    ui->totalAllottedLabel->setText(QString::number(nTotalAllotted) + " of " + QString::number(nSuperblockValue));
    ui->tableWidgetVoting->setSortingEnabled(true);

    // reset "timer"
    ui->voteSecondsLabel->setText("0");
}

void MasternodeList::VoteMany(std::string strCommand)
{
    // Find selected Budget Hash
    QItemSelectionModel* selectionModel = ui->tableWidgetVoting->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();
    if(selected.count() == 0)
        return;

    QModelIndex index = selected.at(0);
    int r = index.row();
    std::string strHash = ui->tableWidgetVoting->item(r, 2)->text().toStdString();
    uint256 hash;
    hash.SetHex(strHash);

    vote_signal_enum_t eVoteSignal = CGovernanceVoting::ConvertVoteSignal("funding");
    if(eVoteSignal == VOTE_SIGNAL_NONE) {
        // "Invalid vote signal. Please using one of the following: (funding|valid|delete|endorsed) OR `custom sentinel code`"
    }

    vote_outcome_enum_t eVoteOutcome = CGovernanceVoting::ConvertVoteOutcome(strCommand);
    if(eVoteOutcome == VOTE_OUTCOME_NONE) {
        // "Invalid vote outcome. Please use one of the following: 'yes', 'no' or 'abstain'"
    }

    int nSuccessful = 0;
    int nFailed = 0;
    std::string statusObj;

    std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
    mnEntries = masternodeConfig.getEntries();

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        std::string strError;
        std::vector<unsigned char> vchMasterNodeSignature;
        std::string strMasterNodeSignMessage;

        CPubKey pubKeyCollateralAddress;
        CKey keyCollateralAddress;
        CPubKey pubKeyMasternode;
        CKey keyMasternode;

        if(!CMessageSigner::GetKeysFromSecret(mne.getPrivKey(), keyMasternode, pubKeyMasternode)){
            nFailed++;
            statusObj += "\nFailed to vote with " + mne.getAlias() + ". Masternode signing error, could not set key correctly: " + statusObj;
            continue;
        }

        uint256 nTxHash;
        nTxHash.SetHex(mne.getTxHash());

        int32_t nOutputIndex = 0;
        if(!ParseInt32(mne.getOutputIndex(), &nOutputIndex)) {
            continue;
        }

        COutPoint outpoint(nTxHash, nOutputIndex);

        CMasternode mn;
        bool fMnFound = mnodeman.Get(outpoint, mn);

        if(!fMnFound) {
            nFailed++;
            statusObj += "\nFailed to find masternode " + mne.getAlias() + " by collateral output. Error: " + statusObj;
            continue;
        }

        CGovernanceVote vote(mn.vin.prevout, hash, eVoteSignal, eVoteOutcome);
        if(!vote.Sign(keyMasternode, pubKeyMasternode)){
            nFailed++;
            statusObj += "\nFailed to vote with " + mne.getAlias() + ". Error: Failure to sign";
            continue;
        }

        CGovernanceException exception;
        if(governance.ProcessVoteAndRelay(vote, exception, *g_connman)) {
            nSuccessful++;
        }
        else {
            nFailed++;
            statusObj += "\nFailed to vote on proposal. Error: " + exception.GetMessage();
        }
    }
    std::string returnObj;
    returnObj = strprintf("Voted successfully %d time(s) and failed %d time(s).", nSuccessful, nFailed);
    if (nFailed > 0)
        returnObj += statusObj;

    QMessageBox msg;
    msg.setText(QString::fromStdString(returnObj));
    msg.exec();
    updateVoteList(true);
}

void MasternodeList::on_voteManyYesButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm vote-many"),
        tr("Are you sure you want to vote with ALL of your masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        VoteMany("yes");
        return;
    }

    VoteMany("yes");
}

void MasternodeList::on_voteManyNoButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm vote-many"),
        tr("Are you sure you want to vote with ALL of your masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        VoteMany("no");
        return;
    }

    VoteMany("no");
}

void MasternodeList::on_voteManyAbstainButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm vote-many"),
        tr("Are you sure you want to vote with ALL of your masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        VoteMany("abstain");
        return;
    }

    VoteMany("abstain");
}

void MasternodeList::on_tableWidgetVoting_itemSelectionChanged()
{
    if(ui->tableWidgetVoting->selectedItems().count() > 0)
    {
        ui->voteManyYesButton->setEnabled(true);
        ui->voteManyNoButton->setEnabled(true);
    }
}
