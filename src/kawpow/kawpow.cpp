#include <kawpow/kawpow.h>
#include <util/system.h>
#include <crypto/kawpow/kawpow_hash.hpp> // Separate Implementierung (kommt noch)
#include <serialize.h>
#include <streams.h>

namespace kawpow {

uint256 HashPoW(const CBlockHeader& block)
{
    // Convert header to byte stream (without nonce and mixhash)
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << block.nVersion;
    ss << block.hashPrevBlock;
    ss << block.hashMerkleRoot;
    ss << block.nTime;
    ss << block.nBits;

    // Combine full header with nonce
    std::vector<unsigned char> header_data(ss.begin(), ss.end());

    // KawPoW expects a full 256-bit nonce (here: expand from uint32_t nonce)
    std::vector<unsigned char> full_nonce(8, 0x00);
    memcpy(&full_nonce[0], &block.nNonce, sizeof(block.nNonce));

    // Use fake block height for now, can later pass in real height if needed
    uint64_t block_height = 0;

    // Calculate final hash using KawPoW
    return kawpow_hash(header_data, full_nonce, block_height);
}

} // namespace kawpow
