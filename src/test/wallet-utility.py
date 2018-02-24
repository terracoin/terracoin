#!/usr/bin/python
# Copyright 2014 BitPay, Inc.
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

import subprocess
import os
import json
import sys
import buildenv
import shutil

def assert_equal(thing1, thing2):
    if thing1 != thing2:
        raise AssertionError("%s != %s"%(str(thing1),str(thing2)))

if __name__ == '__main__':
    datadir = os.environ["srcdir"] + "/test/data"
    execprog = './wallet-utility' + buildenv.exeext
    execargs = '-datadir=' + datadir
    execrun = execprog + ' ' + execargs

    proc = subprocess.Popen(execrun, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, shell=True)
    try:
        outs = proc.communicate()
    except OSError:
        print("OSError, Failed to execute " + execprog)
        sys.exit(1)

    output = json.loads(outs[0])

    assert_equal(output[0], "XwKHjUS2pS4inZT8nkqDSybMHi5hCYNSNx")
    assert_equal(output[1], "XrihbS2Y29C3Gca72UWJiarUeaCHu7wumB")
    assert_equal(output[2], "XqWwRpETvism6uSiAv2oHj4gxzUvtxSNkW")

    execargs = '-datadir=' + datadir + ' -dumppass'
    execrun = execprog + ' ' + execargs

    proc = subprocess.Popen(execrun, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, shell=True)
    try:
        outs = proc.communicate()
    except OSError:
        print("OSError, Failed to execute " + execprog)
        sys.exit(1)

    output = json.loads(outs[0])

    assert_equal(output[0]['addr'], "XwKHjUS2pS4inZT8nkqDSybMHi5hCYNSNx")
    assert_equal(output[0]['pkey'], "XKk4fT2aB1pW5sGa6pBF8MA4bUaAcVEpWUnZQWzDc2XKUBkvxDq1")
    assert_equal(output[1]['addr'], "XrihbS2Y29C3Gca72UWJiarUeaCHu7wumB")
    assert_equal(output[1]['pkey'], "XBMge2M9Fd9YvCw3JqSE1aWEw56xJeRxRcQvb3JztsR45yK12s1H")
    assert_equal(output[2]['addr'], "XqWwRpETvism6uSiAv2oHj4gxzUvtxSNkW")
    assert_equal(output[2]['pkey'], "XBL71uLhA2p8q3qtXRSaLuQ5rwRb9gxFAeuhSbofwNcr26bGexfG")

    if os.path.exists(datadir + '/database'):
        if os.path.isdir(datadir + '/database'):
            shutil.rmtree(datadir + '/database')

    if os.path.exists(datadir + '/db.log'):
        os.remove(datadir + '/db.log')
    sys.exit(0)
