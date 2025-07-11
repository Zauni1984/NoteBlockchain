// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHECKPOINTS_H
#define BITCOIN_CHECKPOINTS_H

#include <uint256.h>

#include <map>

class CBlockIndex;
struct CCheckpointData;

/**
 * Namespace for blockchain checkpoints, used as compiled-in sanity checks.
 * These are periodically updated to reflect known good blocks at certain heights.
 */
namespace Checkpoints {

/**
 * Returns the last CBlockIndex* in mapBlockIndex that matches a checkpoint hash.
 * Used to speed up reindexing and validation.
 *
 * @param data Reference to checkpoint data set
 * @return Pointer to last valid checkpoint block index, or nullptr if none found
 */
CBlockIndex* GetLastCheckpoint(const CCheckpointData& data);

} // namespace Checkpoints

#endif // BITCOIN_CHECKPOINTS_H
