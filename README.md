Terracoin Core staging tree 0.12.1
==================================

`master:` [![Build Status](https://travis-ci.org/terracoin/terracoin.svg?branch=master)](https://travis-ci.org/terracoin/terracoin) `v0.12.1.x:` [![Build Status](https://travis-ci.org/terracoin/terracoin.svg?branch=v0.12.1.x)](https://travis-ci.org/terracoin/terracoin/branches)

https://www.terracoin.io


What is Terracoin?
------------------

Terracoin aims to be the most user-friendly and scalable payments-focused cryptocurrency in the world. The Terracoin network features instant transaction confirmation, double spend protection, anonymity equal to that of physical cash, a self-governing, self-funding model driven by incentivized full nodes and a clear roadmap. While Terracoin is based on Bitcoin and compatible with many key components of the Bitcoin ecosystem, its two-tier network structure offers significant improvements in transaction speed, anonymity and governance. This section of the documentation describes these and many more key features that set Terracoin apart in the blockchain economy. For full details, please read the [Terracoin whitepaper](https://wiki.terracoin.io/view/Whitepaper).

For more information, as well as an immediately useable, binary version of
the Terracoin Core software, see https://www.terracoin.io/#downloads.


License
-------

Terracoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is meant to be stable. Development is normally done in separate branches.
[Tags](https://github.com/terracoin/terracoin/tags) are created to indicate new official,
stable release versions of Terracoin Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows
and Linux, OS X, and that unit and sanity tests are automatically run.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.
