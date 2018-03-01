Bitcoin Core version 0.12.1 is now available from:

  <https://terracoin.io/bin/terracoin-core-0.12.1.8/>

This is a new minor version release, fixed a bug in preparing proposals which
was introduced in 0.12.1.7, all masternode owners are encouraged to update.

Please report bugs using the issue tracker at github:

  <https://github.com/terracoin/terracoin/issues>

Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Terracoin Core (on Mac)
or terracoind/terracoin-qt (on Linux).

Downgrade warning
-----------------

### Downgrade to a version < 0.12.0

Because release 0.12.0 and later will obfuscate the chainstate on every
fresh sync or reindex, the chainstate is not backwards-compatible with
pre-0.12 versions of Terracoin Core or other software.

If you want to downgrade after you have done a reindex with 0.12.0 or later,
you will need to reindex when you first start Terracoin Core version 0.11 or
earlier.

Notable changes
===============

Proposals
---------

A bug was introduced in 0.12.1.7 which stopped the ability to track the tx
confirmations of a proposal, so it could not get submitted.

0.12.1.8 Change log
===================

Detailed release notes follow. This overview includes changes that affect
behaviour, not code moves, refactors and string updates. For convenience in
locating the code changes and accompanying discussion, both the pull request and
git merge commit are mentioned.

### Proposals
- `ae4d4fe` Fix proposal tx confirmation amounts (thesin)

Credits
=======

Thanks to everyone who directly contributed to this release:

- TheSin
- sixoffive

As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/terracoin/).
