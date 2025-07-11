// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#include <compressor.h>

#include <hash.h>
#include <pubkey.h>
#include <script/standard.h>

// Prüft, ob das Script eine P2PKH-Adresse ist (Pay-to-PubKey-Hash)
bool CScriptCompressor::IsToKeyID(CKeyID& hash) const {
    if (script.size() == 25 && script[0] == OP_DUP && script[1] == OP_HASH160 &&
        script[2] == 20 && script[23] == OP_EQUALVERIFY && script[24] == OP_CHECKSIG) {
        memcpy(&hash, &script[3], 20);
        return true;
    }
    return false;
}

// Prüft, ob das Script eine P2SH-Adresse ist (Pay-to-Script-Hash)
bool CScriptCompressor::IsToScriptID(CScriptID& hash) const {
    if (script.size() == 23 && script[0] == OP_HASH160 && script[1] == 20 && script[22] == OP_EQUAL) {
        memcpy(&hash, &script[2], 20);
        return true;
    }
    return false;
}

// Prüft, ob das Script ein direktes PubKey-Script ist
bool CScriptCompressor::IsToPubKey(CPubKey& pubkey) const {
    if (script.size() == 35 && script[0] == 33 && script[34] == OP_CHECKSIG &&
        (script[1] == 0x02 || script[1] == 0x03)) {
        pubkey.Set(&script[1], &script[34]);
        return true;
    }
    if (script.size() == 67 && script[0] == 65 && script[66] == OP_CHECKSIG && script[1] == 0x04) {
        pubkey.Set(&script[1], &script[66]);
        return pubkey.IsFullyValid();
    }
    return false;
}

// Komprimiert das Script zu einem Bytecode-Format
bool CScriptCompressor::Compress(std::vector<unsigned char>& out) const {
    CKeyID keyID;
    if (IsToKeyID(keyID)) {
        out.resize(21);
        out[0] = 0x00;
        memcpy(&out[1], &keyID, 20);
        return true;
    }

    CScriptID scriptID;
    if (IsToScriptID(scriptID)) {
        out.resize(21);
        out[0] = 0x01;
        memcpy(&out[1], &scriptID, 20);
        return true;
    }

    CPubKey pubkey;
    if (IsToPubKey(pubkey)) {
        out.resize(33);
        memcpy(&out[1], &pubkey[1], 32);
        if (pubkey[0] == 0x02 || pubkey[0] == 0x03) {
            out[0] = pubkey[0];
            return true;
        } else if (pubkey[0] == 0x04) {
            out[0] = 0x04 | (pubkey[64] & 0x01);
            return true;
        }
    }

    return false;
}

// Gibt die Spezialgröße zurück, die beim Dekomprimieren verwendet wird
unsigned int CScriptCompressor::GetSpecialSize(unsigned int nSize) const {
    if (nSize == 0 || nSize == 1) return 20;
    if (nSize >= 2 && nSize <= 5) return 32;
    return 0;
}

// Dekomprimiert ein komprimiertes Script
bool CScriptCompressor::Decompress(unsigned int nSize, const std::vector<unsigned char>& in) {
    switch (nSize) {
        case 0x00: {
            script.resize(25);
            script[0] = OP_DUP;
            script[1] = OP_HASH160;
            script[2] = 20;
            memcpy(&script[3], in.data(), 20);
            script[23] = OP_EQUALVERIFY;
            script[24] = OP_CHECKSIG;
            return true;
        }
        case 0x01: {
            script.resize(23);
            script[0] = OP_HASH160;
            script[1] = 20;
            memcpy(&script[2], in.data(), 20);
            script[22] = OP_EQUAL;
            return true;
        }
        case 0x02:
        case 0x03: {
            script.resize(35);
            script[0] = 33;
            script[1] = nSize;
            memcpy(&script[2], in.data(), 32);
            script[34] = OP_CHECKSIG;
            return true;
        }
        case 0x04:
        case 0x05: {
            unsigned char vch[33] = {};
            vch[0] = nSize - 2;
            memcpy(&vch[1], in.data(), 32);
            CPubKey pubkey(&vch[0], &vch[33]);
            if (!pubkey.Decompress()) return false;
            assert(pubkey.size() == 65);
            script.resize(67);
            script[0] = 65;
            memcpy(&script[1], pubkey.begin(), 65);
            script[66] = OP_CHECKSIG;
            return true;
        }
    }
    return false;
}

// Kompression von Beträgen (Amount):
// 0 bleibt 0
// Andernfalls wird in wissenschaftlicher Schreibweise komprimiert.
uint64_t CTxOutCompressor::CompressAmount(uint64_t n) {
    if (n == 0) return 0;
    int e = 0;
    while ((n % 10) == 0 && e < 9) {
        n /= 10;
        ++e;
    }
    if (e < 9) {
        int d = n % 10;
        assert(d >= 1 && d <= 9);
        n /= 10;
        return 1 + (n * 9 + d - 1) * 10 + e;
    } else {
        return 1 + (n - 1) * 10 + 9;
    }
}

// Dekompression eines komprimierten Betrags
uint64_t CTxOutCompressor::DecompressAmount(uint64_t x) {
    if (x == 0) return 0;
    --x;
    int e = x % 10;
    x /= 10;
    uint64_t n = 0;

    if (e < 9) {
        int d = (x % 9) + 1;
        x /= 9;
        n = x * 10 + d;
    } else {
        n = x + 1;
    }

    while (e > 0) {
        n *= 10;
        --e;
    }

    return n;
}
