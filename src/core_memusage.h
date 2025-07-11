// Copyright (c) 2015-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#ifndef BITCOIN_CORE_MEMUSAGE_H
#define BITCOIN_CORE_MEMUSAGE_H

#include <primitives/transaction.h>
#include <primitives/block.h>
#include <memusage.h>

/**
 * Diese Datei definiert rekursive Speicherverbrauchsberechnungen
 * für Transaktions- und Blockobjekte zur genaueren Analyse
 * des dynamisch allokierten Speichers.
 */

// ---------------------------------
// Einzelobjekte
// ---------------------------------

static inline size_t RecursiveDynamicUsage(const CScript& script) {
    return memusage::DynamicUsage(script);
}

static inline size_t RecursiveDynamicUsage(const COutPoint& out) {
    return 0; // Enthält keine dynamischen Daten
}

// ---------------------------------
// Transaktionseingänge
// ---------------------------------

static inline size_t RecursiveDynamicUsage(const CTxIn& in) {
    size_t mem = 0;
    mem += RecursiveDynamicUsage(in.scriptSig);
    mem += RecursiveDynamicUsage(in.prevout);
    mem += memusage::DynamicUsage(in.scriptWitness.stack);

    for (const auto& element : in.scriptWitness.stack) {
        mem += memusage::DynamicUsage(element);
    }

    return mem;
}

// ---------------------------------
// Transaktionsausgänge
// ---------------------------------

static inline size_t RecursiveDynamicUsage(const CTxOut& out) {
    return RecursiveDynamicUsage(out.scriptPubKey);
}

// ---------------------------------
// Transaktionen
// ---------------------------------

static inline size_t RecursiveDynamicUsage(const CTransaction& tx) {
    size_t mem = 0;
    mem += memusage::DynamicUsage(tx.vin);
    mem += memusage::DynamicUsage(tx.vout);

    for (const auto& in : tx.vin) {
        mem += RecursiveDynamicUsage(in);
    }
    for (const auto& out : tx.vout) {
        mem += RecursiveDynamicUsage(out);
    }

    return mem;
}

static inline size_t RecursiveDynamicUsage(const CMutableTransaction& tx) {
    size_t mem = 0;
    mem += memusage::DynamicUsage(tx.vin);
    mem += memusage::DynamicUsage(tx.vout);

    for (const auto& in : tx.vin) {
        mem += RecursiveDynamicUsage(in);
    }
    for (const auto& out : tx.vout) {
        mem += RecursiveDynamicUsage(out);
    }

    return mem;
}

// ---------------------------------
// Blöcke
// ---------------------------------

static inline size_t RecursiveDynamicUsage(const CBlock& block) {
    size_t mem = memusage::DynamicUsage(block.vtx);

    for (const auto& tx : block.vtx) {
        mem += memusage::DynamicUsage(tx);
        mem += RecursiveDynamicUsage(*tx);
    }

    return mem;
}

// ---------------------------------
// Block-Locator (für Sync-Mechanismen)
// ---------------------------------

static inline size_t RecursiveDynamicUsage(const CBlockLocator& locator) {
    return memusage::DynamicUsage(locator.vHave);
}

// ---------------------------------
// Shared Pointers auf Objekte
// ---------------------------------

template<typename X>
static inline size_t RecursiveDynamicUsage(const std::shared_ptr<X>& p) {
    return p ? memusage::DynamicUsage(p) + RecursiveDynamicUsage(*p) : 0;
}

#endif // BITCOIN_CORE_MEMUSAGE_H
