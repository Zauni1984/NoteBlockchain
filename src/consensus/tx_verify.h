// Copyright (c) 2017-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_TX_VERIFY_H
#define BITCOIN_CONSENSUS_TX_VERIFY_H

#include <amount.h>
#include <stdint.h>
#include <vector>

class CBlockIndex;
class CCoinsViewCache;
class CTransaction;
class CValidationState;

/** Prüft transaktionsinterne Konsistenzregeln (ohne Blockchain-Kontext) */
bool CheckTransaction(const CTransaction& tx, CValidationState& state, bool fCheckDuplicateInputs = true);

namespace Consensus {
/**
 * Prüft die Gültigkeit der Eingaben einer Transaktion gegen den UTXO-Cache.
 * Signaturen werden nicht geprüft. Setzt txfee bei Erfolg.
 */
bool CheckTxInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& inputs, int nSpendHeight, CAmount& txfee);
} // namespace Consensus

/** Anzahl klassischer ECDSA-Signatur-Operationen (ohne P2SH/Witness) */
unsigned int GetLegacySigOpCount(const CTransaction& tx);

/** Signatur-Operationen bei P2SH-Eingängen zählen */
unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& inputs);

/** Gesamtkosten für Signatur-Operationen einer Transaktion berechnen */
int64_t GetTransactionSigOpCost(const CTransaction& tx, const CCoinsViewCache& inputs, int flags);

/** Prüft, ob eine Transaktion final ist (nLockTime + nSequence) */
bool IsFinalTx(const CTransaction& tx, int nBlockHeight, int64_t nBlockTime);

/** Ermittelt relative Sperren (BIP 68) einer Transaktion */
std::pair<int, int64_t> CalculateSequenceLocks(const CTransaction& tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block);

/** Bewertet, ob BIP-68-Sperren erfüllt sind */
bool EvaluateSequenceLocks(const CBlockIndex& block, std::pair<int, int64_t> lockPair);

/** Führt vollständige BIP-68-Sperrenprüfung anhand vorheriger Input-Höhen durch */
bool SequenceLocks(const CTransaction& tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block);

#endif // BITCOIN_CONSENSUS_TX_VERIFY_H
