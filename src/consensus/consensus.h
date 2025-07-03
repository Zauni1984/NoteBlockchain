// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_CONSENSUS_H
#define BITCOIN_CONSENSUS_CONSENSUS_H

#include <stdint.h>

/** Maximum serialized block size (buffer limit only) */
static constexpr unsigned int MAX_BLOCK_SERIALIZED_SIZE = 4000000;

/** Maximum allowed block weight as per BIP 141 (network consensus rule) */
static constexpr unsigned int MAX_BLOCK_WEIGHT = 4000000;

/** Maximum allowed number of signature verification operations per block */
static constexpr int64_t MAX_BLOCK_SIGOPS_COST = 80000;

/** Coinbase transaction maturity: outputs can be spent after this many new blocks */
static constexpr int COINBASE_MATURITY = 100;

/** Witness data scale factor as per BIP 141 */
static constexpr int WITNESS_SCALE_FACTOR = 4;

/** Minimum weight of a valid transaction (non-zero, includes overhead) */
static constexpr size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60;

/** Minimum weight to be a serializable transaction (arbitrary internal safety check) */
static constexpr size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10;

/** nSequence/nLockTime flags */
/** Interpret sequence numbers as relative lock-time constraints (BIP 68) */
static constexpr unsigned int LOCKTIME_VERIFY_SEQUENCE = (1 << 0);

/** Use median past time instead of block timestamp (BIP 113) */
static constexpr unsigned int LOCKTIME_MEDIAN_TIME_PAST = (1 << 1);

#endif // BITCOIN_CONSENSUS_CONSENSUS_H
