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

    assert_equal(output[0], "1MdSuDn8rir8dcrYvsWzbSuZTNW1BABKQC")
    assert_equal(output[1], "1H2rmBNe4RyT7fyXAbC5s4AgpEcbsJ82gJ")
    assert_equal(output[2], "1Fq6bZaZy1fAwxr8K2iaSCNu8euEt3eKWo")

    execargs = '-datadir=' + datadir + ' -dumppass'
    execrun = execprog + ' ' + execargs

    proc = subprocess.Popen(execrun, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, shell=True)
    try:
        outs = proc.communicate()
    except OSError:
        print("OSError, Failed to execute " + execprog)
        sys.exit(1)

    output = json.loads(outs[0])

    assert_equal(output[0]['addr'], "1MdSuDn8rir8dcrYvsWzbSuZTNW1BABKQC")
    assert_equal(output[0]['pkey'], "L5g9DBeCsLC42YGC54BNd7y3gTJbAgda8uSeszf2HfFE52fAbKnu")
    assert_equal(output[1]['addr'], "1H2rmBNe4RyT7fyXAbC5s4AgpEcbsJ82gJ")
    assert_equal(output[1]['pkey'], "KwHmBkxmwwX6rsvfH5SMWMKE23qNrqpi43524WyoaW8xgpCfdVAG")
    assert_equal(output[2]['addr'], "1Fq6bZaZy1fAwxr8K2iaSCNu8euEt3eKWo")
    assert_equal(output[2]['pkey'], "KwGBZdxKrMBgmiqWVfShqgD4wvA1htLzo5Znv5UUd1LkcwTa6GMu")

    if os.path.exists(datadir + '/database'):
        if os.path.isdir(datadir + '/database'):
            shutil.rmtree(datadir + '/database')

    if os.path.exists(datadir + '/db.log'):
        os.remove(datadir + '/db.log')
    sys.exit(0)
