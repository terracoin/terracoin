// Copyright (c) 2014-2018 The Crown developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpc/update.h"

#include "init.h"
#include "clientversion.h"
#include "rpc/server.h"
#include "util.h"

#include <univalue.h>
#include <boost/thread.hpp>

#include <sys/stat.h>

using namespace std;
using namespace boost::filesystem;

bool RPCUpdate::started = false;
UniValue RPCUpdate::statusObj(UniValue::VOBJ);

std::string RPCUpdate::GetArchivePath() const
{
    std::string url = updater.GetDownloadUrl();
    return (tempDir / path(url).filename()).string();
}

bool RPCUpdate::Download()
{
    statusObj.clear();
    // Create temporary directory
    tempDir = GetTempPath() / unique_path();
    bool result = TryCreateDirectory(tempDir);
    if (!result) {
        throw runtime_error("Failed to create directory" + tempDir.string());
    }

    // Download archive
    std::string archivePath = GetArchivePath();
    updater.DownloadFile(updater.GetDownloadUrl(), archivePath, &ProgressFunction);
    if (updater.GetStopDownload())
    {
        started = false;
        statusObj.push_back(Pair("Download", "Stopped"));
        remove_all(tempDir);
        return false;
    }
    if (CheckSha(archivePath))
    {
        statusObj.push_back(Pair("Download", "Done - " + archivePath));
    }
    else
    {
        statusObj.push_back(Pair("Download", "Error. SHA-256 verification failed."));
        remove_all(tempDir);
        return false;
    }
    return true;
}

void RPCUpdate::Install()
{
    statusObj.clear();
    if (!Download())
    {
        return;
    }
    // Extract archive
    bool result = TryCreateDirectory(tempDir / "archive");
    if (!result) {
        throw runtime_error(strprintf("Failed to create directory %s", (tempDir / "archive").string()));
    }
    std::string strCommand = strprintf("tar xzf %s -C %s --strip-components=1", GetArchivePath(), (tempDir / "archive").string());
    int nErr = ::system(strCommand.c_str());
    if (nErr) {
        LogPrintf("runCommand error: system(%s) returned %d\n", strCommand, nErr);
        statusObj.push_back(Pair("Extract", "Error. Check debug.log"));
    } else {
        statusObj.push_back(Pair("Extract", "Done"));
    }

    // Copy files to /usr/
    if (!nErr) {
        strCommand = strprintf("cp -rf %s /usr/local/bin/", (tempDir / "archive/bin/*").string());
        nErr = ::system(strCommand.c_str());
        if (nErr) {
            LogPrintf("runCommand error: system(%s) returned %d\n", strCommand, nErr);
            statusObj.push_back(Pair("Install", "Error. Check debug.log."));
        } else {
            statusObj.push_back(Pair("Install", "Done"));
        }

        // Restart terracoind
        StartRestart();
    }

    boost::filesystem::remove_all(tempDir);
}

void RPCUpdate::ProgressFunction(curl_off_t now, curl_off_t total)
{
    int percent = 0;
    if (total != 0) {
        percent = now * 100 / total;
    }
    if (statusObj.size() == 0) {
        statusObj.push_back(Pair("Download", "In Progress"));
    }
    if ((now == total) && now != 0) {
        started = false;
        statusObj.push_back(Pair("Download", "Done"));
    } else if (now != total) {
        started = true;
        statusObj.push_back(Pair("Download", strprintf("%0.1f/%0.1fMB, %d%%",
                                        static_cast<double>(now) / 1024 / 1024,
                                        static_cast<double>(total) / 1024 / 1024,
                                        percent)));
    }
}

bool RPCUpdate::IsStarted() const
{
    return started;
}

UniValue RPCUpdate::GetStatusObject() const
{
    return statusObj;
}

