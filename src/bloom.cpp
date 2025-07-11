// Copyright (c) 2012-2024 NoteCoin Core Developers
// Distributed under the MIT software license, see the accompanying file COPYING.

#include <bloom.h>

#include <primitives/transaction.h>
#include <hash.h>
#include <script/script.h>
#include <script/standard.h>
#include <random.h>
#include <streams.h>

#include <cmath>
#include <algorithm>
#include <limits>

constexpr double LN2SQUARED = 0.4804530139182014;
constexpr double LN2 = 0.6931471805599453;

CBloomFilter::CBloomFilter(unsigned int nElements, double nFPRate, unsigned int nTweakIn, unsigned char nFlagsIn)
    : vData(std::min(static_cast<unsigned int>(-1 / LN2SQUARED * nElements * std::log(nFPRate)), MAX_BLOOM_FILTER_SIZE * 8) / 8),
      isFull(false),
      isEmpty(true),
      nHashFuncs(std::min(static_cast<unsigned int>(vData.size() * 8 / nElements * LN2), MAX_HASH_FUNCS)),
      nTweak(nTweakIn),
      nFlags(nFlagsIn) {}

CBloomFilter::CBloomFilter(unsigned int nElements, double nFPRate, unsigned int nTweakIn)
    : vData(static_cast<unsigned int>(-1 / LN2SQUARED * nElements * std::log(nFPRate)) / 8),
      isFull(false),
      isEmpty(true),
      nHashFuncs(static_cast<unsigned int>(vData.size() * 8 / nElements * LN2)),
      nTweak(nTweakIn),
      nFlags(BLOOM_UPDATE_NONE) {}

inline unsigned int CBloomFilter::Hash(unsigned int nHashNum, const std::vector<unsigned char>& data) const {
    return MurmurHash3(nHashNum * 0xFBA4C795 + nTweak, data) % (vData.size() * 8);
}

void CBloomFilter::insert(const std::vector<unsigned char>& key) {
    if (isFull) return;
    for (unsigned int i = 0; i < nHashFuncs; ++i) {
        const unsigned int idx = Hash(i, key);
        vData[idx >> 3] |= (1 << (7 & idx));
    }
    isEmpty = false;
}

void CBloomFilter::insert(const COutPoint& outpoint) {
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << outpoint;
    insert({ss.begin(), ss.end()});
}

void CBloomFilter::insert(const uint256& hash) {
    insert({hash.begin(), hash.end()});
}

bool CBloomFilter::contains(const std::vector<unsigned char>& key) const {
    if (isFull) return true;
    if (isEmpty) return false;
    for (unsigned int i = 0; i < nHashFuncs; ++i) {
        const unsigned int idx = Hash(i, key);
        if (!(vData[idx >> 3] & (1 << (7 & idx)))) return false;
    }
    return true;
}

bool CBloomFilter::contains(const COutPoint& outpoint) const {
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << outpoint;
    return contains({ss.begin(), ss.end()});
}

bool CBloomFilter::contains(const uint256& hash) const {
    return contains({hash.begin(), hash.end()});
}

void CBloomFilter::clear() {
    std::fill(vData.begin(), vData.end(), 0);
    isFull = false;
    isEmpty = true;
}

void CBloomFilter::reset(unsigned int newTweak) {
    clear();
    nTweak = newTweak;
}

bool CBloomFilter::IsWithinSizeConstraints() const {
    return vData.size() <= MAX_BLOOM_FILTER_SIZE && nHashFuncs <= MAX_HASH_FUNCS;
}

