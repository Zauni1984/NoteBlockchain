// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#include <core_io.h>

#include <base58.h>
#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <script/script.h>
#include <script/standard.h>
#include <serialize.h>
#include <streams.h>
#include <univalue.h>
#include <util.h>
#include <utilmoneystr.h>
#include <utilstrencodings.h>

// --------------------------------------------
// Betrag als Dezimalzahl ausgeben
// --------------------------------------------

UniValue ValueFromAmount(const CAmount& amount) {
    bool sign = amount < 0;
    int64_t n_abs = sign ? -amount : amount;
    int64_t quotient = n_abs / COIN;
    int64_t remainder = n_abs % COIN;
    return UniValue(UniValue::VNUM, strprintf("%s%d.%08d", sign ? "-" : "", quotient, remainder));
}

// --------------------------------------------
// CScript als menschenlesbaren String ausgeben
// --------------------------------------------

std::string FormatScript(const CScript& script) {
    std::string result;
    auto it = script.begin();
    opcodetype op;

    while (it != script.end()) {
        auto it2 = it;
        std::vector<unsigned char> vch;

        if (script.GetOp2(it, op, &vch)) {
            if (op == OP_0) {
                result += "0 ";
            } else if ((op >= OP_1 && op <= OP_16) || op == OP_1NEGATE) {
                result += strprintf("%i ", op - OP_1NEGATE - 1);
            } else if (op >= OP_NOP && op <= OP_NOP10) {
                std::string str(GetOpName(op));
                result += (str.rfind("OP_", 0) == 0) ? str.substr(3) + " " : str + " ";
            } else {
                result += vch.empty()
                    ? strprintf("0x%x ", HexStr(it2, it))
                    : strprintf("0x%x 0x%x ", HexStr(it2, it - vch.size()), HexStr(it - vch.size(), it));
            }
        } else {
            result += strprintf("0x%x ", HexStr(it2, script.end()));
            break;
        }
    }

    if (!result.empty() && result.back() == ' ') {
        result.pop_back(); // Letztes Leerzeichen entfernen
    }

    return result;
}

// --------------------------------------------
// SigHash-Typen zur Anzeige
// --------------------------------------------

const std::map<unsigned char, std::string> mapSigHashTypes = {
    {SIGHASH_ALL,                     "ALL"},
    {SIGHASH_ALL | SIGHASH_ANYONECANPAY, "ALL|ANYONECANPAY"},
    {SIGHASH_NONE,                    "NONE"},
    {SIGHASH_NONE | SIGHASH_ANYONECANPAY, "NONE|ANYONECANPAY"},
    {SIGHASH_SINGLE,                 "SINGLE"},
    {SIGHASH_SINGLE | SIGHASH_ANYONECANPAY, "SINGLE|ANYONECANPAY"},
};

// --------------------------------------------
// Script → Assembly-ähnlicher String
// --------------------------------------------

std::string ScriptToAsmStr(const CScript& script, const bool fAttemptSighashDecode) {
    std::string str;
    opcodetype opcode;
    std::vector<unsigned char> vch;
    auto pc = script.begin();

    while (pc < script.end()) {
        if (!str.empty()) str += " ";

        if (!script.GetOp(pc, opcode, vch)) {
            str += "[error]";
            return str;
        }

        if (0 <= opcode && opcode <= OP_PUSHDATA4) {
            if (vch.size() <= 4) {
                str += strprintf("%d", CScriptNum(vch, false).getint());
            } else if (fAttemptSighashDecode && !script.IsUnspendable()) {
                std::string strSigHashDecode;
                if (CheckSignatureEncoding(vch, SCRIPT_VERIFY_STRICTENC, nullptr)) {
                    const unsigned char chSigHashType = vch.back();
                    auto it = mapSigHashTypes.find(chSigHashType);
                    if (it != mapSigHashTypes.end()) {
                        strSigHashDecode = "[" + it->second + "]";
                        vch.pop_back();
                    }
                }
                str += HexStr(vch) + strSigHashDecode;
            } else {
                str += HexStr(vch);
            }
        } else {
            str += GetOpName(opcode);
        }
    }

    return str;
}

