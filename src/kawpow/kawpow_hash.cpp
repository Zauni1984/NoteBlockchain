#include <kawpow/kawpow_hash.hpp>
#include <crypto/progpow/progpow.hpp>  // externe Lib – implementieren wir gleich
#include <cstring>

uint256 kawpow_hash(const std::vector<unsigned char>& header,
                    const std::vector<unsigned char>& full_nonce,
                    uint64_t height)
{
    // ProgPoW needs:
    // - header_hash (uint256)
    // - nonce (uint64_t)
    // - height (for seed)

    if (header.size() < 80) {
        // Not a valid block header
        return uint256();
    }

    uint256 header_hash = Hash(header.begin(), header.end()); // double SHA256
    uint64_t nonce = 0;
    memcpy(&nonce, &full_nonce[0], sizeof(uint64_t));

    // Run KawPoW (ProgPoW) algo
    uint256 result_hash = progpow::hash(header_hash, nonce, height);

    return result_hash;
}
