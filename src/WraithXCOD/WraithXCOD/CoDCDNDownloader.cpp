#include "stdafx.h"

// The class we are implementing
#include "CoDCDNDownloader.h"

CoDCDNDownloader::~CoDCDNDownloader()
{
}

void CoDCDNDownloader::AddFailed(const uint64_t cacheID)
{
    FailedAttempts[cacheID] = std::chrono::system_clock::now();
}

bool CoDCDNDownloader::HasFailed(const uint64_t cacheID)
{
    auto find = FailedAttempts.find(cacheID);

    if (find != FailedAttempts.end())
    {
        auto currentClock = std::chrono::system_clock::now();
        auto timePassed = (currentClock - find->second);

        // If it's been 60 seconds since we last 
        if (timePassed <= std::chrono::duration<int64_t, std::milli>(60000))
        {
            return true;
        }
        else
        {
            FailedAttempts.erase(find);
        }
    }

    // No entry or time has passed that we can try again.
    return false;
}
