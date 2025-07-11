// Copyright (c) 2012 Pieter Wuille
// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <addrman.h>

#include <hash.h>
#include <serialize.h>
#include <streams.h>
#include <algorithm>

int CAddrInfo::GetTriedBucket(const uint256& nKey) const {
    auto hash1 = (CHashWriter(SER_GETHASH, 0) << nKey << GetKey()).GetHash().GetCheapHash();
    auto hash2 = (CHashWriter(SER_GETHASH, 0) << nKey << GetGroup() << (hash1 % ADDRMAN_TRIED_BUCKETS_PER_GROUP)).GetHash().GetCheapHash();
    return hash2 % ADDRMAN_TRIED_BUCKET_COUNT;
}

int CAddrInfo::GetNewBucket(const uint256& nKey, const CNetAddr& src) const {
    auto sourceGroup = src.GetGroup();
    auto hash1 = (CHashWriter(SER_GETHASH, 0) << nKey << GetGroup() << sourceGroup).GetHash().GetCheapHash();
    auto hash2 = (CHashWriter(SER_GETHASH, 0) << nKey << sourceGroup << (hash1 % ADDRMAN_NEW_BUCKETS_PER_SOURCE_GROUP)).GetHash().GetCheapHash();
    return hash2 % ADDRMAN_NEW_BUCKET_COUNT;
}

int CAddrInfo::GetBucketPosition(const uint256& nKey, bool fNew, int nBucket) const {
    auto hash = (CHashWriter(SER_GETHASH, 0) << nKey << (fNew ? 'N' : 'K') << nBucket << GetKey()).GetHash().GetCheapHash();
    return hash % ADDRMAN_BUCKET_SIZE;
}

bool CAddrInfo::IsTerrible(int64_t now) const {
    if (nLastTry && nLastTry >= now - 60) return false;
    if (nTime > now + 600) return true;
    if (nTime == 0 || now - nTime > ADDRMAN_HORIZON_DAYS * 86400) return true;
    if (nLastSuccess == 0 && nAttempts >= ADDRMAN_RETRIES) return true;
    if (now - nLastSuccess > ADDRMAN_MIN_FAIL_DAYS * 86400 && nAttempts >= ADDRMAN_MAX_FAILURES) return true;
    return false;
}

double CAddrInfo::GetChance(int64_t now) const {
    double chance = 1.0;
    int64_t sinceLastTry = std::max<int64_t>(now - nLastTry, 0);
    if (sinceLastTry < 600) chance *= 0.01;
    chance *= std::pow(0.66, std::min(nAttempts, 8));
    return chance;
}

CAddrInfo* CAddrMan::Find(const CNetAddr& addr, int* pnId) {
    auto it = mapAddr.find(addr);
    if (it == mapAddr.end()) return nullptr;
    if (pnId) *pnId = it->second;
    auto it2 = mapInfo.find(it->second);
    return (it2 != mapInfo.end()) ? &it2->second : nullptr;
}

CAddrInfo* CAddrMan::Create(const CAddress& addr, const CNetAddr& source, int* pnId) {
    int id = nIdCount++;
    mapInfo[id] = CAddrInfo(addr, source);
    mapAddr[addr] = id;
    mapInfo[id].nRandomPos = vRandom.size();
    vRandom.push_back(id);
    if (pnId) *pnId = id;
    return &mapInfo[id];
}

void CAddrMan::SwapRandom(unsigned int pos1, unsigned int pos2) {
    if (pos1 == pos2) return;
    assert(pos1 < vRandom.size() && pos2 < vRandom.size());
    std::swap(vRandom[pos1], vRandom[pos2]);
    mapInfo[vRandom[pos1]].nRandomPos = pos1;
    mapInfo[vRandom[pos2]].nRandomPos = pos2;
}

void CAddrMan::Delete(int id) {
    assert(mapInfo.count(id));
    auto& info = mapInfo[id];
    assert(!info.fInTried && info.nRefCount == 0);
    SwapRandom(info.nRandomPos, vRandom.size() - 1);
    vRandom.pop_back();
    mapAddr.erase(info);
    mapInfo.erase(id);
    nNew--;
}

void CAddrMan::ClearNew(int bucket, int pos) {
    if (vvNew[bucket][pos] != -1) {
        int id = vvNew[bucket][pos];
        auto& info = mapInfo[id];
        assert(info.nRefCount > 0);
        info.nRefCount--;
        vvNew[bucket][pos] = -1;
        if (info.nRefCount == 0) Delete(id);
    }
}

void CAddrMan::MakeTried(CAddrInfo& info, int id) {
    for (int bucket = 0; bucket < ADDRMAN_NEW_BUCKET_COUNT; ++bucket) {
        int pos = info.GetBucketPosition(nKey, true, bucket);
        if (vvNew[bucket][pos] == id) {
            vvNew[bucket][pos] = -1;
            info.nRefCount--;
        }
    }
    nNew--;
    assert(info.nRefCount == 0);

    int triedBucket = info.GetTriedBucket(nKey);
    int triedPos = info.GetBucketPosition(nKey, false, triedBucket);

    if (vvTried[triedBucket][triedPos] != -1) {
        int evictId = vvTried[triedBucket][triedPos];
        auto& evictInfo = mapInfo[evictId];
        evictInfo.fInTried = false;
        vvTried[triedBucket][triedPos] = -1;
        nTried--;

        int newBucket = evictInfo.GetNewBucket(nKey);
        int newPos = evictInfo.GetBucketPosition(nKey, true, newBucket);
        ClearNew(newBucket, newPos);
        evictInfo.nRefCount = 1;
        vvNew[newBucket][newPos] = evictId;
        nNew++;
    }
    assert(vvTried[triedBucket][triedPos] == -1);

    vvTried[triedBucket][triedPos] = id;
    nTried++;
    info.fInTried = true;
}
