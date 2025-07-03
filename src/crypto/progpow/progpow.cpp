#include <crypto/progpow/progpow.hpp>
#include <crypto/keccak.h> // vorhandene Keccak-Implementation (SHA3)

namespace progpow {

// Dummy seed hash generator (real implementation uses fixed table)
static uint256 get_seed_hash(uint64_t height)
{
    // Simplified: just hash the height into a 256-bit seed
    CHash256 hasher;
    hasher.Write((const unsigned char*)&height, sizeof(height));
    uint256 seed;
    hasher.Finalize(seed.begin());
    return seed;
}

uint256 hash(const uint256& header_hash, uint64_t nonce, uint64_t height)
{
    // Generate seed hash for height
    uint256 seed_hash = get_seed_hash(height);

    // Combine header_hash + nonce + seed_hash into final hash
    CHash256 hasher;
    hasher.Write(header_hash.begin(), header_hash.size());
    hasher.Write((const unsigned char*)&nonce, sizeof(nonce));
    hasher.Write(seed_hash.begin(), seed_hash.size());

    uint256 result;
    hasher.Finalize(result.begin());
    return result;
}

} // namespace progpow
