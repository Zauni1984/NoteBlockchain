// Copyright (c) 2016-2024 NoteCoin Core Developers
// Distributed under the MIT software license. See COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTECOIN_BLOCK_ENCODINGS_H
#define NOTECOIN_BLOCK_ENCODINGS_H

#include <primitives/block.h>
#include <memory>
#include <limits>

class CTxMemPool;

//! Helper for compressing transactions during (de)serialization
struct TransactionCompressor {
private:
    CTransactionRef& tx;

public:
    explicit TransactionCompressor(CTransactionRef& txIn) : tx(txIn) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(tx); // TODO: Replace with actual compression
    }
};

//! Request specific transactions from a compact block
class BlockTransactionsRequest {
public:
    uint256 blockhash;
    std::vector<uint16_t> indexes;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(blockhash);
        uint64_t size = indexes.size();
        READWRITE(COMPACTSIZE(size));

        if (ser_action.ForRead()) {
            size_t i = 0;
            while (indexes.size() < size) {
                indexes.resize(std::min<uint64_t>(1000 + indexes.size(), size));
                for (; i < indexes.size(); ++i) {
                    uint64_t delta = 0;
                    READWRITE(COMPACTSIZE(delta));
                    if (delta > std::numeric_limits<uint16_t>::max())
                        throw std::ios_base::failure("index overflowed 16 bits");
                    indexes[i] = delta;
                }
            }

            uint16_t offset = 0;
            for (auto& idx : indexes) {
                if (uint64_t(idx) + offset > std::numeric_limits<uint16_t>::max())
                    throw std::ios_base::failure("indexes overflowed 16 bits");
                idx += offset;
                offset = idx + 1;
            }
        } else {
            for (size_t i = 0; i < indexes.size(); ++i) {
                uint64_t delta = indexes[i] - (i == 0 ? 0 : (indexes[i - 1] + 1));
                READWRITE(COMPACTSIZE(delta));
            }
        }
    }
};

//! Compact message for sending actual transactions of a block
class BlockTransactions {
public:
    uint256 blockhash;
    std::vector<CTransactionRef> txn;

    BlockTransactions() = default;
    explicit BlockTransactions(const BlockTransactionsRequest& req)
        : blockhash(req.blockhash), txn(req.indexes.size()) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(blockhash);
        uint64_t size = txn.size();
        READWRITE(COMPACTSIZE(size));

        if (ser_action.ForRead()) {
            size_t i = 0;
            while (txn.size() < size) {
                txn.resize(std::min<uint64_t>(1000 + txn.size(), size));
                for (; i < txn.size(); ++i)
                    READWRITE(REF(TransactionCompressor(txn[i])));
            }
        } else {
            for (auto& tx : txn)
                READWRITE(REF(TransactionCompressor(tx)));
        }
    }
};

//! Represents a prefilled transaction used in CBlockHeaderAndShortTxIDs
struct PrefilledTransaction {
    uint16_t index;
    CTransactionRef tx;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        uint64_t idx = index;
        READWRITE(COMPACTSIZE(idx));
        if (idx > std::numeric_limits<uint16_t>::max())
            throw std::ios_base::failure("index overflowed 16 bits");
        index = static_cast<uint16_t>(idx);
        READWRITE(REF(TransactionCompressor(tx)));
    }
};

//! Possible results when trying to reconstruct a block
enum ReadStatus_t {
    READ_STATUS_OK = 0,
    READ_STATUS_INVALID,
    READ_STATUS_FAILED,
    READ_STATUS_CHECKBLOCK_FAILED
};

class CBlockHeaderAndShortTxIDs {
private:
    mutable uint64_t shorttxidk0 = 0;
    mutable uint64_t shorttxidk1 = 0;
    uint64_t nonce = 0;

    void FillShortTxIDSelector() const;

    friend class PartiallyDownloadedBlock;

    static const int SHORTTXIDS_LENGTH = 6;

protected:
    std::vector<uint64_t> shorttxids;
    std::vector<PrefilledTransaction> prefilledtxn;

public:
    CBlockHeader header;

    CBlockHeaderAndShortTxIDs() = default;
    CBlockHeaderAndShortTxIDs(const CBlock& block, bool fUseWTXID);

    uint64_t GetShortID(const uint256& txhash) const;

    size_t BlockTxCount() const {
        return shorttxids.size() + prefilledtxn.size();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(header);
        READWRITE(nonce);

        uint64_t shortids_size = shorttxids.size();
        READWRITE(COMPACTSIZE(shortids_size));

        if (ser_action.ForRead()) {
            size_t i = 0;
            while (shorttxids.size() < shortids_size) {
                shorttxids.resize(std::min<uint64_t>(1000 + shorttxids.size(), shortids_size));
                for (; i < shorttxids.size(); ++i) {
                    uint32_t lsb = 0;
                    uint16_t msb = 0;
                    READWRITE(lsb);
                    READWRITE(msb);
                    shorttxids[i] = (static_cast<uint64_t>(msb) << 32) | lsb;
                }
            }
        } else {
            for (const auto& id : shorttxids) {
                uint32_t lsb = static_cast<uint32_t>(id & 0xFFFFFFFF);
                uint16_t msb = static_cast<uint16_t>((id >> 32) & 0xFFFF);
                READWRITE(lsb);
                READWRITE(msb);
            }
        }

        READWRITE(prefilledtxn);

        if (ser_action.ForRead())
            FillShortTxIDSelector();
    }
};

//! Manages the reconstruction of blocks from compact representations
class PartiallyDownloadedBlock {
protected:
    std::vector<CTransactionRef> txn_available;
    size_t prefilled_count = 0;
    size_t mempool_count = 0;
    size_t extra_count = 0;
    CTxMemPool* pool = nullptr;

public:
    CBlockHeader header;

    explicit PartiallyDownloadedBlock(CTxMemPool* poolIn)
        : pool(poolIn) {}

    ReadStatus InitData(const CBlockHeaderAndShortTxIDs& cmpctblock,
                        const std::vector<std::pair<uint256, CTransactionRef>>& extra_txn);

    bool IsTxAvailable(size_t index) const;

    ReadStatus FillBlock(CBlock& block,
                         const std::vector<CTransactionRef>& vtx_missing);
};

#endif // NOTECOIN_BLOCK_ENCODINGS_H
