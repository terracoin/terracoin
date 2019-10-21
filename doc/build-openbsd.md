OpenBSD build guide
======================
(updated for OpenBSD 6.5)

This guide describes how to build terracoind and command-line utilities on OpenBSD.

As OpenBSD is most common as a server OS, we will not bother with the GUI.

Preparation
-------------

Run the following as root to install the base dependencies for building:

```bash
pkg_add gmake libtool libevent boost
pkg_add autoconf # (select highest version, e.g. 2.69)
pkg_add automake # (select highest version, e.g. 1.15)
pkg_add python # (select version 2.7.x, not 3.x)
ln -sf /usr/local/bin/python2.7 /usr/local/bin/python2
```

**Important**: From OpenBSD 6.2 onwards a C++11-supporting clang compiler is
part of the base image, and while building it is necessary to make sure that this
compiler is used and not ancient g++ 4.2.1. This is done by appending
`CC=cc CXX=c++` to configuration commands. Mixing different compilers
within the same executable will result in linker errors.

### Building BerkeleyDB

BerkeleyDB is only necessary for the wallet functionality. To skip this, pass `--disable-wallet` to `./configure`.

It is mandatory to use Berkeley DB 4.8. There is no BerkeleyDB library package in OpenBSD so if you need it you should build it yourself. To do so you can download the installation script included in BTC contrib [here](https://github.com/bitcoin/bitcoin/blob/master/contrib/install_db4.sh) and use it in the following way from the folder where you've decided to put it:

```shell
./install_db4.sh `pwd` CC=cc CXX=c++
```

Before using it please keep in mind that the backslash (```\```) may not be interpreted correctly by your version of bash so it could be advisable to edit instal_db4.sh and to change the line 

`"${BDB_PREFIX}/${BDB_VERSION}/dist/configure" \
  --enable-cxx --disable-shared --disable-replication --with-pic --prefix="${BDB_PREFIX}" \
  "${@}"`
into 

`"${BDB_PREFIX}/${BDB_VERSION}/dist/configure"  --enable-cxx --disable-shared --disable-replication --with-pic --prefix="${BDB_PREFIX}"  "${@}"`

Then set `BDB_PREFIX` for the next section:

```shell
export BDB_PREFIX="$PWD/db4"
```

### Building Terracoin Core

**Important**: use `gmake`, not `make`. The non-GNU `make` will exit with a horrible error.

Preparation:
```bash

# Replace this with the autoconf version that you installed. Include only
# the major and minor parts of the version: use "2.69" for "autoconf-2.69p2".
export AUTOCONF_VERSION=2.69

# Replace this with the automake version that you installed. Include only
# the major and minor parts of the version: use "1.16" for "automake-1.16.1".
export AUTOMAKE_VERSION=1.16

./autogen.sh
```
Make sure `BDB_PREFIX` is set to the appropriate path from the above steps.

To configure with wallet:
```bash
./configure --with-gui=no CC=cc CXX=c++ CPP=cpp LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"
```

To configure without wallet:
```bash
./configure --disable-wallet --with-gui=no CC=cc CXX=c++
```

Build and run the tests:
```bash
gmake # use -jX here for parallelism
gmake check
```
Resource limits
-------------------

If the build runs into out-of-memory errors, the instructions in this section
might help.

The standard ulimit restrictions in OpenBSD are very strict:

    data(kbytes)         1572864

This is, unfortunately, in some cases not enough to compile some `.cpp` files in the project,
(see issue [#6658](https://github.com/bitcoin/bitcoin/issues/6658)).
If your user is in the `staff` group the limit can be raised with:

    ulimit -d 3000000

The change will only affect the current shell and processes spawned by it. To
make the change system-wide, change `datasize-cur` and `datasize-max` in
`/etc/login.conf`, and reboot.
