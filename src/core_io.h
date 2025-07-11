// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#ifndef BITCOIN_CORE_IO_H
#define BITCOIN_CORE_IO_H

#include <amount.h>

#include <string>
#include <vector>

class CBlock;
class CScript;
class CTransaction;
struct CMutableTransaction;
class uint256;
class UniValue;

/**
 * Funktionen zum Parsen und Serialisieren von Transaktionen, Blöcken und Scripts
 * – genutzt für RPC, Debugging und das CLI.
 */

// ==========================
// core_read.cpp Schnittstelle
// ==========================

/** Wandelt einen String in ein CScript um (z. B. Assembly-Format). */
CScript ParseScript(const std::string& s);

/** Gibt ein Script als ASM-String zurück, optional mit Signatur-Hash-Erkennung. */
std::string ScriptToAsmStr(const CScript& script, const bool fAttemptSighashDecode = false);

/** Dekodiert eine Hex-codierte Transaktion (mit/ohne Witness). */
bool DecodeHexTx(CMutableTransaction& tx, const std::string& hex_tx, bool try_no_witness = false, bool try_witness = true);

/** Dekodiert einen Hex-codierten Block. */
bool DecodeHexBlk(CBlock& block, const std::string& strHexBlk);

/** Liest einen Hash-Wert aus einem UniValue-Eintrag. */
uint256 ParseHashUV(const UniValue& v, const std::string& strName);

/** Liest einen Hash-Wert aus einem String. */
uint256 ParseHashStr(const std::string& str, const std::string& strName);

/** Liest ein Hex-Array aus einem UniValue-Eintrag. */
std::vector<unsigned char> ParseHexUV(const UniValue& v, const std::string& strName);

// ===========================
// core_write.cpp Schnittstelle
// ===========================

/** Wandelt einen Geldbetrag in einen JSON-Wert um. */
UniValue ValueFromAmount(const CAmount& amount);

/** Gibt ein Script als formatierten String zurück. */
std::string FormatScript(const CScript& script);

/** Kodiert eine Transaktion als Hex-String. */
std::string EncodeHexTx(const CTransaction& tx, const int serializeFlags = 0);

/** Fügt ein ScriptPubKey-Objekt zu einem JSON-Wert hinzu. */
void ScriptPubKeyToUniv(const CScript& scriptPubKey, UniValue& out, bool fIncludeHex);

/** Konvertiert eine Transaktion in ein JSON-Objekt. */
void TxToUniv(const CTransaction& tx, const uint256& hashBlock, UniValue& entry, bool include_hex = true, int serialize_flags = 0);

#endif // BITCOIN_CORE_IO_H
