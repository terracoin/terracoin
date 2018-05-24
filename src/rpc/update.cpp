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
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>

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
    statusObj.setObject();
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

    std::string shacheck = CheckSha(archivePath);
    if (shacheck.compare("OK") == 0)
    {
        statusObj.push_back(Pair("Download", "Done - " + archivePath));
    }
    else
    {
        statusObj.push_back(Pair("Download", shacheck));
        remove_all(tempDir);
        return false;
    }
    return true;
}

#if (defined(__MINGW32__) || defined(__CYGWIN__))
struct heap_delete
{
    typedef LPVOID pointer;
    void operator()(LPVOID p)
    {
        ::HeapFree(::GetProcessHeap(), 0, p);
    }
};
typedef std::unique_ptr<LPVOID, heap_delete> heap_unique_ptr;

struct handle_delete
{
    typedef HANDLE pointer;
    void operator()(HANDLE p)
    {
        ::CloseHandle(p);
    }
};
typedef std::unique_ptr<HANDLE, handle_delete> handle_unique_ptr;

typedef uint32_t uid_t;

BOOL GetUserSID(HANDLE token, PSID* sid)
{
    if (
        token == nullptr || token == INVALID_HANDLE_VALUE
        || sid == nullptr
        )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    DWORD tokenInformationLength = 0;
    ::GetTokenInformation(
        token, TokenUser, nullptr, 0, &tokenInformationLength);
    if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        return FALSE;
    }
    heap_unique_ptr data(
        ::HeapAlloc(
            ::GetProcessHeap(), HEAP_ZERO_MEMORY,     
            tokenInformationLength));
    if (data.get() == nullptr)
    {
        return FALSE;
    }
    BOOL getTokenInfo = ::GetTokenInformation(
        token, TokenUser, data.get(),
        tokenInformationLength, &tokenInformationLength);
    if (! getTokenInfo)
    {
        return FALSE;
    }
    PTOKEN_USER pTokenUser = (PTOKEN_USER)(data.get());
    DWORD sidLength = ::GetLengthSid(pTokenUser->User.Sid);
    heap_unique_ptr sidPtr(
        ::HeapAlloc(
            GetProcessHeap(), HEAP_ZERO_MEMORY, sidLength));
    PSID sidL = (PSID)(sidPtr.get());
    if (sidL == nullptr)
    {
        return FALSE;
    }
    BOOL copySid = ::CopySid(sidLength, sidL, pTokenUser->User.Sid);
    if (! copySid)
    {
        return FALSE;
    }
    if (!IsValidSid(sidL))
    {
        return FALSE;
    }
    *sid = sidL;
    sidPtr.release();
    return TRUE;
}

uid_t GetUID(HANDLE token)
{
    PSID sid = nullptr;
    BOOL getSID = GetUserSID(token, &sid);
    if (! getSID || ! sid)
    {
        return -1;
    }
    heap_unique_ptr sidPtr((LPVOID)(sid));
    LPWSTR stringSid = nullptr;
    BOOL convertSid = ::ConvertSidToStringSidW(
        sid, &stringSid);
    if (! convertSid)
    {
        return -1;
    }
    uid_t ret = -1;
    LPCWSTR p = ::wcsrchr(stringSid, L'-');
    if (p && ::iswdigit(p[1]))
    {
        ++p;
        ret = ::_wtoi(p);
    }
    ::LocalFree(stringSid);
    return ret;
}

uid_t getuid()
{
    HANDLE process = ::GetCurrentProcess();
    handle_unique_ptr processPtr(process);
    HANDLE token = nullptr;
    BOOL openToken = ::OpenProcessToken(
        process, TOKEN_READ|TOKEN_QUERY_SOURCE, &token);
    if (! openToken)
    {
        return -1;
    }
    handle_unique_ptr tokenPtr(token);
    uid_t ret = GetUID(token);
    return ret;
}
#endif 

std::string getexe(bool pathonly = false)
{
    boost::filesystem::path result = boost::filesystem::read_symlink( "/proc/self/exe" );
    char *path = new char[MAX_PATH];
    strcpy( path, result.c_str() );

    if (pathonly)
        return std::string( dirname( path ) );
    else
        return std::string( result.c_str() );
}

void RPCUpdate::Install()
{
    statusObj.setObject();
    if (!Download())
    {
        return;
    }

    string installLoc = getexe();
    string install_path = getexe(true);
    struct stat buffer;
    stat(installLoc.c_str(), &buffer);
    int installuid = buffer.st_uid;
    int installgid = buffer.st_gid;
    string usesudo = "";

    if (getuid() != installuid)
        usesudo = strprintf("sudo -n -u %d:%d ", installuid, installgid);

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

    // Copy files to install_path
    if (!nErr) {
        strCommand = strprintf("%scp -rf %s %s", usesudo, (tempDir / "archive/bin/*").string(), install_path);
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
    int percent = now * 100 / updater.GetDownloadSize();
    if (statusObj.size() == 0) {
        statusObj.push_back(Pair("Download", "In Progress"));
    }
    if ((now == updater.GetDownloadSize()) && now != 0) {
        started = false;
        statusObj.push_back(Pair("Download", "Done"));
    } else if (now != updater.GetDownloadSize()) {
        started = true;
        statusObj.push_back(Pair("Download", strprintf("%0.1f/%0.1fMB, %d%%",
                                        static_cast<double>(now) / 1024 / 1024,
                                        static_cast<double>(updater.GetDownloadSize()) / 1024 / 1024,
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

std::string RPCUpdate::CheckSha(const std::string& fileName) const
{
    std::string result;
    std::string newSha = updater.GetDownloadSha256Sum();
    try
    {
        std::string sha = Sha256Sum(fileName);
        if (!sha.empty()) {
            if (newSha.compare(sha) == 0) {
                result = "OK";
            } else {
                result = "Error. SHA-256 verification did NOT match. (" + newSha + " != " + sha + ")";
            }
        } else {
            result = "Error. SHA-256 verification failed to run.";
        }
    } catch(std::string &e) {
        result = "Error. SHA-256 verification failed: " + e;
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
            obj.push_back(Pair("Size", updater.GetDownloadSize()));
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

        string installLoc = getexe();
        string install_path = getexe(true);
        struct stat buffer;
        if (stat(installLoc.c_str(), &buffer) != 0)
        {
            throw runtime_error(strprintf("Terracoin Core not located in %s, can not install.", install_path.c_str()));
        }
        int installeduid = buffer.st_uid;

        if (::system("command -v tar > /dev/null 2>&1"))
        {
            throw runtime_error("The command 'tar' could not be found. Please install it and try again.");
        }

        if (getuid() != installeduid) {
            if (::system("command -v sudo > /dev/null 2>&1"))
            {
                throw runtime_error("The command 'sudo' could not be found. Please install it and try again.");
            }
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
