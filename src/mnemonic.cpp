// Copyright (c) 2025 Notecoin Developers
// Distributed under the MIT software license.

#include "mnemonic.h"

#include <bip39.h>
#include <random.h>
#include <utilstrencodings.h>

std::string GenerateMnemonic(int strength)
{
    // Strength must be divisible by 32 (128 = 12 words, 256 = 24 words)
    if (strength != 128 && strength != 256)
        strength = 128;

    std::vector<unsigned char> entropy(strength / 8);
    GetStrongRandBytes(entropy.data(), entropy.size());

    char* phrase = bip39_mnemonic_from_bytes(nullptr, entropy.data(), entropy.size());
    std::string result = phrase ? std::string(phrase) : "";
    if (phrase) {
        free(phrase); // Free memory allocated by bip39
    }
    return result;
}

bool IsValidMnemonic(const std::string& mnemonic)
{
    return bip39_mnemonic_check(nullptr, mnemonic.c_str());
}

std::vector<unsigned char> MnemonicToSeed(const std::string& mnemonic, const std::string& passphrase)
{
    std::vector<unsigned char> seed(64);
    bip39_mnemonic_to_seed(mnemonic.c_str(), passphrase.c_str(), seed.data(), nullptr);
    return seed;
}
