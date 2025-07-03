#ifndef PROGPOW_HASH_HPP
#define PROGPOW_HASH_HPP

#include <uint256.h>
#include <cstdint>

namespace progpow {

/**
 * Computes the KawPoW (ProgPoW) hash for block verification.
 *
 * @param header_hash  Double-SHA256 of the block header
 * @param nonce        64-bit block nonce
 * @param height       Block height (used for seed)
 *
 * @return Final KawPoW result (uint256)
 */
uint256 hash(const uint256& header_hash, uint64_t nonce, uint64_t height);

} // namespace progpow

#endif // PROGPOW_HASH_HPP