bool CBloomFilter::IsRelevantAndUpdate(const CTransaction& tx) {
    if (isFull) return true;
    if (isEmpty) return false;

    const uint256& hash = tx.GetHash();
    bool match = contains(hash);

    for (unsigned int i = 0; i < tx.vout.size(); ++i) {
        const CTxOut& txout = tx.vout[i];
        CScript::const_iterator pc = txout.scriptPubKey.begin();
        std::vector<unsigned char> data;
        while (pc < txout.scriptPubKey.end()) {
            opcodetype opcode;
            if (!txout.scriptPubKey.GetOp(pc, opcode, data)) break;
            if (!data.empty() && contains(data)) {
                match = true;
                if ((nFlags & BLOOM_UPDATE_MASK) == BLOOM_UPDATE_ALL) {
                    insert(COutPoint(hash, i));
                } else if ((nFlags & BLOOM_UPDATE_MASK) == BLOOM_UPDATE_P2PUBKEY_ONLY) {
                    txnouttype type;
                    std::vector<std::vector<unsigned char>> solutions;
                    if (Solver(txout.scriptPubKey, type, solutions) &&
                        (type == TX_PUBKEY || type == TX_MULTISIG)) {
                        insert(COutPoint(hash, i));
                    }
                }
                break;
            }
        }
    }

    if (match) return true;

    for (const CTxIn& txin : tx.vin) {
        if (contains(txin.prevout)) return true;

        CScript::const_iterator pc = txin.scriptSig.begin();
        std::vector<unsigned char> data;
        while (pc < txin.scriptSig.end()) {
            opcodetype opcode;
            if (!txin.scriptSig.GetOp(pc, opcode, data)) break;
            if (!data.empty() && contains(data)) return true;
        }
    }

    return false;
}

void CBloomFilter::UpdateEmptyFull() {
    isFull = std::all_of(vData.begin(), vData.end(), [](uint8_t b) { return b == 0xff; });
    isEmpty = std::all_of(vData.begin(), vData.end(), [](uint8_t b) { return b == 0; });
}

// --- Rolling Bloom Filter Implementation ---

CRollingBloomFilter::CRollingBloomFilter(unsigned int nElements, double fpRate) {
    double logFpRate = std::log(fpRate);
    nHashFuncs = std::clamp(static_cast<int>(std::round(logFpRate / std::log(0.5))), 1, 50);
    nEntriesPerGeneration = (nElements + 1) / 2;
    uint32_t nMaxElements = nEntriesPerGeneration * 3;
    uint32_t nFilterBits = static_cast<uint32_t>(std::ceil(
        -1.0 * nHashFuncs * nMaxElements / std::log(1.0 - std::exp(logFpRate / nHashFuncs))));

    data.clear();
    data.resize(((nFilterBits + 63) / 64) << 1);
    reset();
}

inline static uint32_t RollingBloomHash(unsigned int nHashNum, uint32_t nTweak, const std::vector<unsigned char>& data) {
    return MurmurHash3(nHashNum * 0xFBA4C795 + nTweak, data);
}

void CRollingBloomFilter::insert(const std::vector<unsigned char>& key) {
    if (nEntriesThisGeneration == nEntriesPerGeneration) {
        nEntriesThisGeneration = 0;
        nGeneration = (nGeneration % 3) + 1;

        uint64_t genMask1 = ~(uint64_t)(nGeneration & 1);
        uint64_t genMask2 = ~(uint64_t)(nGeneration >> 1);
        for (size_t p = 0; p < data.size(); p += 2) {
            data[p] &= genMask1 ^ data[p];
            data[p + 1] &= genMask2 ^ data[p + 1];
        }
    }

    ++nEntriesThisGeneration;
    for (int n = 0; n < nHashFuncs; ++n) {
        uint32_t h = RollingBloomHash(n, nTweak, key);
        int bit = h & 0x3F;
        uint32_t pos = (h >> 6) % data.size();
        data[pos & ~1] = (data[pos & ~1] & ~((uint64_t)1 << bit)) | ((uint64_t)(nGeneration & 1) << bit);
        data[pos | 1] = (data[pos | 1] & ~((uint64_t)1 << bit)) | ((uint64_t)(nGeneration >> 1) << bit);
    }
}

void CRollingBloomFilter::insert(const uint256& hash) {
    insert({hash.begin(), hash.end()});
}

bool CRollingBloomFilter::contains(const std::vector<unsigned char>& key) const {
    for (int n = 0; n < nHashFuncs; ++n) {
        uint32_t h = RollingBloomHash(n, nTweak, key);
        int bit = h & 0x3F;
        uint32_t pos = (h >> 6) % data.size();
        if (!(((data[pos & ~1] | data[pos | 1]) >> bit) & 1)) return false;
    }
    return true;
}

bool CRollingBloomFilter::contains(const uint256& hash) const {
    return contains({hash.begin(), hash.end()});
}

void CRollingBloomFilter::reset() {
    nTweak = GetRand(std::numeric_limits<unsigned int>::max());
    nEntriesThisGeneration = 0;
    nGeneration = 1;
    std::fill(data.begin(), data.end(), 0);
}
