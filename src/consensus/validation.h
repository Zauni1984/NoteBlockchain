// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_VALIDATION_H
#define BITCOIN_CONSENSUS_VALIDATION_H

#include <string>
#include <primitives/transaction.h>
#include <primitives/block.h>
#include <consensus/consensus.h>

/** "reject" message codes */
static const unsigned char REJECT_MALFORMED         = 0x01;
static const unsigned char REJECT_INVALID           = 0x10;
static const unsigned char REJECT_OBSOLETE          = 0x11;
static const unsigned char REJECT_DUPLICATE         = 0x12;
static const unsigned char REJECT_NONSTANDARD       = 0x40;
static const unsigned char REJECT_INSUFFICIENTFEE   = 0x42;
static const unsigned char REJECT_CHECKPOINT        = 0x43;

/**
 * Validierungsstatus einer Transaktion oder eines Blocks.
 * Wird von Konsensfunktionen genutzt, um Fehler und Regeln zu unterscheiden.
 */
class CValidationState {
private:
    enum mode_state {
        MODE_VALID,    //!< Alles in Ordnung
        MODE_INVALID,  //!< Regelverstoß (DoS möglich)
        MODE_ERROR     //!< Laufzeitfehler
    } mode;

    int nDoS;
    std::string strRejectReason;
    unsigned int chRejectCode;
    bool corruptionPossible;
    std::string strDebugMessage;

public:
    CValidationState()
        : mode(MODE_VALID), nDoS(0), chRejectCode(0), corruptionPossible(false) {}

    bool DoS(int level, bool ret = false,
             unsigned int code = 0, const std::string& reason = "",
             bool corruption = false, const std::string& debug = "") {
        chRejectCode = code;
        strRejectReason = reason;
        corruptionPossible = corruption;
        strDebugMessage = debug;
        if (mode == MODE_ERROR) return ret;
        nDoS += level;
        mode = MODE_INVALID;
        return ret;
    }

    bool Invalid(bool ret = false,
                 unsigned int code = 0, const std::string& reason = "",
                 const std::string& debug = "") {
        return DoS(0, ret, code, reason, false, debug);
    }

    bool Error(const std::string& reason) {
        if (mode == MODE_VALID) strRejectReason = reason;
        mode = MODE_ERROR;
        return false;
    }

    bool IsValid() const   { return mode == MODE_VALID; }
    bool IsInvalid() const { return mode == MODE_INVALID; }
    bool IsError() const   { return mode == MODE_ERROR; }

    bool IsInvalid(int& dosOut) const {
        if (IsInvalid()) {
            dosOut = nDoS;
            return true;
        }
        return false;
    }

    bool CorruptionPossible() const { return corruptionPossible; }
    void SetCorruptionPossible()    { corruptionPossible = true; }

    unsigned int GetRejectCode()   const { return chRejectCode; }
    std::string  GetRejectReason() const { return strRejectReason; }
    std::string  GetDebugMessage() const { return strDebugMessage; }
};

/**
 * Berechnet das Gewicht einer Transaktion nach BIP141:
 * weight = (stripped_size * 3) + total_size
 */
static inline int64_t GetTransactionWeight(const CTransaction& tx) {
    return ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * (WITNESS_SCALE_FACTOR - 1)
         + ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
}

/**
 * Berechnet das Gewicht eines Blocks nach BIP141.
 */
static inline int64_t GetBlockWeight(const CBlock& block) {
    return ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * (WITNESS_SCALE_FACTOR - 1)
         + ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION);
}

#endif // BITCOIN_CONSENSUS_VALIDATION_H
