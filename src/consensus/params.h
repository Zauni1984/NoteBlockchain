// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include <uint256.h>
#include <limits>

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    DEPLOYMENT_CSV,     // Deployment of BIP68, BIP112, and BIP113.
    DEPLOYMENT_SEGWIT,  // Deployment of BIP141, BIP143, and BIP147.
    // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
    MAX_VERSION_BITS_DEPLOYMENTS
};

/**
 * Struct for each individual consensus rule change using BIP9.
 */
struct BIP9Deployment {
    /** Bit position to select the particular bit in nVersion. */
    int bit;
    /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
    int64_t nStartTime;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout;

    /** Constant for nTimeout very far in the future. */
    static constexpr int64_t NO_TIMEOUT = std::numeric_limits<int64_t>::max();

    /** Special value for nStartTime indicating that the deployment is always active.
     *  Useful for tests that should bypass activation mechanics.
     */
    static constexpr int64_t ALWAYS_ACTIVE = -1;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    int nSubsidyHalvingInterval;

    /** Block height at which BIP16 becomes active */
    int BIP16Height;

    /** Block height and hash at which BIP34 becomes active */
    int BIP34Height;
    uint256 BIP34Hash;

    /** Block height at which BIP65 becomes active */
    int BIP65Height;

    /** Block height at which BIP66 becomes active */
    int BIP66Height;

    /**
     * Minimum number of blocks including miner confirmation of the total in a retargeting period,
     * (nPowTargetTimespan / nPowTargetSpacing), also used for BIP9 deployments.
     * Examples: 1916 for 95%, 1512 for testnets.
     */
    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;

    /** BIP9 deployments */
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];

    /** Proof of Work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
    int64_t nPowTargetTimespan;

    int64_t DifficultyAdjustmentInterval() const {
        return nPowTargetTimespan / nPowTargetSpacing;
    }

    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    /** DigiShield parameters */
    int64_t nAveragingInterval;
    int64_t nMaxAdjustDownV4;
    int64_t nMaxAdjustUpV4;
    int64_t multiAlgoTargetSpacingV4;
    int64_t nAveragingTargetTimespanV4;
    int64_t nMaxActualTimespanV4;
    int64_t nLocalTargetAdjustment;
    int nMinActualTimespanV4;

    /** Block height at which DigiShield becomes active */
    int nDigiShieldHFHeight;
};

} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
