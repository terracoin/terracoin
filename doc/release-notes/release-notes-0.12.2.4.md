Terracoin Core version 0.12.2 is now available from:

  <https://terracoin.io/bin/terracoin-core-0.12.2.4/>

This is a new minor release, updated to the terracoin 0.12.2.x branch. This includes ignoring all nodes lower then 0.12.1.7, and the problematic 0.12.2.3.  Enabling concensus for BIP 68, 112, 113, DIP 1, and finally enforcring V4 blocks.

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

Add ability to ignore based on client version
---------------------------------------------

This allows us to forward with softforks a little bit cleaner.  Previously this was done with protocol version only, now we have the change to do it with a few different options.

BIP 68, 112, 113
----------------

BIPs that should have already been enabled on your network will now start the concensus count to lock it in.


DIP 0001
--------

DIP 0001 will not start the concensus to lock it in, this will bring 2 MB blocks and 10 times lower fees.

V4 Blocks
---------

We have accepted V4 blocks for a while, but it wasn't enforced in the code, causing inconsistencies int he BIPs we already have locked in, These will not start a 950 block lock in, making older client vulnerable to accepting a V2/3 block and thus splitting, please update ASAP.

0.12.2.4 Change log
===================

See detailed [change log](https://github.com/terracoin/terracoin/compare/v0.12.2.3...terracoin:v0.12.2.4) below.

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

- [v0.12.2.3](release-notes/release-notes-0.12.2.3.md) released Jun/28/2018
- [v0.12.1.8](release-notes/release-notes-0.12.1.8.md) released Feb/28/2018
- [v0.12.1.7](release-notes/release-notes-0.12.1.7.md) released Feb/24/2018
- [v0.12.1.6](release-notes/release-notes-0.12.1.6.md) released Jan/15/2018
- [v0.12.1.5](release-notes/release-notes-0.12.1.5.md) released Sep/21/2017
