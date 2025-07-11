// Copyright (c) 2025 Notecoin Developers
// Distributed under the MIT software license.

#ifndef NOTECOIN_MNEMONIC_H
#define NOTECOIN_MNEMONIC_H

#include <string>
#include <vector>

/**
 * Generate a new BIP39 mnemonic phrase (typically 12 or 24 words).
 * @param strength Entropy strength in bits (128 = 12 words, 256 = 24 words)
 * @return Human-readable mnemonic phrase
 */
std::string GenerateMnemonic(int strength = 128);

/**
 * Validate if a mnemonic is a valid BIP39 phrase
 * @param mnemonic The mnemonic phrase to validate
 * @return true if valid, false otherwise
 */
bool IsValidMnemonic(const std::string& mnemonic);

/**
 * Convert mnemonic to binary seed
 * @param mnemonic The BIP39 mnemonic phrase
 * @param passphrase Optional passphrase ("mnemonic" by default)
 * @return A 512-bit seed vector
 */
std::vector<unsigned char> MnemonicToSeed(const std::string& mnemonic, const std::string& passphrase = "mnemonic");

#endif // NOTECOIN_MNEMONIC_H
