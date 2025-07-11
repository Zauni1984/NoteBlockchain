// Bech32 Encoder/Decoder Interface
// Copyright (c) 2017 Pieter Wuille
// Distributed under the MIT software license.

#ifndef NOTECHAIN_BECH32_H
#define NOTECHAIN_BECH32_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * Bech32 is a string encoding format defined in BIP 173.
 * It is used to encode SegWit and other newer address formats.
 * 
 * Format: <human-readable-part> + '1' + <base32-data> + <6-char-checksum>
 */

namespace bech32 {

/**
 * Encode a Bech32 string.
 * 
 * @param hrp     Human-readable part (e.g., "bc", "tb")
 * @param values  Data payload (typically converted witness program)
 * @return        Bech32-encoded string or empty string on failure
 */
std::string Encode(const std::string& hrp, const std::vector<uint8_t>& values);

/**
 * Decode a Bech32 string.
 * 
 * @param str     Bech32 string
 * @return        Pair of (human-readable part, data payload);
 *                empty hrp string indicates failure
 */
std::pair<std::string, std::vector<uint8_t>> Decode(const std::string& str);

} // namespace bech32

#endif // NOTECHAIN_BECH32_H
