// Copyright (c) 2012 Pieter Wuille
// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_ADDRMAN_H
#define BITCOIN_ADDRMAN_H

#include <netaddress.h>
#include <protocol.h>
#include <random.h>
#include <sync.h>
#include <timedata.h>
#include <util.h>

#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <cassert>

class CAddrInfo : public CAddress {
public:
    int64_t nLastTry{0};
    int64_t nLastCountAttempt{0};

private:
    CNetAddr source;
    int64_t nLastSuccess{0};
    int nAttempts{0};
    int nRefCount{0};
    bool fInTried{false};
    int nRandomPos{-1};

    friend class CAddrMan;

public:
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(*(CAddress*)this);
        READWRITE(source);
        READWRITE(nLastSuccess);
        READWRITE(nAttempts);
    }

    CAddrInfo(const CAddress& addrIn, const CNetAddr& addrSource)
        : CAddress(addrIn), source(addrSource) {}

    CAddrInfo() = default;

    int GetTriedBucket(const uint256& nKey) const;
    int GetNewBucket(const uint256& nKey, const CNetAddr& src) const;
    int GetNewBucket(const uint256& nKey) const {
        return GetNewBucket(nKey, source);
    }
    int GetBucketPosition(const uint256& nKey, bool fNew, int nBucket) const;
    bool IsTerrible(int64_t nNow = GetAdjustedTime()) const;
    double GetChance(int64_t nNow = GetAdjustedTime()) const;
};

// --- Constants ---
constexpr int ADDRMAN_TRIED_BUCKET_COUNT_LOG2 = 8;
constexpr int ADDRMAN_NEW_BUCKET_COUNT_LOG2 = 10;
constexpr int ADDRMAN_BUCKET_SIZE_LOG2 = 6;

constexpr int ADDRMAN_TRIED_BUCKET_COUNT = 1 << ADDRMAN_TRIED_BUCKET_COUNT_LOG2;
constexpr int ADDRMAN_NEW_BUCKET_COUNT = 1 << ADDRMAN_NEW_BUCKET_COUNT_LOG2;
constexpr int ADDRMAN_BUCKET_SIZE = 1 << ADDRMAN_BUCKET_SIZE_LOG2;

constexpr int ADDRMAN_TRIED_BUCKETS_PER_GROUP = 8;
constexpr int ADDRMAN_NEW_BUCKETS_PER_SOURCE_GROUP = 64;
constexpr int ADDRMAN_NEW_BUCKETS_PER_ADDRESS = 8;

constexpr int ADDRMAN_HORIZON_DAYS = 30;
constexpr int ADDRMAN_RETRIES = 3;
constexpr int ADDRMAN_MAX_FAILURES = 10;
constexpr int ADDRMAN_MIN_FAIL_DAYS = 7;
constexpr int ADDRMAN_GETADDR_MAX_PCT = 23;
constexpr int ADDRMAN_GETADDR_MAX = 2500;

class CAddrMan {
private:
    mutable CCriticalSection cs;
    int nIdCount{0};
    std::map<int, CAddrInfo> mapInfo;
    std::map<CNetAddr, int> mapAddr;
    std::vector<int> vRandom;

    int nTried{0};
    int vvTried[ADDRMAN_TRIED_BUCKET_COUNT][ADDRMAN_BUCKET_SIZE]{};

    int nNew{0};
    int vvNew[ADDRMAN_NEW_BUCKET_COUNT][ADDRMAN_BUCKET_SIZE]{};

    int64_t nLastGood{1};
    uint256 nKey;
    FastRandomContext insecure_rand;

    CAddrInfo* Find(const CNetAddr& addr, int* pnId = nullptr);
    CAddrInfo* Create(const CAddress& addr, const CNetAddr& addrSource, int* pnId = nullptr);
    void SwapRandom(unsigned int nRandomPos1, unsigned int nRandomPos2);
    void MakeTried(CAddrInfo& info, int nId);
    void Delete(int nId);
    void ClearNew(int nUBucket, int nUBucketPos);
    void Good_(const CService& addr, int64_t nTime);
    bool Add_(const CAddress& addr, const CNetAddr& source, int64_t nTimePenalty);
    void Attempt_(const CService& addr, bool fCountFailure, int64_t nTime);
    CAddrInfo Select_(bool newOnly);
    virtual int RandomInt(int nMax);
#ifdef DEBUG_ADDRMAN
    int Check_();
#endif
    void GetAddr_(std::vector<CAddress>& vAddr);
    void Connected_(const CService& addr, int64_t nTime);
    void SetServices_(const CService& addr, ServiceFlags nServices);

public:
    CAddrMan() { Clear(); }
    ~CAddrMan() { nKey.SetNull(); }

    void Clear();
    void Check();

    bool Add(const CAddress& addr, const CNetAddr& source, int64_t nTimePenalty = 0);
    bool Add(const std::vector<CAddress>& vAddr, const CNetAddr& source, int64_t nTimePenalty = 0);
    void Good(const CService& addr, int64_t nTime = GetAdjustedTime());
    void Attempt(const CService& addr, bool fCountFailure, int64_t nTime = GetAdjustedTime());
    CAddrInfo Select(bool newOnly = false);
    std::vector<CAddress> GetAddr();
    void Connected(const CService& addr, int64_t nTime = GetAdjustedTime());
    void SetServices(const CService& addr, ServiceFlags nServices);

    template<typename Stream>
    void Serialize(Stream& s) const;

    template<typename Stream>
    void Unserialize(Stream& s);

    size_t size() const {
        LOCK(cs);
        return vRandom.size();
    }
};

// Implementations of Serialize and Unserialize are ausgelagert in addrman.cpp

#endif // BITCOIN_ADDRMAN_H
