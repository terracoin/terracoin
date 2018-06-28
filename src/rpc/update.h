// Copyright (c) 2014-2018 The Crown developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RPC_UPDATE_H
#define RPC_UPDATE_H 

#include "updater.h"
#include <boost/filesystem.hpp>

#include <univalue.h>

class RPCUpdate
{
public:
    bool Download();
    void Install();
    bool IsStarted() const;
    UniValue GetStatusObject() const;
    static void ProgressFunction(curl_off_t now, curl_off_t total);

private:
    std::string CheckSha(const std::string& fileName) const;
    std::string GetArchivePath() const;

private:
    static UniValue statusObj;
    static bool started;
    boost::filesystem::path tempDir;
};

#endif // RPC_UPDATE_H
