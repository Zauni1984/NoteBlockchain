// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTE_AMOUNT_H
#define NOTE_AMOUNT_H

#include <cstdint>

//! Basic type for currency amount (satoshis)
using CAmount = int64_t;

//! Conversion constants
static constexpr CAmount COIN = 100'000'000;
static constexpr CAmount CENT = 1'000'000;

//! Initial block reward
static constexpr CAmount INITIAL_REWARD = 4'756;

//! Block interval for yearly halving (e.g., 2-minute blocks → ~1051200 blocks/year)
static constexpr int YEARLY_INTERVAL = 1'051'200;

//! Maximum valid amount (consensus-critical sanity check)
static constexpr CAmount MAX_MONEY = 55'000'000'000LL * COIN;

//! Verify if amount is within the valid monetary range
inline bool MoneyRange(const CAmount& nValue) {
    return nValue >= 0 && nValue <= MAX_MONEY;
}

#endif // NOTE_AMOUNT_H
