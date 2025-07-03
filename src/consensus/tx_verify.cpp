// Copyright (c) 2017-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_verify.h>
#include <consensus/consensus.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <consensus/validation.h>
#include <chain.h>
#include <coins.h>

bool IsFinalTx(const CTransaction& tx, int nBlockHeight, int64_t nBlockTime)
{
    if (tx.nLockTime == 0)
        return true;

    const int64_t lockTarget = (tx.nLockTime < LOCKTIME_THRESHOLD) ? nBlockHeight : nBlockTime;
    if (tx.nLockTime < lockTarget)
        return true;

    for (const auto& txin : tx.vin) {
        if (txin.nSequence != CTxIn::SEQUENCE_FINAL)
            return false;
    }

    return true;
}

std::pair<int, int64_t> CalculateSequenceLocks(const CTransaction& tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block)
{
    assert(prevHeights->size() == tx.vin.size());

    int minHeight = -1;
    int64_t minTime = -1;

    bool enforceBIP68 = static_cast<uint32_t>(tx.nVersion) >= 2 && (flags & LOCKTIME_VERIFY_SEQUENCE);
    if (!enforceBIP68)
        return {minHeight, minTime};

    for (size_t i = 0; i < tx.vin.size(); ++i) {
        const CTxIn& txin = tx.vin[i];

        if (txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_DISABLE_FLAG) {
            (*prevHeights)[i] = 0;
            continue;
        }

        int coinHeight = (*prevHeights)[i];

        if (txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG) {
            int64_t coinTime = block.GetAncestor(std::max(coinHeight - 1, 0))->GetMedianTimePast();
            minTime = std::max(minTime, coinTime + ((txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_MASK) << CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) - 1);
        } else {
            minHeight = std::max(minHeight, coinHeight + static_cast<int>(txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_MASK) - 1);
        }
    }

    return {minHeight, minTime};
}

bool EvaluateSequenceLocks(const CBlockIndex& block, std::pair<int, int64_t> lockPair)
{
    assert(block.pprev);
    return (lockPair.first < block.nHeight && lockPair.second < block.pprev->GetMedianTimePast());
}

bool SequenceLocks(const CTransaction& tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block)
{
    return EvaluateSequenceLocks(block, CalculateSequenceLocks(tx, flags, prevHeights, block));
}

unsigned int GetLegacySigOpCount(const CTransaction& tx)
{
    unsigned int count = 0;
    for (const auto& txin : tx.vin)
        count += txin.scriptSig.GetSigOpCount(false);
    for (const auto& txout : tx.vout)
        count += txout.scriptPubKey.GetSigOpCount(false);
    return count;
}

unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& inputs)
{
    if (tx.IsCoinBase())
        return 0;

    unsigned int count = 0;
    for (unsigned int i = 0; i < tx.vin.size(); ++i) {
        const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
        assert(!coin.IsSpent());
        const CTxOut& prevout = coin.out;

        if (prevout.scriptPubKey.IsPayToScriptHash())
            count += prevout.scriptPubKey.GetSigOpCount(tx.vin[i].scriptSig);
    }
    return count;
}

int64_t GetTransactionSigOpCost(const CTransaction& tx, const CCoinsViewCache& inputs, int flags)
{
    int64_t cost = GetLegacySigOpCount(tx) * WITNESS_SCALE_FACTOR;

    if (tx.IsCoinBase())
        return cost;

    if (flags & SCRIPT_VERIFY_P2SH)
        cost += GetP2SHSigOpCount(tx, inputs) * WITNESS_SCALE_FACTOR;

    for (unsigned int i = 0; i < tx.vin.size(); ++i) {
        const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
        assert(!coin.IsSpent());
        const CTxOut& prevout = coin.out;
        cost += CountWitnessSigOps(tx.vin[i].scriptSig, prevout.scriptPubKey, &tx.vin[i].scriptWitness, flags);
    }

    return cost;
}

bool CheckTransaction(const CTransaction& tx, CValidationState& state, bool fCheckDuplicateInputs)
{
    if (tx.vin.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vin-empty");

    if (tx.vout.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vout-empty");

    if (::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT)
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-oversize");

    CAmount nValueOut = 0;
    for (const auto& txout : tx.vout) {
        if (txout.nValue < 0)
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-negative");
        if (txout.nValue > MAX_MONEY)
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-toolarge");

        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-txouttotal-toolarge");
    }

    if (fCheckDuplicateInputs) {
        std::set<COutPoint> inPoints;
        for (const auto& txin : tx.vin) {
            if (!inPoints.insert(txin.prevout).second)
                return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputs-duplicate");
        }
    }

    if (tx.IsCoinBase()) {
        if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100)
            return state.DoS(100, false, REJECT_INVALID, "bad-cb-length");
    } else {
        for (const auto& txin : tx.vin) {
            if (txin.prevout.IsNull())
                return state.DoS(10, false, REJECT_INVALID, "bad-txns-prevout-null");
        }
    }

    return true;
}

bool Consensus::CheckTxInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& inputs, int nSpendHeight, CAmount& txfee)
{
    if (!inputs.HaveInputs(tx)) {
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputs-missingorspent", false,
                         strprintf("%s: inputs missing/spent", __func__));
    }

    CAmount nValueIn = 0;
    for (unsigned int i = 0; i < tx.vin.size(); ++i) {
        const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
        assert(!coin.IsSpent());

        if (coin.IsCoinBase() && nSpendHeight - coin.nHeight < COINBASE_MATURITY) {
            return state.Invalid(false, REJECT_INVALID, "bad-txns-premature-spend-of-coinbase",
                                 strprintf("tried to spend coinbase at depth %d", nSpendHeight - coin.nHeight));
        }

        nValueIn += coin.out.nValue;
        if (!MoneyRange(coin.out.nValue) || !MoneyRange(nValueIn)) {
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputvalues-outofrange");
        }
    }

    const CAmount valueOut = tx.GetValueOut();
    if (nValueIn < valueOut) {
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-in-belowout", false,
                         strprintf("value in (%s) < value out (%s)", FormatMoney(nValueIn), FormatMoney(valueOut)));
    }

    txfee = nValueIn - valueOut;
    if (!MoneyRange(txfee))
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-fee-outofrange");

    return true;
}
