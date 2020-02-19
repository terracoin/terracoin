Terracoin Core version 0.12.2 is now available from:

  <https://terracoin.io/bin/terracoin-core-0.12.2.5/>

This is a new minor release. This includes hard coding a ban on Terracon Core 0.12.2.3, other are based on protocol verison with was increased to 70208. Added a gitian definition for arm builds. Updated the seeds. Removed qt4 from the configure script since 5 is required. Also contains various bugfixes and other improvements.


Please report bugs using the issue tracker at github:

  <https://github.com/terracoin/terracoin/issues>

Upgrading and downgrading
=========================

How to Upgrade
--------------

Please see your [wiki]<https://wiki.terracoin.io/view/Upgrading_the_Core_Wallet>

Downgrade warning
-----------------

### Downgrade to a version < 0.12.2.3

Because release 0.12.2.3 includes the per-UTXO fix which changes the structure of the internal database, this release is not fully backwards compatible. You will have to reindex the database if you decide to use any previous version.

This does not affect wallet forward or backward compatibility.

Notable changes
===============

Remove ability to ignore based on client version
------------------------------------------------

This was a bad assumption.  Since clients can define there own version string including a version it's not a good idea to ban based on this.  We did not have any such nodes and as such was a bad assumption in 0.12.2.4, A Hard coded ban on 0.12.2.3 was made and all other banning was returned to how it was pre 0.12.2.4.

RPC
---

Added sort to `masternode outputs` via txid tiemstamp in ascending order.

Added optional sub command `full` to `masternode outputs` which adds the address the txid is from.

Fixes
-----

Fixed how the total and reminaing payments are calculated on the proposals tab.

Changed the term `Monthly` to `Payment` on the proposals tab since it was presumptuous.
 
TestNet
-------

Changed tested from 1 hour superblocks to 30 hours.  Was too hard to test with superblocks going so fast.

Testnet has been reset and only 0.12.2.5+ is allowed on it.

Build System
------------

Remove qt4 as an option, qt5 is required.  Add arm options for gitian.

0.12.2.5 Change log
===================

See detailed [change log](https://github.com/terracoin/terracoin/compare/v0.12.2.4...terracoin:v0.12.2.5) below.

Credits
=======

Thanks to everyone who directly contributed to this release:

- TheSin

- All the testers

As well as Bitcoin Core Developers, Dash Core Developers, Crown Core Developers and everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/terracoin/).

Older releases
==============

Terracoin Core tree 0.12.1.x was a fork of Dash Core tree 0.12.1.x

Terracoin Core tree 0.12.2.x was a fork of Dash Core tree 0.12.2.x

These release are considered obsolete. Old release notes can be found here:

- [v0.12.2.4](release-notes/release-notes-0.12.2.4.md) released Jul/19/2018
- [v0.12.2.3](release-notes/release-notes-0.12.2.3.md) released Jun/28/2018
- [v0.12.1.8](release-notes/release-notes-0.12.1.8.md) released Feb/28/2018
- [v0.12.1.7](release-notes/release-notes-0.12.1.7.md) released Feb/24/2018
- [v0.12.1.6](release-notes/release-notes-0.12.1.6.md) released Jan/15/2018
- [v0.12.1.5](release-notes/release-notes-0.12.1.5.md) released Sep/21/2017
