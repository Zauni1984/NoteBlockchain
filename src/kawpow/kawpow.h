#ifndef KAWPOW_H
#define KAWPOW_H

#include <uint256.h>
#include <primitives/block.h>

namespace kawpow {

/**
 * Computes the KawPoW hash for a given block header.
 */
uint256 HashPoW(const CBlockHeader& block);

} // namespace kawpow

#endif // KAWPOW_H
