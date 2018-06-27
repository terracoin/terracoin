Terracoin Core version 0.12.2 is now available from:

  <https://terracoin.io/bin/terracoin-core-0.12.2.3/>

This is a new major release, updated to the dash 0.12.2.x branch. This includes many updates to masternodes, InstantSend, PrivateSend. Also includes the first release of the new in client (Qt) proposal list, voting and creation tools. Aswell as the first update notification with download option (Install available about linux masternodes via RPC calls). `getblocktemplate` (gbt) has made a return in this version to help miners out. Everyone is encouraged to update.

Please report bugs using the issue tracker at github:

  <https://github.com/terracoin/terracoin/issues>

Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely shut down (which might take a few minutes for older versions), then run the installer (on Windows) or just copy over /Applications/Terracoin Core (on Mac) or terracoind/terracoin-qt (on Linux). Because of the per-UTXO fix (see below) there is a one-time database upgrade operation, so expect a slightly longer startup time on the first run.

Downgrade warning
-----------------

### Downgrade to a version < 0.12.2.2

Because release 0.12.2.2 includes the per-UTXO fix (see below) which changes the structure of the internal database, this release is not fully backwards compatible. You will have to reindex the database if you decide to use any previous version.

This does not affect wallet forward or backward compatibility.

Notable changes
===============

Per-UTXO fix
------------

This fixes a potential vulnerability, so called 'Corebleed', which was demonstrated this summer at the Вrеаkіng Віtсоіn Соnfеrеnсе іn Раrіs. The DoS can cause nodes to allocate excessive amounts of memory, which leads them to a halt. You can read more about the fix in the original Bitcoin Core pull request https://github.com/bitcoin/bitcoin/pull/10195

To fix this issue in Terracoin Core however, we had to backport a lot of other improvements from Bitcoin Core, see full list of backports in the detailed change log below.

Proposals Tab
-------------

For masternode users you will see two new tabs in the masternode area.  Proposal list where you can see more info (in the client if it's on services, or in your browser if not).  And you can vote right in the client.

Also you'll be able to create new proposals right in the client.  We still recommend that you start with a discovery first and you will be able to link the discovery to the proposal on services.


Updater
-------

As of version 0.12.2.3 and update detector has been added with an option in preferences to enable a check on startup.  It will notify you if you client is out of date and allow you to download the newer version with an SHA256 sum check built in for your protection, or you can click to open the download are in your browser.

In addition on linux masternodes there are new rpc calls, `update check`, `update install`, `update status` and `update stop`. you can use the rpc calls to make a cronjob to always keep your masternode up to date.

Additional indexes fix
----------------------

If you were using additional indexes like `addressindex`, `spentindex` or `timestampindex` it's possible that they are not accurate. Please consider reindexing the database by starting your node with `-reindex` command line option. This is a one-time operation, the issue should be fixed now.

InstantSend fix
---------------

InstantSend should work with multisig addresses now.

PrivateSend fix
---------------

Some internal data structures were not cleared properly, which could lead to a slightly higher memory consumption over a long period of time. This was a minor issue which was not affecting mixing speed or user privacy in any way.

Removal of support for local masternodes
----------------------------------------

Keeping a wallet with 5000 TRC unlocked for 24/7 is definitely not a good idea anymore. Because of this fact, it's also no longer reasonable to update and test this feature, so it's completely removed now. If for some reason you were still using it, please follow one of the guides and setup a remote masternode instead.This is currently a warning but it will become an error in the next release.

Sporks
------

Added MASTERNODE_PAY_PROTO_MIN as a spork, it will determind the minimum protocol version a masternode must have to get a payment.  This always us to put timeouts on older protocols.

Other improvements and bug fixes
--------------------------------

As a result of previous intensive refactoring and some additional fixes, it should be possible to compile Terracoin Core with `--disable-wallet` option now.

This release also improves sync process and significantly lowers the time after which `getblocktemplate` rpc becomes available on node start.

Updated our BIP32, BIP44, BIP45 CoinType properly set to 83 now.

And as usual, various small bugs and typos were fixed and more refactoring was done too.

0.12.2.3 Change log
===================

See detailed [change log](https://github.com/terracoin/terracoin/compare/v0.12.1.8...terracoin:v0.12.2.3) below.

Credits
=======

Thanks to everyone who directly contributed to this release:

- TheSin
- sixoffive

- All the testers

As well as Bitcoin Core Developers, Dash Core Developers, Crown Core Developers and everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/terracoin/).

Older releases
==============

Terracoin Core tree 0.12.1.x was a fork of Dash Core tree 0.12.1.x

Terracoin Core tree 0.12.2.x was a fork of Dash Core tree 0.12.2.x

These release are considered obsolete. Old release notes can be found here:

- [v0.12.1.8](release-notes/release-notes-0.12.1.8.md) released Feb/28/2018
- [v0.12.1.7](release-notes/release-notes-0.12.1.7.md) released Feb/24/2018
- [v0.12.1.6](release-notes/release-notes-0.12.1.6.md) released Jan/15/2018
- [v0.12.1.5](release-notes/release-notes-0.12.1.5.md) released Sep/21/2017
