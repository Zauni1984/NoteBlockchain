// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <checkpoints.h>

#include <chain.h>
#include <chainparams.h>
#include <reverse_iterator.h>
#include <validation.h>

namespace Checkpoints {

//! Find the most recent checkpoint block that exists in the current block index map
CBlockIndex* GetLastCheckpoint(const CCheckpointData& data)
{
    const MapCheckpoints& checkpoints = data.mapCheckpoints;

    for (const auto& [height, hash] : reverse_iterate(checkpoints)) {
        const auto it = mapBlockIndex.find(hash);
        if (it != mapBlockIndex.end()) {
            return it->second;
        }
    }

    return nullptr;
}

} // namespace Checkpoints