bool RPCUpdate::CheckSha(const std::string& fileName) const
{
    bool result = false;
    std::string newSha = updater.GetDownloadSha256Sum();
    try
    {
        std::string sha = Sha256Sum(fileName);
        if (!sha.empty()) {
            if (newSha.compare(sha) == 0) {
                result = true;
            } else {
                result = false;
            }
        } else {
            result = false;
        }
    } catch(std::runtime_error &e) {
        result = false;
    }
    return result;
}

UniValue update(const UniValue& params, bool fHelp)
{
    RPCUpdate rpcUpdate;
    string strCommand;
    if (params.size() >= 1)
        strCommand = params[0].get_str();

    if (fHelp  || (strCommand != "check" && strCommand != "download" && strCommand != "status" && 
                   strCommand != "install" && strCommand != "stop"))
        throw runtime_error(
                "update \"command\"... ( \"passphrase\" )\n"
                "Set of commands to check and download application updates\n"
                "\nArguments:\n"
                "1. \"command\"        (string or set of strings, required) The command to execute\n"
                "2. \"passphrase\"     (string, optional) The wallet passphrase\n"
                "\nAvailable commands:\n"
                "  check        - Check for application updates\n"
                "  download     - Download a new version\n"
                "  status       - Check download status\n"
                "  install      - Install update\n"
                "  stop         - Stop update/install\n"
                );

    if (strCommand == "check")
    {
        if (params.size() > 1) {
            throw runtime_error("Too many parameters\n");
        }

        if (updater.GetStatus())
        {
            // There is an update
            UniValue obj(UniValue::VOBJ);
            obj.push_back(Pair("Current Version", FormatVersion(CLIENT_VERSION)));
            obj.push_back(Pair("Update Version", FormatVersion(updater.GetVersion())));
            obj.push_back(Pair("OS", updater.GetOsString()));
            obj.push_back(Pair("Url", updater.GetDownloadUrl()));
            obj.push_back(Pair("Sha256hash", updater.GetDownloadSha256Sum()));
            return obj;
        }
        return "You are running the latest version of Terracoin Core - " + FormatVersion(CLIENT_VERSION);
    }
    
    if (strCommand == "download")
    {
        if (!updater.GetStatus())
        {
            return "You are running the latest version of Terracoin Core - " + FormatVersion(CLIENT_VERSION);
        }
        if (rpcUpdate.IsStarted())
        {
            return "Download is in progress. Run 'terracoin-cli update status' to check the status.";
        }
        boost::thread t(boost::bind(&RPCUpdate::Download, rpcUpdate));
        return "Terracoin Core download started. \nRun 'terracoin-cli update status' to check download status.";
    }

    if (strCommand == "status")
    {
        return rpcUpdate.GetStatusObject();
    }

    if (strCommand == "stop")
    {
        updater.StopDownload();
        return "Terracoin Core download stopped.";
    }

    if (strCommand == "install")
    {
        if (!fServer)
        {
            throw runtime_error("Command is available only in server mode."
                "\nterracoin-qt will automatically check and notify if there is an updates\n");
        }

        if (updater.GetOS() != Updater::LINUX_32 && updater.GetOS() != Updater::LINUX_64)
        {
            throw runtime_error("Command is available only in Linux.");
        }

        struct stat buffer;
        if (stat("/usr/local/bin/terracoind", &buffer) != 0)
        {
            throw runtime_error("Terracoin Core not located in /usr/local/bin, can not install.");
        }

        if (::system("command -v sudo"))
        {
            throw runtime_error("The command 'sudo' could not be found. Please install it and try again.");
        }

        if (::system("command -v tar"))
        {
            throw runtime_error("The command 'tar' could not be found. Please install it and try again.");
        }

        if (!updater.GetStatus())
        {
            return "You are running the latest version of Terracoin Core - " + FormatVersion(CLIENT_VERSION);
        }
        if (rpcUpdate.IsStarted())
        {
            return "Download is in progress. Run 'terracoin-cli update status' to check the status.";
        }
        boost::thread t(boost::bind(&RPCUpdate::Install, rpcUpdate));
        return "Terracoin Core install started. \nRun 'terracoin-cli update status' to check download status.";
    }
    return "";
}