// --------------------------------------------
// CTransaction → Hex
// --------------------------------------------

std::string EncodeHexTx(const CTransaction& tx, const int serializeFlags) {
    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION | serializeFlags);
    ssTx << tx;
    return HexStr(ssTx.begin(), ssTx.end());
}

// --------------------------------------------
// ScriptPubKey in JSON-Objekt umwandeln
// --------------------------------------------

void ScriptPubKeyToUniv(const CScript& scriptPubKey, UniValue& out, bool fIncludeHex) {
    txnouttype type;
    std::vector<CTxDestination> addresses;
    int nRequired;

    out.pushKV("asm", ScriptToAsmStr(scriptPubKey));
    if (fIncludeHex) {
        out.pushKV("hex", HexStr(scriptPubKey.begin(), scriptPubKey.end()));
    }

    if (!ExtractDestinations(scriptPubKey, type, addresses, nRequired)) {
        out.pushKV("type", GetTxnOutputType(type));
        return;
    }

    out.pushKV("reqSigs", nRequired);
    out.pushKV("type", GetTxnOutputType(type));

    UniValue a(UniValue::VARR);
    for (const auto& addr : addresses) {
        a.push_back(EncodeDestination(addr));
    }
    out.pushKV("addresses", a);
}

// --------------------------------------------
// CTransaction → JSON-Objekt
// --------------------------------------------

void TxToUniv(const CTransaction& tx, const uint256& hashBlock, UniValue& entry,
              bool include_hex, int serialize_flags) {
    entry.pushKV("txid", tx.GetHash().GetHex());
    entry.pushKV("hash", tx.GetWitnessHash().GetHex());
    entry.pushKV("version", tx.nVersion);
    entry.pushKV("size", static_cast<int>(::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION)));
    entry.pushKV("vsize", (GetTransactionWeight(tx) + WITNESS_SCALE_FACTOR - 1) / WITNESS_SCALE_FACTOR);
    entry.pushKV("locktime", static_cast<int64_t>(tx.nLockTime));

    // Inputs
    UniValue vin(UniValue::VARR);
    for (const auto& txin : tx.vin) {
        UniValue in(UniValue::VOBJ);

        if (tx.IsCoinBase()) {
            in.pushKV("coinbase", HexStr(txin.scriptSig));
        } else {
            in.pushKV("txid", txin.prevout.hash.GetHex());
            in.pushKV("vout", static_cast<int64_t>(txin.prevout.n));

            UniValue scriptSig(UniValue::VOBJ);
            scriptSig.pushKV("asm", ScriptToAsmStr(txin.scriptSig, true));
            scriptSig.pushKV("hex", HexStr(txin.scriptSig));
            in.pushKV("scriptSig", scriptSig);

            if (!txin.scriptWitness.IsNull()) {
                UniValue txinwitness(UniValue::VARR);
                for (const auto& item : txin.scriptWitness.stack) {
                    txinwitness.push_back(HexStr(item));
                }
                in.pushKV("txinwitness", txinwitness);
            }
        }

        in.pushKV("sequence", static_cast<int64_t>(txin.nSequence));
        vin.push_back(in);
    }
    entry.pushKV("vin", vin);

    // Outputs
    UniValue vout(UniValue::VARR);
    for (unsigned int i = 0; i < tx.vout.size(); ++i) {
        const CTxOut& txout = tx.vout[i];
        UniValue out(UniValue::VOBJ);

        out.pushKV("value", ValueFromAmount(txout.nValue));
        out.pushKV("n", static_cast<int64_t>(i));

        UniValue scriptPubKey(UniValue::VOBJ);
        ScriptPubKeyToUniv(txout.scriptPubKey, scriptPubKey, true);
        out.pushKV("scriptPubKey", scriptPubKey);

        vout.push_back(out);
    }
    entry.pushKV("vout", vout);

    if (!hashBlock.IsNull()) {
        entry.pushKV("blockhash", hashBlock.GetHex());
    }

    if (include_hex) {
        entry.pushKV("hex", EncodeHexTx(tx, serialize_flags));
    }
}
