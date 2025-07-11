// Copyright (c) 2025 Notecoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key.h>
#include <crypto/hmac_sha512.h>
#include <random.h>
#include <utilstrencodings.h>
#include <hash.h>
#include <support/cleanse.h>
#include <chainparams.h>
#include <streams.h>
#include <util/system.h>

#include <secp256k1.h>
#include <secp256k1_recovery.h>

#include <assert.h>
#include <string>
#include <vector>

#include <bip39.h> // BIP39-Unterstützung

static secp256k1_context* secp256k1_context_sign = nullptr;

bool ECC_InitSanityCheck() {
    CKey key;
    key.MakeNewKey(true);
    CPubKey pubkey = key.GetPubKey();
    return key.VerifyPubKey(pubkey);
}

void ECC_Start() {
    assert(secp256k1_context_sign == nullptr);
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    assert(ctx != nullptr);

    std::vector<unsigned char, secure_allocator<unsigned char>> vseed(32);
    GetRandBytes(vseed.data(), 32);
    bool ret = secp256k1_context_randomize(ctx, vseed.data());
    assert(ret);

    secp256k1_context_sign = ctx;
}

void ECC_Stop() {
    secp256k1_context *ctx = secp256k1_context_sign;
    secp256k1_context_sign = nullptr;
    if (ctx) {
        secp256k1_context_destroy(ctx);
    }
}

bool CKey::Check(const unsigned char *vch) {
    return secp256k1_ec_seckey_verify(secp256k1_context_sign, vch);
}

void CKey::MakeNewKey(bool fCompressedIn) {
    do {
        GetStrongRandBytes(keydata.data(), keydata.size());
    } while (!Check(keydata.data()));
    fValid = true;
    fCompressed = fCompressedIn;
}

CPubKey CKey::GetPubKey() const {
    assert(fValid);
    secp256k1_pubkey pubkey;
    size_t clen = CPubKey::PUBLIC_KEY_SIZE;
    CPubKey result;
    int ret = secp256k1_ec_pubkey_create(secp256k1_context_sign, &pubkey, begin());
    assert(ret);
    secp256k1_ec_pubkey_serialize(secp256k1_context_sign, (unsigned char*)result.begin(), &clen, &pubkey, fCompressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED);
    assert(result.size() == clen);
    assert(result.IsValid());
    return result;
}

// BIP39: Mnemonic zu Seed
std::vector<unsigned char> MnemonicToSeed(const std::string& mnemonic, const std::string& passphrase = "mnemonic") {
    std::vector<unsigned char> seed(64);
    bip39_mnemonic_to_seed(mnemonic.c_str(), passphrase.c_str(), seed.data(), nullptr);
    return seed;
}

// BIP39 + BIP32: Seed zu Masterkey
bool SeedToExtKey(const std::vector<unsigned char>& seed, CExtKey& out) {
    if (seed.size() < 64) return false;
    static const unsigned char hashkey[] = {'B','i','t','c','o','i','n',' ','s','e','e','d'};
    std::vector<unsigned char, secure_allocator<unsigned char>> vout(64);
    CHMAC_SHA512(hashkey, sizeof(hashkey)).Write(seed.data(), seed.size()).Finalize(vout.data());
    out.key.Set(vout.data(), vout.data() + 32, true);
    memcpy(out.chaincode.begin(), vout.data() + 32, 32);
    out.nDepth = 0;
    out.nChild = 0;
    memset(out.vchFingerprint, 0, sizeof(out.vchFingerprint));
    return true;
}

// BIP44 Pfad: m/44'/coin_type'/account'/change/address
bool DeriveBIP44(const CExtKey& master, CExtKey& out, uint32_t coin_type, uint32_t account, uint32_t change, uint32_t index) {
    CExtKey purposeKey, coinKey, accountKey, changeKey;
    if (!master.Derive(purposeKey, 44 | 0x80000000)) return false;
    if (!purposeKey.Derive(coinKey, coin_type | 0x80000000)) return false;
    if (!coinKey.Derive(accountKey, account | 0x80000000)) return false;
    if (!accountKey.Derive(changeKey, change)) return false;
    return changeKey.Derive(out, index);
}

// In CExtKey integrierte Methoden:

CExtKey CExtKey::FromMnemonic(const std::string& mnemonic, const std::string& passphrase) {
    std::vector<unsigned char> seed = MnemonicToSeed(mnemonic, passphrase);
    CExtKey master;
    if (!SeedToExtKey(seed, master)) {
        throw std::runtime_error("Invalid mnemonic or seed conversion failed");
    }
    return master;
}

bool CExtKey::DeriveBIP44(CExtKey& out, uint32_t account, uint32_t change, uint32_t index, uint32_t coin_type) const {
    CExtKey purposeKey, coinKey, accountKey, changeKey;
    if (!this->Derive(purposeKey, 44 | 0x80000000)) return false;
    if (!purposeKey.Derive(coinKey, coin_type | 0x80000000)) return false;
    if (!coinKey.Derive(accountKey, account | 0x80000000)) return false;
    if (!accountKey.Derive(changeKey, change)) return false;
    return changeKey.Derive(out, index);
}
