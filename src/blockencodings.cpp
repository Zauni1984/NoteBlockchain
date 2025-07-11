// Copyright (c) 2016-2024 NoteCoin Core Developers
// Distributed under the MIT software license, see COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <blockencodings.h>
#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <chainparams.h>
#include <hash.h>
#include <random.h>
#include <streams.h>
#include <txmempool.h>
#include <validation.h>
#include <util/system.h>

#include <unordered_map>
#include <limits>

CBlockHeaderAndShortTxIDs::CBlockHeaderAndShortTxIDs(const CBlock& block, bool fUseWTXID)
    : nonce(GetRand(std::numeric_limits<uint64_t>::max())),
      shorttxids(block.vtx.size() - 1),
      prefilledtxn(1),
      header(block)
{
    FillShortTxIDSelector();
    prefilledtxn[0] = {0, block.vtx[0]}; // coinbase

    for (size_t i = 1; i < block.vtx.size(); ++i) {
        const CTransaction& tx = *block.vtx[i];
        shorttxids[i - 1] = GetShortID(fUseWTXID ? tx.GetWitnessHash() : tx.GetHash());
    }
}

void CBlockHeaderAndShortTxIDs::FillShortTxIDSelector() const
{
    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << header << nonce;

    CSHA256 hasher;
    hasher.Write(UCHARVECTOR(stream).data(), stream.size());

    uint256 shorttxidhash;
    hasher.Finalize(shorttxidhash.begin());
    shorttxidk0 = shorttxidhash.GetUint64(0);
    shorttxidk1 = shorttxidhash.GetUint64(1);
}

uint64_t CBlockHeaderAndShortTxIDs::GetShortID(const uint256& txhash) const
{
    static_assert(SHORTTXIDS_LENGTH == 6, "Expected 6-byte shorttxids");
    return SipHashUint256(shorttxidk0, shorttxidk1, txhash) & 0xffffffffffffULL;
}

ReadStatus PartiallyDownloadedBlock::InitData(const CBlockHeaderAndShortTxIDs& cmpctblock,
                                              const std::vector<std::pair<uint256, CTransactionRef>>& extra_txn)
{
    if (cmpctblock.header.IsNull() ||
        (cmpctblock.shorttxids.empty() && cmpctblock.prefilledtxn.empty()))
        return READ_STATUS_INVALID;

    if (cmpctblock.shorttxids.size() + cmpctblock.prefilledtxn.size() > MAX_BLOCK_WEIGHT / MIN_SERIALIZABLE_TRANSACTION_WEIGHT)
        return READ_STATUS_INVALID;

    assert(header.IsNull() && txn_available.empty());
    header = cmpctblock.header;
    txn_available.resize(cmpctblock.BlockTxCount());

    int32_t lastprefilledindex = -1;
    for (const auto& prefilled : cmpctblock.prefilledtxn) {
        if (prefilled.tx->IsNull()) return READ_STATUS_INVALID;

        lastprefilledindex += prefilled.index + 1;
        if (lastprefilledindex > std::numeric_limits<uint16_t>::max()) return READ_STATUS_INVALID;
        if ((uint32_t)lastprefilledindex > cmpctblock.shorttxids.size() + prefilled_count)
            return READ_STATUS_INVALID;

        txn_available[lastprefilledindex] = prefilled.tx;
    }

    prefilled_count = cmpctblock.prefilledtxn.size();

    std::unordered_map<uint64_t, uint16_t> shorttxids;
    shorttxids.reserve(cmpctblock.shorttxids.size());

    uint16_t index_offset = 0;
    for (size_t i = 0; i < cmpctblock.shorttxids.size(); ++i) {
        while (txn_available[i + index_offset]) ++index_offset;
        shorttxids[cmpctblock.shorttxids[i]] = i + index_offset;

        if (shorttxids.bucket_size(shorttxids.bucket(cmpctblock.shorttxids[i])) > 12)
            return READ_STATUS_FAILED;
    }

    if (shorttxids.size() != cmpctblock.shorttxids.size())
        return READ_STATUS_FAILED; // collision

    std::vector<bool> have_txn(txn_available.size());

    {
        LOCK(pool->cs);
        for (const auto& txinfo : pool->vTxHashes) {
            uint64_t shortid = cmpctblock.GetShortID(txinfo.first);
            auto it = shorttxids.find(shortid);
            if (it != shorttxids.end()) {
                auto& pos = it->second;
                if (!have_txn[pos]) {
                    txn_available[pos] = txinfo.second->GetSharedTx();
                    have_txn[pos] = true;
                    mempool_count++;
                } else {
                    if (txn_available[pos]) {
                        txn_available[pos].reset();
                        mempool_count--;
                    }
                }
            }
            if (mempool_count == shorttxids.size()) break;
        }
    }

    for (const auto& extx : extra_txn) {
        uint64_t shortid = cmpctblock.GetShortID(extx.first);
        auto it = shorttxids.find(shortid);
        if (it != shorttxids.end()) {
            auto& pos = it->second;
            if (!have_txn[pos]) {
                txn_available[pos] = extx.second;
                have_txn[pos] = true;
                mempool_count++;
                extra_count++;
            } else if (txn_available[pos] &&
                       txn_available[pos]->GetWitnessHash() != extx.second->GetWitnessHash()) {
                txn_available[pos].reset();
                mempool_count--;
                extra_count--;
            }
        }
        if (mempool_count == shorttxids.size()) break;
    }

    LogPrint(BCLog::CMPCTBLOCK, "Initialized PartiallyDownloadedBlock for block %s using cmpctblock size %lu\n",
             cmpctblock.header.GetHash().ToString(),
             GetSerializeSize(cmpctblock, SER_NETWORK, PROTOCOL_VERSION));

    return READ_STATUS_OK;
}

bool PartiallyDownloadedBlock::IsTxAvailable(size_t index) const
{
    assert(!header.IsNull());
    assert(index < txn_available.size());
    return txn_available[index] != nullptr;
}

ReadStatus PartiallyDownloadedBlock::FillBlock(CBlock& block,
                                               const std::vector<CTransactionRef>& vtx_missing)
{
    assert(!header.IsNull());
    block = header;
    block.vtx.resize(txn_available.size());

    size_t missing_idx = 0;
    for (size_t i = 0; i < txn_available.size(); ++i) {
        if (!txn_available[i]) {
            if (missing_idx >= vtx_missing.size())
                return READ_STATUS_INVALID;
            block.vtx[i] = vtx_missing[missing_idx++];
        } else {
            block.vtx[i] = std::move(txn_available[i]);
        }
    }

    header.SetNull();
    txn_available.clear();

    if (vtx_missing.size() != missing_idx)
        return READ_STATUS_INVALID;

    CValidationState state;
    if (!CheckBlock(block, state, Params().GetConsensus())) {
        return state.CorruptionPossible() ? READ_STATUS_FAILED : READ_STATUS_CHECKBLOCK_FAILED;
    }

    LogPrint(BCLog::CMPCTBLOCK,
             "Reconstructed block %s: %lu prefilled, %lu from mempool (incl. %lu extra), %lu requested\n",
             block.GetHash().ToString(), prefilled_count, mempool_count, extra_count, vtx_missing.size());

    if (vtx_missing.size() < 5) {
        for (const auto& tx : vtx_missing) {
            LogPrint(BCLog::CMPCTBLOCK, "Missing TX included: %s\n", tx->GetHash().ToString());
        }
    }

    return READ_STATUS_OK;
}
