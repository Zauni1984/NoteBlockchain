// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#include <core_io.h>

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <serialize.h>
#include <streams.h>
#include <univalue.h>
#include <util.h>
#include <utilstrencodings.h>
#include <version.h>

#include <boost/algorithm/string.hpp>
#include <map>

// --------------------------------------------
// Skript Parsen: String → CScript
// --------------------------------------------

CScript ParseScript(const std::string& s) {
    static std::map<std::string, opcodetype> mapOpNames;
    CScript result;

    if (mapOpNames.empty()) {
        for (unsigned int op = 0; op <= MAX_OPCODE; ++op) {
            if (op < OP_NOP && op != OP_RESERVED) continue;
            const char* name = GetOpName((opcodetype)op);
            if (strcmp(name, "OP_UNKNOWN") == 0) continue;

            std::string strName(name);
            mapOpNames[strName] = (opcodetype)op;
            boost::replace_first(strName, "OP_", "");
            mapOpNames[strName] = (opcodetype)op;
        }
    }

    std::vector<std::string> words;
    boost::split(words, s, boost::is_any_of(" \t\n"), boost::token_compress_on);

    for (const std::string& word : words) {
        if (word.empty()) continue;

        if (boost::all(word, boost::is_digit()) || 
            (boost::starts_with(word, "-") && boost::all(std::string(word.begin() + 1, word.end()), boost::is_digit()))) {
            result << atoi64(word);
        }
        else if (boost::starts_with(word, "0x") && word.size() > 2 && IsHex(word.substr(2))) {
            std::vector<unsigned char> raw = ParseHex(word.substr(2));
            result.insert(result.end(), raw.begin(), raw.end());
        }
        else if (word.size() >= 2 && word.front() == '\'' && word.back() == '\'') {
            std::vector<unsigned char> value(word.begin() + 1, word.end() - 1);
            result << value;
        }
        else if (mapOpNames.count(word)) {
            result << mapOpNames[word];
        }
        else {
            throw std::runtime_error("script parse error");
        }
    }

    return result;
}

// --------------------------------------------
// Gültigkeit der Skripte in Transaktionen
// --------------------------------------------

bool CheckTxScriptsSanity(const CMutableTransaction& tx) {
    if (!CTransaction(tx).IsCoinBase()) {
        for (const auto& in : tx.vin) {
            if (!in.scriptSig.HasValidOps() || in.scriptSig.size() > MAX_SCRIPT_SIZE)
                return false;
        }
    }

    for (const auto& out : tx.vout) {
        if (!out.scriptPubKey.HasValidOps() || out.scriptPubKey.size() > MAX_SCRIPT_SIZE)
            return false;
    }

    return true;
}

// --------------------------------------------
// Hex-Transaktion decodieren
// --------------------------------------------

bool DecodeHexTx(CMutableTransaction& tx, const std::string& hex_tx, bool try_no_witness, bool try_witness) {
    if (!IsHex(hex_tx)) return false;
    std::vector<unsigned char> txData(ParseHex(hex_tx));

    if (try_no_witness) {
        CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS);
        try {
            ssData >> tx;
            if (ssData.eof() && (!try_witness || CheckTxScriptsSanity(tx))) return true;
        } catch (...) {}
    }

    if (try_witness) {
        CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
        try {
            ssData >> tx;
            if (ssData.empty()) return true;
        } catch (...) {}
    }

    return false;
}

// --------------------------------------------
// Hex-Block decodieren
// --------------------------------------------

bool DecodeHexBlk(CBlock& block, const std::string& strHexBlk) {
    if (!IsHex(strHexBlk)) return false;
    std::vector<unsigned char> blockData(ParseHex(strHexBlk));

    CDataStream ssBlock(blockData, SER_NETWORK, PROTOCOL_VERSION);
    try {
        ssBlock >> block;
    } catch (...) {
        return false;
    }

    return true;
}

// --------------------------------------------
// Hash-Werte und Hex-Werte aus UniValue extrahieren
// --------------------------------------------

uint256 ParseHashUV(const UniValue& v, const std::string& strName) {
    std::string strHex = v.isStr() ? v.getValStr() : "";
    return ParseHashStr(strHex, strName);
}

uint256 ParseHashStr(const std::string& strHex, const std::string& strName) {
    if (!IsHex(strHex))
        throw std::runtime_error(strName + " must be hexadecimal string (not '" + strHex + "')");

    uint256 result;
    result.SetHex(strHex);
    return result;
}

std::vector<unsigned char> ParseHexUV(const UniValue& v, const std::string& strName) {
    std::string strHex = v.isStr() ? v.getValStr() : "";
    if (!IsHex(strHex))
        throw std::runtime_error(strName + " must be hexadecimal string (not '" + strHex + "')");

    return ParseHex(strHex);
}
