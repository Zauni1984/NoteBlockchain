// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// SPDX-License-Identifier: MIT

#ifndef NOTECHAIN_CHAIN_H
#define NOTECHAIN_CHAIN_H

#include <algorithm>
#include <vector>
#include <string>

#include <arith_uint256.h>
#include <primitives/block.h>
#include <pow.h>
#include <tinyformat.h>
#include <uint256.h>

// Maximal erlaubter Zeitvorsprung eines Blocks gegenüber der aktuellen Netzzeit (z. B. zur Validierung)
static constexpr int64_t MAX_FUTURE_BLOCK_TIME = 2 * 60 * 60;
static constexpr int64_t TIMESTAMP_WINDOW = MAX_FUTURE_BLOCK_TIME;

// Informationen zu einer Block-Datei
class CBlockFileInfo {
public:
    unsigned int nBlocks{0};
    unsigned int nSize{0};
    unsigned int nUndoSize{0};
    unsigned int nHeightFirst{0};
    unsigned int nHeightLast{0};
    uint64_t nTimeFirst{0};
    uint64_t nTimeLast{0};

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(VARINT(nBlocks));
        READWRITE(VARINT(nSize));
        READWRITE(VARINT(nUndoSize));
        READWRITE(VARINT(nHeightFirst));
        READWRITE(VARINT(nHeightLast));
        READWRITE(VARINT(nTimeFirst));
        READWRITE(VARINT(nTimeLast));
    }

    void AddBlock(unsigned int nHeightIn, uint64_t nTimeIn) {
        if (nBlocks == 0 || nHeightFirst > nHeightIn)
            nHeightFirst = nHeightIn;
        if (nBlocks == 0 || nTimeFirst > nTimeIn)
            nTimeFirst = nTimeIn;

        nBlocks++;
        nHeightLast = std::max(nHeightLast, nHeightIn);
        nTimeLast = std::max(nTimeLast, nTimeIn);
    }

    std::string ToString() const;
};

// Position eines Blocks auf der Festplatte
struct CDiskBlockPos {
    int nFile{-1};
    unsigned int nPos{0};

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(VARINT(nFile));
        READWRITE(VARINT(nPos));
    }

    bool IsNull() const { return nFile == -1; }

    std::string ToString() const {
        return strprintf("CBlockDiskPos(nFile=%i, nPos=%i)", nFile, nPos);
    }
};

// Statusflags eines Blocks
enum BlockStatus : uint32_t {
    BLOCK_VALID_UNKNOWN = 0,
    BLOCK_VALID_HEADER = 1,
    BLOCK_VALID_TREE = 2,
    BLOCK_VALID_TRANSACTIONS = 3,
    BLOCK_VALID_CHAIN = 4,
    BLOCK_VALID_SCRIPTS = 5,
    BLOCK_VALID_MASK = BLOCK_VALID_HEADER | BLOCK_VALID_TREE | BLOCK_VALID_TRANSACTIONS |
                       BLOCK_VALID_CHAIN | BLOCK_VALID_SCRIPTS,

    BLOCK_HAVE_DATA = 8,
    BLOCK_HAVE_UNDO = 16,
    BLOCK_HAVE_MASK = BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO,

    BLOCK_FAILED_VALID = 32,
    BLOCK_FAILED_CHILD = 64,
    BLOCK_FAILED_MASK = BLOCK_FAILED_VALID | BLOCK_FAILED_CHILD,

    BLOCK_OPT_WITNESS = 128
};

// Index eines Blocks in der Blockchain
class CBlockIndex {
public:
    const uint256* phashBlock{nullptr};
    CBlockIndex* pprev{nullptr};
    CBlockIndex* pskip{nullptr};

    int nHeight{0};
    int nFile{0};
    unsigned int nDataPos{0};
    unsigned int nUndoPos{0};
    arith_uint256 nChainWork;
    unsigned int nTx{0};
    unsigned int nChainTx{0};
    uint32_t nStatus{0};

    int32_t nVersion{0};
    uint256 hashMerkleRoot;
    uint32_t nTime{0};
    uint32_t nBits{0};
    uint32_t nNonce{0};

    int32_t nSequenceId{0};
    unsigned int nTimeMax{0};

    explicit CBlockIndex(const CBlockHeader& block);
    CBlockIndex() = default;

    void SetNull();

    CDiskBlockPos GetBlockPos() const;
    CDiskBlockPos GetUndoPos() const;
    CBlockHeader GetBlockHeader() const;
    uint256 GetBlockHash() const;
    uint256 GetBlockPoWHash() const;

    int64_t GetBlockTime() const { return int64_t(nTime); }
    int64_t GetBlockTimeMax() const { return int64_t(nTimeMax); }

    static constexpr int nMedianTimeSpan = 11;
    int64_t GetMedianTimePast() const;

    std::string ToString() const;

    bool IsValid(BlockStatus nUpTo = BLOCK_VALID_TRANSACTIONS) const;
    bool RaiseValidity(BlockStatus nUpTo);

    void BuildSkip();
    CBlockIndex* GetAncestor(int height);
    const CBlockIndex* GetAncestor(int height) const;
};

// Auf Festplatte gespeicherter Blockindex (inkl. Hash)
class CDiskBlockIndex : public CBlockIndex {
public:
    uint256 hashPrev;

    CDiskBlockIndex() = default;
    explicit CDiskBlockIndex(const CBlockIndex* pindex);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        int _nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(VARINT(_nVersion));

        READWRITE(VARINT(nHeight));
        READWRITE(VARINT(nStatus));
        READWRITE(VARINT(nTx));
        if (nStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
            READWRITE(VARINT(nFile));
        if (nStatus & BLOCK_HAVE_DATA)
            READWRITE(VARINT(nDataPos));
        if (nStatus & BLOCK_HAVE_UNDO)
            READWRITE(VARINT(nUndoPos));

        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    uint256 GetBlockHash() const;
    std::string ToString() const;
};

// Repräsentiert die Haupt-Blockchain
class CChain {
private:
    std::vector<CBlockIndex*> vChain;

public:
    CBlockIndex* Genesis() const { return vChain.empty() ? nullptr : vChain[0]; }
    CBlockIndex* Tip() const { return vChain.empty() ? nullptr : vChain.back(); }
    int Height() const { return static_cast<int>(vChain.size()) - 1; }

    CBlockIndex* operator[](int nHeight) const {
        return (nHeight >= 0 && nHeight < static_cast<int>(vChain.size())) ? vChain[nHeight] : nullptr;
    }

    bool Contains(const CBlockIndex* pindex) const {
        return (*this)[pindex->nHeight] == pindex;
    }

    CBlockIndex* Next(const CBlockIndex* pindex) const {
        return Contains(pindex) ? (*this)[pindex->nHeight + 1] : nullptr;
    }

    void SetTip(CBlockIndex* pindex);
    CBlockLocator GetLocator(const CBlockIndex* pindex = nullptr) const;
    const CBlockIndex* FindFork(const CBlockIndex* pindex) const;
    CBlockIndex* FindEarliestAtLeast(int64_t nTime) const;

    friend bool operator==(const CChain& a, const CChain& b) {
        return a.vChain.size() == b.vChain.size() &&
               a.vChain.back() == b.vChain.back();
    }
};

// Utility-Funktionen
arith_uint256 GetBlockProof(const CBlockIndex& block);
int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params&);
const CBlockIndex* LastCommonAncestor(const CBlockIndex* pa, const CBlockIndex* pb);

#endif // NOTECHAIN_CHAIN_H
