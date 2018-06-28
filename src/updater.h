// Copyright (c) 2014-2018 The Crown developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef UPDATER_H
#define UPDATER_H 

#include <iostream>
#define CURL_STATICLIB
#include "curl/curl.h"

#include <univalue.h>
#include <boost/optional.hpp>

class UpdateInfo;
class Updater;

extern Updater updater;

class Updater
{
public:
    enum OS {
        LINUX_32,
        LINUX_64,
        WINDOWS_32,
        WINDOWS_64,
        MAC_OS,
    };
    Updater();
    bool GetStatus();
    int GetVersion() const
    {
        return version;
    }
    OS GetOS() const
    {
        return os;
    }
    CURLcode DownloadFile(std::string url, std::string fileName, void(progressFunction)(curl_off_t, curl_off_t));
    void DownloadFileAsync(std::string url, std::string fileName, void(progressFunction)(curl_off_t, curl_off_t));
    void StopDownload();
    std::string GetDownloadUrl(boost::optional<OS> version = boost::none);
    std::string GetDownloadSha256Sum(boost::optional<OS> version = boost::none);
    int GetDownloadSize(boost::optional<OS> version = boost::none);
    std::string GetOsString(boost::optional<OS> os = boost::none);
    bool GetStopDownload()
    {
        return stopDownload;
    }

private:
    std::string updaterInfoUrl;
    OS os;
    bool status;
    int version;
    bool stopDownload;
    UniValue jsonData;
    const std::string url;

private:
    void LoadUpdateInfo();
    UniValue ParseJson(std::string info);
    void SetJsonPath();
    static OS DetectOS();
    bool NeedToBeUpdated();
    int GetVersionFromJson();
    std::string GetUrl(const UniValue& value);
    std::string GetSha256sum(UniValue value);
    int GetSize(UniValue value);
    void SetCAPath(CURL* curl);
    void CheckAndUpdateStatus(const std::string& updateData);
};

#endif
