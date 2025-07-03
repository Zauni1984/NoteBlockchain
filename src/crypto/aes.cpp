// Copyright (c) 2016-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <crypto/aes.h>
#include <crypto/common.h>

#include <cstring>

extern "C" {
#include <crypto/ctaes/ctaes.c>
}

AES128Encrypt::AES128Encrypt(const unsigned char key[16]) {
    AES128_init(&ctx, key);
}

AES128Encrypt::~AES128Encrypt() {
    std::memset(&ctx, 0, sizeof(ctx));
}

void AES128Encrypt::Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const {
    AES128_encrypt(&ctx, 1, ciphertext, plaintext);
}

AES128Decrypt::AES128Decrypt(const unsigned char key[16]) {
    AES128_init(&ctx, key);
}

AES128Decrypt::~AES128Decrypt() {
    std::memset(&ctx, 0, sizeof(ctx));
}

void AES128Decrypt::Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const {
    AES128_decrypt(&ctx, 1, plaintext, ciphertext);
}

AES256Encrypt::AES256Encrypt(const unsigned char key[32]) {
    AES256_init(&ctx, key);
}

AES256Encrypt::~AES256Encrypt() {
    std::memset(&ctx, 0, sizeof(ctx));
}

void AES256Encrypt::Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const {
    AES256_encrypt(&ctx, 1, ciphertext, plaintext);
}

AES256Decrypt::AES256Decrypt(const unsigned char key[32]) {
    AES256_init(&ctx, key);
}

AES256Decrypt::~AES256Decrypt() {
    std::memset(&ctx, 0, sizeof(ctx));
}

void AES256Decrypt::Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const {
    AES256_decrypt(&ctx, 1, plaintext, ciphertext);
}

template <typename T>
static int CBCEncrypt(const T& enc, const unsigned char iv[AES_BLOCKSIZE], const unsigned char* data, int size, bool pad, unsigned char* out) {
    int written = 0;
    int padsize = size % AES_BLOCKSIZE;
    unsigned char mixed[AES_BLOCKSIZE];

    if (!data || !size || !out) return 0;
    if (!pad && padsize != 0) return 0;

    std::memcpy(mixed, iv, AES_BLOCKSIZE);

    while (written + AES_BLOCKSIZE <= size) {
        for (int i = 0; i < AES_BLOCKSIZE; i++)
            mixed[i] ^= *data++;
        enc.Encrypt(out + written, mixed);
        std::memcpy(mixed, out + written, AES_BLOCKSIZE);
        written += AES_BLOCKSIZE;
    }

    if (pad) {
        for (int i = 0; i < padsize; i++)
            mixed[i] ^= *data++;
        for (int i = padsize; i < AES_BLOCKSIZE; i++)
            mixed[i] ^= AES_BLOCKSIZE - padsize;
        enc.Encrypt(out + written, mixed);
        written += AES_BLOCKSIZE;
    }

    return written;
}

template <typename T>
static int CBCDecrypt(const T& dec, const unsigned char iv[AES_BLOCKSIZE], const unsigned char* data, int size, bool pad, unsigned char* out) {
    int written = 0;
    bool fail = false;
    const unsigned char* prev = iv;

    if (!data || !size || !out) return 0;
    if (size % AES_BLOCKSIZE != 0) return 0;

    while (written != size) {
        dec.Decrypt(out, data + written);
        for (int i = 0; i < AES_BLOCKSIZE; i++)
            *out++ ^= prev[i];
        prev = data + written;
        written += AES_BLOCKSIZE;
    }

    if (pad) {
        unsigned char padsize = *--out;
        fail = !padsize | (padsize > AES_BLOCKSIZE);
        padsize *= !fail;

        for (int i = AES_BLOCKSIZE; i > 0; i--)
            fail |= ((i > AES_BLOCKSIZE - padsize) & (*out-- != padsize));

        written -= padsize;
    }

    return written * !fail;
}

AES256CBCEncrypt::AES256CBCEncrypt(const unsigned char key[AES256_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn)
    : enc(key), pad(padIn) {
    std::memcpy(iv, ivIn, AES_BLOCKSIZE);
}

AES256CBCEncrypt::~AES256CBCEncrypt() {
    std::memset(iv, 0, sizeof(iv));
}

int AES256CBCEncrypt::Encrypt(const unsigned char* data, int size, unsigned char* out) const {
    return CBCEncrypt(enc, iv, data, size, pad, out);
}

AES256CBCDecrypt::AES256CBCDecrypt(const unsigned char key[AES256_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn)
    : dec(key), pad(padIn) {
    std::memcpy(iv, ivIn, AES_BLOCKSIZE);
}

AES256CBCDecrypt::~AES256CBCDecrypt() {
    std::memset(iv, 0, sizeof(iv));
}

int AES256CBCDecrypt::Decrypt(const unsigned char* data, int size, unsigned char* out) const {
    return CBCDecrypt(dec, iv, data, size, pad, out);
}

AES128CBCEncrypt::AES128CBCEncrypt(const unsigned char key[AES128_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn)
    : enc(key), pad(padIn) {
    std::memcpy(iv, ivIn, AES_BLOCKSIZE);
}

AES128CBCEncrypt::~AES128CBCEncrypt() {
    std::memset(iv, 0, AES_BLOCKSIZE);
}

int AES128CBCEncrypt::Encrypt(const unsigned char* data, int size, unsigned char* out) const {
    return CBCEncrypt(enc, iv, data, size, pad, out);
}

AES128CBCDecrypt::AES128CBCDecrypt(const unsigned char key[AES128_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn)
    : dec(key), pad(padIn) {
    std::memcpy(iv, ivIn, AES_BLOCKSIZE);
}

AES128CBCDecrypt::~AES128CBCDecrypt() {
    std::memset(iv, 0, AES_BLOCKSIZE);
}

int AES128CBCDecrypt::Decrypt(const unsigned char* data, int size, unsigned char* out) const {
    return CBCDecrypt(dec, iv, data, size, pad, out);
}
