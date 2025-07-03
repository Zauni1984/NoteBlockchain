// Copyright (c) 2015-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_AES_H
#define BITCOIN_CRYPTO_AES_H

extern "C" {
#include <crypto/ctaes/ctaes.h>
}

constexpr int AES_BLOCKSIZE = 16;
constexpr int AES128_KEYSIZE = 16;
constexpr int AES256_KEYSIZE = 32;

/** AES-128 encryption */
class AES128Encrypt {
private:
    AES128_ctx ctx;

public:
    explicit AES128Encrypt(const unsigned char key[16]);
    ~AES128Encrypt();
    void Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const;
};

/** AES-128 decryption */
class AES128Decrypt {
private:
    AES128_ctx ctx;

public:
    explicit AES128Decrypt(const unsigned char key[16]);
    ~AES128Decrypt();
    void Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const;
};

/** AES-256 encryption */
class AES256Encrypt {
private:
    AES256_ctx ctx;

public:
    explicit AES256Encrypt(const unsigned char key[32]);
    ~AES256Encrypt();
    void Encrypt(unsigned char ciphertext[16], const unsigned char plaintext[16]) const;
};

/** AES-256 decryption */
class AES256Decrypt {
private:
    AES256_ctx ctx;

public:
    explicit AES256Decrypt(const unsigned char key[32]);
    ~AES256Decrypt();
    void Decrypt(unsigned char plaintext[16], const unsigned char ciphertext[16]) const;
};

/** AES-256 CBC mode encryption */
class AES256CBCEncrypt {
public:
    AES256CBCEncrypt(const unsigned char key[AES256_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn);
    ~AES256CBCEncrypt();
    int Encrypt(const unsigned char* data, int size, unsigned char* out) const;

private:
    const AES256Encrypt enc;
    const bool pad;
    unsigned char iv[AES_BLOCKSIZE];
};

/** AES-256 CBC mode decryption */
class AES256CBCDecrypt {
public:
    AES256CBCDecrypt(const unsigned char key[AES256_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn);
    ~AES256CBCDecrypt();
    int Decrypt(const unsigned char* data, int size, unsigned char* out) const;

private:
    const AES256Decrypt dec;
    const bool pad;
    unsigned char iv[AES_BLOCKSIZE];
};

/** AES-128 CBC mode encryption */
class AES128CBCEncrypt {
public:
    AES128CBCEncrypt(const unsigned char key[AES128_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn);
    ~AES128CBCEncrypt();
    int Encrypt(const unsigned char* data, int size, unsigned char* out) const;

private:
    const AES128Encrypt enc;
    const bool pad;
    unsigned char iv[AES_BLOCKSIZE];
};

/** AES-128 CBC mode decryption */
class AES128CBCDecrypt {
public:
    AES128CBCDecrypt(const unsigned char key[AES128_KEYSIZE], const unsigned char ivIn[AES_BLOCKSIZE], bool padIn);
    ~AES128CBCDecrypt();
    int Decrypt(const unsigned char* data, int size, unsigned char* out) const;

private:
    const AES128Decrypt dec;
    const bool pad;
    unsigned char iv[AES_BLOCKSIZE];
};

#endif // BITCOIN_CRYPTO_AES_H
