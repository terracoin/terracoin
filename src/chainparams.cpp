// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Copyright (c) 2017-2018 The Terracoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "June 4th 1978 - March 6th 2009 ; Rest In Peace, Stephanie.";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */


class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 1050000;
        consensus.nMasternodePaymentsStartBlock = 100000; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 158000; // actual historical value
        consensus.nMasternodePaymentsIncreasePeriod = 576*30; // 17280 - actual historical value
        consensus.nInstantSendKeepLock = 24;
        consensus.nSuperblockStartBlock = 1087500;
        consensus.nSuperblockCycle = 21600; // ~(30*24*60)/2, 30 days
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 95980;
        consensus.BIP34Hash = uint256S("0x0000000000006a908847f2d6b7ac98e8ac9ce54c544aca63c66473e637f4741e");
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 60 * 60;
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPowEMAHeight = 101633;
        consensus.nPowV2Height = 181202;
        consensus.nPowKGWHeight = 833001; // nPowKGWHeight >= nPowDGWHeight means "no KGW"
        consensus.nPowDGWHeight = 833001;
        consensus.nRuleChangeActivationThreshold = 90; // 300% of nMinerConfirmationWindow
        consensus.nMinerConfirmationWindow = 30; // nPowTargetTimespan / nPowTargetSpacing

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 0; // Not yet enabled

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1530403200; // Jul 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1561939200; // Jul 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 3226; // 80% of 4032

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000013c31ec956b02ded0e535c"); // 1270000

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0xb40f66a62b2802614fe4941e1abbb6b112d7b3945d5c09ef6a1932c1d0a5585e"); // 1210000

        consensus.nAuxpowChainId = 0x0032;
        consensus.nAuxpowStartHeight = 833000;
        consensus.fStrictChainId = true;
        consensus.nLegacyBlocksBefore = 833000;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x42;
        pchMessageStart[1] = 0xba;
        pchMessageStart[2] = 0xbe;
        pchMessageStart[3] = 0x56;
        //vAlertPubKey = ParseHex("048240a8748a80a286b270ba126705ced4f2ce5a7847b3610ea3c06513150dade2a8512ed5ea86320824683fc0818f0ac019214973e677acd1244f6d0571fc5103");
        nDefaultPort = 13333;
        nMaxTipAge = 144 * 2 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1351242683, 2820375594, 0x1d00ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000000804bbc6a621a9dbb564ce469f492e1ccf2d70f8a6b241e26a277afa2"));
        assert(genesis.hashMerkleRoot == uint256S("0x0f8b09f93803b067580c16c3f3a6aaa901be06ad892cea9f02d8a4f93628f196"));

        vSeeds.push_back(CDNSSeedData("terracoin.io", "seed.terracoin.io"));
        vSeeds.push_back(CDNSSeedData("southofheaven.ca", "dnsseed.southofheaven.ca"));

        // Terracoin addresses start with '1' (Bitcoin defaults)
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        // Terracoin script addresses start with '3' (Bitcoin defaults)
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        // Terracoin private keys start with '5' or 'K' or 'LL' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        // Terracoin BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        // Terracoin BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // Terracoin BIP44 coin type
        // https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        nExtCoinType = 83;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour
        strSporkPubKey = "02f1b4c2d95dee0f02de365173ed859b8604f9ce3653ef1f9c7d4723a2b3458b30";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (    0, uint256S("0x00000000804bbc6a621a9dbb564ce469f492e1ccf2d70f8a6b241e26a277afa2"))
            (    1, uint256S("0x000000000fbc1c60a7610c894f98d102390e9e00cc18caced4eb4198ec0c3645"))
            (   31, uint256S("0x00000000045341e3ebdfa180e4a0f1e4da23829609517a3673b4a796714a7593"))
            (  104, uint256S("0x000000006afe30806352f2015829527dd91f19fbc2d28f799c3ad61c37746fdb"))
            (  355, uint256S("0x0000000000c0e178ddd6a8f15724f37470428c233883c302267299b21fb5d237"))
            (  721, uint256S("0x00000000027671a417f3d3eafac6d150f0b2ccba37f0b63f75bc657ec25950ed"))
            ( 2000, uint256S("0x000000000283ecd683fe30d4542929a78873df7818029793f84e9c65dc337d94"))
            ( 3000, uint256S("0x0000000000317b69ff2a56284442fede7ffa66f75d000f1cf34171ad671db4b6"))
            ( 4000, uint256S("0x00000000001190cad5d66d028b6afcf22db58d2b5c17abf2bf2e1353be13097d"))
            ( 4255, uint256S("0x000000000018b3ba5b241f3e88b4a88e580f9e7dd0fc7fc786ff22c586e53dd9"))
            ( 5631, uint256S("0x00000000001243509866938e344c0010bd88b156da27ef9d707cb1f1698f2a32"))
            ( 6000, uint256S("0x00000000000c1abfd7c29d07e23ef52631c6874e42e6a240a4d3678d9c716d81"))
            ( 7395, uint256S("0x0000000000005d8e1281d6b28fe6b504ab81e7e3ec561e97b7a98973e449f7fb"))
            ( 8001, uint256S("0x00000000001ca63707536b6dfc8ba4c5aa7fce19b6295ef73e195554cdb92d44"))
            ( 8157, uint256S("0x00000000001303998da2714abf02159f1421103e77fee3b876d69c0fa7b108d9"))
            (12311, uint256S("0x00000000002b5708fefdceb5db42cd2135eaf23f23a95c285e8f310262f8d639"))
            (13224, uint256S("0x00000000000765c69f777ccbc44fab23edab9126f1b4ec5078450aebc3809c36"))
            (14401, uint256S("0x0000000000388541b88c57883a480fd6cfa7b93f68e0c71538b08b2c4d875fa2"))
            (15238, uint256S("0x000000000000ae09d46bc1a9c5ca4c7b5e51bf23d3108926daa140d64355d390"))
            (18426, uint256S("0x00000000001fb1c075dbfa27f7ba83928a2d35152ccae59c742f698fb8cb8108"))
            (19114, uint256S("0x000000000022224f57adecffdc8f5cffb3b932838310aefdd12aaabcc6122259"))
            (20124, uint256S("0x00000000002374e0e1fcee28b520dff3f3e86cb7ff7afdea112aaf2002101900"))
            (21711, uint256S("0x00000000002189b0ae139b8c449bbcb99d520b8a798b7e0f0ff8ab8539e9bcb4"))
            (22100, uint256S("0x0000000000142ace7d8db003da69191896986c5564e604e3100d14c17daaf90f"))
            (22566, uint256S("0x00000000000c28dc052d277d18e104e4e63c53e4018273b3af49f772af205d43"))
            (24076, uint256S("0x00000000000522702c7f0ee6fa2ce3978cc0f3056ecc73d8cde1035891c5c4d5"))
            (25372, uint256S("0x0000000000202428a01ad6b8a2e256162b5bba35efd1e7f45dc42dbf5861f784"))
            (25538, uint256S("0x00000000002faa5586493e3821d90482348b0462d625d03d086a4a2a2301f6ef"))
            (26814, uint256S("0x0000000000179b0ed2a7d4390ff2c6213f1c522790b4e084a4e2036b53e6765b"))
            (28326, uint256S("0x0000000000141af96e4ab491d6534a6740491d62799b1669418d33bb007acfd7"))
            (28951, uint256S("0x00000000002404c991c7f9e1d641e8938f1ab704a3f9e1d22589d817948a202f"))
            (30765, uint256S("0x00000000004be710d96035855f406c1393c922d5948d82dce494368e3993c76a"))
            (32122, uint256S("0x0000000000109367e18d9c9762cdb8bfb3c524628ad2ce9bcb02a6a9bfa13e39"))
            (33526, uint256S("0x0000000000406137d7afb03df768f0c6d7ab1efdbc14325d018b62b7ef17fa1c"))
            (34310, uint256S("0x0000000000325205f89b7685639fc997248cbaf8dbf2d53ad2e00f98948de58c"))
            (35277, uint256S("0x00000000000214b28bc7b1ea230417442417d2381673bcec4ac3ef87c6d0b9f8"))
            (36341, uint256S("0x0000000000143a6fd244037ebf301c6898f34c6db428916057eadddec4e35061"))
            (51013, uint256S("0x00000000002afdd383affb15708fd329feaa68fdabe477bce4eceec7525dc7f2"))
            (63421, uint256S("0x000000000020867e3050e7f4b4402c578215a2174723f614d31d5f21eb61a173"))
            (67755, uint256S("0x00000000001151bbacf6f169312aaa0c71e0f71765ceb4e8ebffabc219993ba7"))
            (69198, uint256S("0x000000000041498e3911ccbd9aee327bb9bc58dcdf4cd51956909ce5575fccf5"))
            (71945, uint256S("0x00000000006d1cb2ad6700614c54a962bbe7d6baeb02c227b9dda2e0a59077da"))
            (74654, uint256S("0x00000000001b274b44531f13d5a023ae0450e765e403893c64aea7c6c21eec8e"))
            (77505, uint256S("0x00000000003be70f239212ba753a1fdd985fee027e276556619eeb40a771c151"))
            (95971, uint256S("0x000000000009d877e435995db1565558e30a7f8ee220cad5d0a4a055d0ebb8bf"))
            (106681, uint256S("0x000000000000d081d43eb74429e7344f18c4300796faaa54f5432c4976a9fc2b"))
            (106685, uint256S("0x00000000000017e88ad9cf01e74941a134434bf0c39ef254498e4cbb754b604f"))
            (106693, uint256S("0x00000000000271eb870d1573f3c1e1ba4c33a56f80333b89219d8affb2a8dadc"))
            (106715, uint256S("0x0000000000012d109bfe2a5b464defb0214b5b25090acb9cc0fd9ca054c53d6a"))
            (106725, uint256S("0x000000000007c734700d0be1f0c2358f57a009752257da8dcc2206185e9a580a"))
            (106748, uint256S("0x000000000000148fc833528722f41af106c08ed2da67e777f5a6b8ec2d60d695"))
            (106753, uint256S("0x0000000000003cb9976d3785c2960728bdd67d4ddfce640682b9c9c4df8582d9"))
            (106759, uint256S("0x000000000002ed231df0f8c4f21f2a73eeacbbc88f411b438ad4fd4b60418c42"))
            (107368, uint256S("0x000000000009062fe1b4c0654b3d152282545aa8beba3c0e4981d9dfa71b1eaa"))
            (108106, uint256S("0x00000000000282bf4e2bd0571c42165a67ffede3b81f2387e301369162107020"))
            (110197, uint256S("0x0000000000002d863064910c8964f5d8e2883aca9760c19368fe043263e2bfdd"))
            (137162, uint256S("0x00000000000342fd6e38765cc6f8f56d60c49e3e9522a54d99f561d35800a293"))
            (175950, uint256S("0x00000000000099c20a1fab3ace0421aa7038ca4fef541211aea8ed458b70a930"))
            (176570, uint256S("0x000000000000144a313379e6b76827b7959fb79322ebfd2bd9bb1ec04b8a015c"))
            (187891, uint256S("0x00000000000081f7748611af5ce18f40ab8affcabe39d1c68a038ecc0eb04310"))
            (191101, uint256S("0x000000000000250376b9fca7d353be9de6cc6266368d84e9bb8ab7ed06a4af27"))
            (263883, uint256S("0x0000000000000301007ab6d4ecb5cc223e80fb7e057d42f6963ba95b3b45981f"))
            (318300, uint256S("0x0000000000000bed2da6a46a698932caacf7fc164767b13c283b246c06440fb5"))
            (346805, uint256S("0x0000000000001054a0e53771c81dc9d057f8b2e6fcbf2069634807b89124c4f9"))
            (400000, uint256S("0x00000000000004dff66d12ca7b513edad65ef4e168a18f0ab42c24fafa849685"))
            (500000, uint256S("0x000000000000043df935dd5cc3ba642d383a14abff1a43a87be250571886a23f"))
            (600000, uint256S("0x000000000000057f583b956d0d2e16d6375aa5454245e422c5839993829e4c82"))
            (700000, uint256S("0x0000000000001a2eaa3708fa024c487b5b8aa5953ddd613eae14f0fd1a097fef"))
            (800000, uint256S("0x000000000000046cbd6582c6edf59c2aef9947c817a23e7f4ecff5d022fc0f8e"))
            (833000, uint256S("0x6fd7b973fa762ea35dc214a46276f95970c4260b251cb3e1e3c9337096b93f6d"))
            (900000, uint256S("0x6b9457bc395353eb5f08a88229125f3f30872e0771c5bbd5436beb950e20571a"))
            (1000000, uint256S("0x003bdc5e722fda8bb52ff1f54b3fe4896bed0708274ef787de6209d6817b7edd"))
            (1087500, uint256S("0x5b03f9206287debfdfa60496481da4b994d7d3a3a7264f2dd22c8e5c9cf443a1"))
            (1210000, uint256S("0xb40f66a62b2802614fe4941e1abbb6b112d7b3945d5c09ef6a1932c1d0a5585e"))
            (1260375, uint256S("0x470cc5f7e82d4169d8ec8ba09bc5f9cd20192168075a3c330232d85e311b913d"))
            (1260376, uint256S("0xc101914bd67438b2f37a0ca888a9007af608f16f15cf1187dff6890db7f9eba0"))
            (1270000, uint256S("0x583b3b08d02c7004ba5738de5900c382e7225def47bd3a3ad9c245d0131fdca1")),
            1506940111, // * UNIX timestamp of last checkpoint block
            1656788,    // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            1000        // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 1050000;
        consensus.nMasternodePaymentsStartBlock = 4010; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 4030;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendKeepLock = 6;
        consensus.nSuperblockStartBlock = 4200; // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPeymentsStartBlock
        consensus.nSuperblockCycle = 24; // Superblocks can be issued hourly on testnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x57e446ce39f87a0949e7400db06b1e2e1680fe4bc4621db0af04b5ecabb92abd");
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nPowEMAHeight = 4001;
        consensus.nPowV2Height = 4001;
        consensus.nPowKGWHeight = 4001; // nPowKGWHeight >= nPowDGWHeight means "no KGW"
        consensus.nPowDGWHeight = 4001;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1506556800; // September 28th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1530403200; // Jul 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1561939200; // Jul 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 50; // 50% of 100

        consensus.nAuxpowStartHeight = 0;
        consensus.nAuxpowChainId = 0x0001;
        consensus.fStrictChainId = false;
        consensus.nLegacyBlocksBefore = -1;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000001a9c85200164b"); // 4001

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x1ff88eb8ac889806543a831aecdf2e2b59126ede116b406f24df6a4e12a7d622"); // 4001

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;
        //vAlertPubKey = ParseHex("04517d8a699cb43d3938d7b24faaff7cda448ca4ea267723ba614784de661949bf632d6304316b244646dea079735b9a6fc4af804efb4752075b9fe2245e14e412");
        nDefaultPort = 18321;
        nMaxTipAge = 0x7fffffff; // allow mining on top of old blocks for testnet
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1502818099, 3483568824, 0x1d00ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000000a48f093611895d7452e456b646d213d238e86dc2c0db7d15fe6c555d"));
        assert(genesis.hashMerkleRoot == uint256S("0x0f8b09f93803b067580c16c3f3a6aaa901be06ad892cea9f02d8a4f93628f196"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("terracoin.io", "testnetseed.terracoin.io"));

        // Testnet Terracoin addresses start with 'm' or 'n' (Bitcoin defaults)
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        // Testnet Terracoin script addresses start with '2' (Bitcoin defaults)
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet Terracoin BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet Terracoin BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Testnet Terracoin BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour
        strSporkPubKey = "02ba07bdd2ec80a1836102c4a496f6e6e09cb969aa69e98b727040b4d96a382972";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (    0, uint256S("0x00000000a48f093611895d7452e456b646d213d238e86dc2c0db7d15fe6c555d"))
            (    6, uint256S("0x3bfc2e6b4a2e6edb1db9e1fe5aff1e3ec6d6bd8933d794cabfaea3efd97b0d45"))
            ( 4001, uint256S("0x1ff88eb8ac889806543a831aecdf2e2b59126ede116b406f24df6a4e12a7d622")),
            1502818099,
            0,
            1000.0
        };

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMasternodePaymentsStartBlock = 240;
        consensus.nMasternodePaymentsIncreaseBlock = 350;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendKeepLock = 6;
        consensus.nSuperblockStartBlock = 1500;
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nPowEMAHeight = 101631; // same as mainnet
        consensus.nPowV2Height = 181200; // same as mainnet
        consensus.nPowKGWHeight = 833000; // same as mainnet
        consensus.nPowDGWHeight = 833000; // same as mainnet
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 999999999999ULL;

        consensus.nAuxpowStartHeight = 0;
        consensus.nAuxpowChainId = 0x0001;
        consensus.fStrictChainId = true;
        consensus.nLegacyBlocksBefore = 0;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 0; // never delay GETHEADERS in regtests
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 2, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x5424a2e7fce169e6b9841d0b543a9c53ba111f3b31921b9fd768baa8ead4a8d2"));
        assert(genesis.hashMerkleRoot == uint256S("0x0f8b09f93803b067580c16c3f3a6aaa901be06ad892cea9f02d8a4f93628f196"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x5424a2e7fce169e6b9841d0b543a9c53ba111f3b31921b9fd768baa8ead4a8d2")),
            0,
            0,
            0
        };
        // Regtest Terracoin addresses start with 'm' or 'n' (Bitcoin defaults)
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        // Regtest Terracoin script addresses start with '2' (Bitcoin defaults)
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest Terracoin BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Regtest Terracoin BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Regtest Terracoin BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;
   }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
