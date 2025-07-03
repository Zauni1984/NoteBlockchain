#ifndef KAWPOW_HASH_HPP
#define KAWPOW_HASH_HPP

#include <uint256.h>
#include <vector>

/**
 * Computes the KawPoW (ProgPoW-based) hash.
 *
 * @param header     Serialized block header (without nonce/mixhash)
 * @param full_nonce 64-bit nonce (as 8-byte vector)
 * @param height      Block height (used as input to ProgPoW seed hash)
 *
 * @return 32-byte (uint256) KawPoW hash
 */
uint256 kawpow_hash(const std::vector<unsigned char>& header,
                    const std::vector<unsigned char>& full_nonce,
                    uint64_t height);

#endif // KAWPOW_HASH_HPP
