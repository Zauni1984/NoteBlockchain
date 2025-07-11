// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#ifndef BITCOIN_COMPRESSOR_H
#define BITCOIN_COMPRESSOR_H

#include <primitives/transaction.h>
#include <script/script.h>
#include <serialize.h>

class CKeyID;
class CPubKey;
class CScriptID;

/**
 * Kompakte Serialisierung für CScript.
 *
 * Erkennung häufiger Script-Typen und deren effiziente Kodierung:
 * - Pay-to-PubKey-Hash (21 Byte)
 * - Pay-to-Script-Hash (21 Byte)
 * - Pay-to-PubKey mit 0x02, 0x03 oder 0x04 (33 Byte)
 *
 * Alle anderen Scripts werden normal mit Länge serialisiert.
 */
class CScriptCompressor {
private:
    static const unsigned int nSpecialScripts = 6;
    CScript& script;

protected:
    bool IsToKeyID(CKeyID& hash) const;
    bool IsToScriptID(CScriptID& hash) const;
    bool IsToPubKey(CPubKey& pubkey) const;

    bool Compress(std::vector<unsigned char>& out) const;
    unsigned int GetSpecialSize(unsigned int nSize) const;
    bool Decompress(unsigned int nSize, const std::vector<unsigned char>& out);

public:
    explicit CScriptCompressor(CScript& scriptIn) : script(scriptIn) {}

    template<typename Stream>
    void Serialize(Stream& s) const {
        std::vector<unsigned char> compr;
        if (Compress(compr)) {
            s << CFlatData(compr);
        } else {
            unsigned int nSize = script.size() + nSpecialScripts;
            s << VARINT(nSize);
            s << CFlatData(script);
        }
    }

    template<typename Stream>
    void Unserialize(Stream& s) {
        unsigned int nSize = 0;
        s >> VARINT(nSize);

        if (nSize < nSpecialScripts) {
            std::vector<unsigned char> vch(GetSpecialSize(nSize), 0x00);
            s >> REF(CFlatData(vch));
            Decompress(nSize, vch);
        } else {
            nSize -= nSpecialScripts;
            if (nSize > MAX_SCRIPT_SIZE) {
                // Bei zu großen Scripts abbrechen und Dummy einfügen
                script.clear();
                script << OP_RETURN;
                s.ignore(nSize);
            } else {
                script.resize(nSize);
                s >> REF(CFlatData(script));
            }
        }
    }
};

/**
 * Wrapper für CTxOut zur komprimierten Serialisierung.
 * Spart Platz beim Speichern und Übertragen von Transaktionsausgängen.
 */
class CTxOutCompressor {
private:
    CTxOut& txout;

public:
    static uint64_t CompressAmount(uint64_t nAmount);
    static uint64_t DecompressAmount(uint64_t nAmount);

    explicit CTxOutCompressor(CTxOut& txoutIn) : txout(txoutIn) {}

    ADD_SERIALIZE_METHODS;

    template<typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        if (!ser_action.ForRead()) {
            uint64_t nVal = CompressAmount(txout.nValue);
            READWRITE(VARINT(nVal));
        } else {
            uint64_t nVal = 0;
            READWRITE(VARINT(nVal));
            txout.nValue = DecompressAmount(nVal);
        }

        CScriptCompressor cscript(REF(txout.scriptPubKey));
        READWRITE(cscript);
    }
};

#endif // BITCOIN_COMPRESSOR_H
