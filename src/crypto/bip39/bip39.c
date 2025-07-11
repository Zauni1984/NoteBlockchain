// bip39.c – Public domain BIP39 implementation (minimalistic)
#include "bip39.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

static const char *wordlist[] = {
#include "wordlist_english.inl" // siehe unten
};

#define MAX_WORDS 24
#define BITS_PER_WORD 11

int bip39_mnemonic_from_bytes(const uint8_t *entropy, size_t entropy_len, char *mnemonic, size_t mnemonic_len) {
    if (!entropy || !mnemonic || entropy_len < 16 || entropy_len > 32 || entropy_len % 4 != 0)
        return 0;

    size_t checksum_bits = entropy_len / 4;
    size_t total_bits = entropy_len * 8 + checksum_bits;
    size_t word_count = total_bits / BITS_PER_WORD;

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(entropy, entropy_len, hash);

    // Build bit string
    uint32_t bits[33] = {0}; // max 264 bits
    memcpy(bits, entropy, entropy_len);
    bits[entropy_len] = hash[0]; // add checksum byte

    char *out = mnemonic;
    size_t out_used = 0;
    int bit_pos = 0;

    for (size_t i = 0; i < word_count; ++i) {
        int idx = 0;
        for (int j = 0; j < BITS_PER_WORD; ++j) {
            int byte_pos = (bit_pos + j) / 8;
            int bit_offset = 7 - ((bit_pos + j) % 8);
            int bit = (bits[byte_pos] >> bit_offset) & 1;
            idx = (idx << 1) | bit;
        }
        bit_pos += BITS_PER_WORD;

        const char *word = wordlist[idx];
        size_t len = strlen(word);
        if (out_used + len + 2 > mnemonic_len) return 0;
        if (i > 0) out[out_used++] = ' ';
        memcpy(out + out_used, word, len);
        out_used += len;
    }
    out[out_used] = '\0';
    return 1;
}

int bip39_mnemonic_check(const char *mnemonic) {
    if (!mnemonic) return 0;
    char copy[256];
    strncpy(copy, mnemonic, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    char *token;
    int count = 0;

    token = strtok(copy, " ");
    while (token) {
        int found = 0;
        for (int i = 0; i < 2048; i++) {
            if (strcmp(token, wordlist[i]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
        count++;
        token = strtok(NULL, " ");
    }

    return (count == 12 || count == 15 || count == 18 || count == 21 || count == 24);
}

int bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase, uint8_t *out_seed, size_t *out_len) {
    if (!mnemonic || !out_seed) return 0;

    char salt[256] = "mnemonic";
    if (passphrase) {
        strncat(salt, passphrase, sizeof(salt) - strlen(salt) - 1);
    }

    PKCS5_PBKDF2_HMAC(mnemonic, strlen(mnemonic),
                      (unsigned char*)salt, strlen(salt),
                      2048, EVP_sha512(), 64, out_seed);

    if (out_len) *out_len = 64;
    return 1;
}
