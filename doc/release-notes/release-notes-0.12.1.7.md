Bitcoin Core version 0.12.1 is now available from:

  <https://terracoin.io/bin/terracoin-core-0.12.1.7/>

This is a new minor version release, including adding a wallet-utility binary,
insight fixes to indexes, updated icons, background, styling, various bugfixes,
logfile control and a warning about running a wallet on a masternode.

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

Insight
-------

The most notible changes are the ones made for insight, a new utility
`wallet-utility` will be included in the utils build.  Also how transactions
are indexed has changed.  A `reindex` is not required, but it is encoraged to
index these changes.

Miscellaneous
-------------

Icons have all changed to be more consistent and the main background image was
changed to make more of the client more visible.

Added `-debuglogfile` option which allows you to specified an absolute or
relative file for the debug.log.  This will allow you to put it and name it as
you'd like.  By default everythign will stay the same.

0.12.1.7 Change log
===================

Detailed release notes follow. This overview includes changes that affect
behavior, not code moves, refactors and string updates. For convenience in
locating the code changes and accompanying discussion, both the pull request and
git merge commit are mentioned.

### RPC and other APIs
- #13 `cc53711` Cherry pick a few dash patches, including insight required changes (thesin)
- `ffs75ed` Add block.auxpow to CBlockHeader, PR6 (thesin)

### Parameters
- `c87876e` Add -debuglogfile param to set logfile location and name (thesin)

### Builds
- `dc99462` Allow building with Qt4 (strictly Qt<5.2), PR10 (walkjivefly)

### Miscellaneous
- `66811e3` Add a warning about running a wallet on a masternode (thesin)
- `240de00` Update all logos to unify, style updates and change background image (thesin)

Credits
=======

Thanks to everyone who directly contributed to this release:

- TheSin
- walkjivefly
- hanzac

As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/terracoin/).

