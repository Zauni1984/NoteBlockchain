// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Copyright (c) 2025 Notecoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KEY_H
#define BITCOIN_KEY_H

#include <pubkey.h>
#include <serialize.h>
#include <support/allocators/secure.h>
#include <uint256.h>
#include <chainparams.h>

#include <stdexcept>
#include <vector>
#include <string>

/** CPrivKey: Serialized private key */
typedef std::vector<unsigned char, secure_allocator<unsigned char>> CPrivKey;

/** A Bitcoin private key (secp256k1) */
class CKey {
public:
    static const unsigned int PRIVATE_KEY_SIZE            = 279;
    static const unsigned int COMPRESSED_PRIVATE_KEY_SIZE = 214;

    CKey();

    friend bool operator==(const CKey& a, const CKey& b);

    template <typename T>
    void Set(const T pbegin, const T pend, bool fCompressedIn);

    unsigned int size() const;
    const unsigned char* begin() const;
    const unsigned char* end() const;

    bool IsValid() const;
    bool IsCompressed() const;

    void MakeNewKey(bool fCompressed);

    CPrivKey GetPrivKey() const;
    CPubKey GetPubKey() const;

    bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, uint32_t test_case = 0) const;
    bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;

    bool Derive(CKey& keyChild, ChainCode& ccChild, unsigned int nChild, const ChainCode& cc) const;

    bool VerifyPubKey(const CPubKey& vchPubKey) const;
    bool Load(const CPrivKey& privkey, const CPubKey& vchPubKey, bool fSkipCheck);

private:
    bool fValid;
    bool fCompressed;
    std::vector<unsigned char, secure_allocator<unsigned char>> keydata;

    static bool Check(const unsigned char* vch);
};

/** Extended Key (BIP32) */
struct CExtKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    CKey key;

    friend bool operator==(const CExtKey& a, const CExtKey& b);

    void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
    bool Derive(CExtKey& out, unsigned int nChild) const;
    CExtPubKey Neuter() const;
    void SetMaster(const unsigned char* seed, unsigned int nSeedLen);

    /** Neues Feature: Ableitung aus einer BIP39-Mnemonic */
    static CExtKey FromMnemonic(const std::string& mnemonic, const std::string& passphrase = "mnemonic");

    /** Neues Feature: Ableitung nach BIP44-Pfad m/44'/coin_type'/account'/change/address */
    bool DeriveBIP44(CExtKey& out, uint32_t account, uint32_t change, uint32_t index, uint32_t coin_type = Params().BIP44CoinType()) const;

    template <typename Stream>
    void Serialize(Stream& s) const {
        unsigned int len = BIP32_EXTKEY_SIZE;
        ::WriteCompactSize(s, len);
        unsigned char code[BIP32_EXTKEY_SIZE];
        Encode(code);
        s.write((const char*)&code[0], len);
    }

    template <typename Stream>
    void Unserialize(Stream& s) {
        unsigned int len = ::ReadCompactSize(s);
        unsigned char code[BIP32_EXTKEY_SIZE];
        if (len != BIP32_EXTKEY_SIZE)
            throw std::runtime_error("Invalid extended key size");
        s.read((char*)&code[0], len);
        Decode(code);
    }
};

// ECC context management
void ECC_Start(void);
void ECC_Stop(void);
bool ECC_InitSanityCheck(void);

#endif // BITCOIN_KEY_H
