// Copyright (c) 2009-2025 The Notecoin Core developers
// Distributed under the MIT software license.

#include <config/bitcoin-config.h>
#include <clientversion.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <util/moneystr.h>
#include <util/translation.h>
#include <univalue.h>
#include <fs.h>

#include <core_io.h>
#include <key_io.h>
#include <keystore.h>
#include <primitives/transaction.h>
#include <script/sign.h>
#include <streams.h>
#include <txmempool.h>
#include <wallet/scriptpubkeyman.h>

#include <iostream>
#include <memory>

static void SetupBitcoinTx()
{
    SetupEnvironment();
    gArgs.ForceSetArg("-datadir", ".");
    SelectParams(CBaseChainParams::MAIN);
}

static void PrintHelp()
{
    std::cout << "notecoin-tx - command line transaction builder\n\n"
              << "Usage:\n"
              << "  notecoin-tx [hex] [commands...] > hex\n\n"
              << "Commands:\n"
              << "  delin=N       remove input N\n"
              << "  in=TXID:VOUT  add input\n"
              << "  outaddr=VALUE:ADDRESS  add output to ADDRESS\n"
              << "  outdata=VALUE:DATA     add OP_RETURN output\n"
              << "  sign=PRIVATEKEY        sign all inputs with key\n"
              << "  sendraw                send transaction to the network (not implemented)\n";
}

static CMutableTransaction DecodeHexTransactionOrDie(const std::string& hex)
{
    CMutableTransaction tx;
    if (!DecodeHexTx(tx, hex)) {
        throw std::runtime_error("Invalid transaction hex string");
    }
    return tx;
}

static std::string EncodeTxToHex(const CMutableTransaction& tx)
{
    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;
    return HexStr(ssTx.begin(), ssTx.end());
}

int main(int argc, char* argv[])
{
    SetupBitcoinTx();

    if (argc < 2) {
        PrintHelp();
        return EXIT_FAILURE;
    }

    try {
        CMutableTransaction tx;

        // Check if first argument is a hex string or a command
        int argStart = 1;
        if (!IsHex(argv[1])) {
            tx = CMutableTransaction();
        } else {
            tx = DecodeHexTransactionOrDie(argv[1]);
            argStart = 2;
        }

        for (int i = argStart; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg.find("delin=") == 0) {
                int index = std::stoi(arg.substr(7));
                if (index < 0 || static_cast<size_t>(index) >= tx.vin.size())
                    throw std::runtime_error("Input index out of range");
                tx.vin.erase(tx.vin.begin() + index);

            } else if (arg.find("in=") == 0) {
                std::string s = arg.substr(3);
                size_t sep = s.find(':');
                if (sep == std::string::npos)
                    throw std::runtime_error("Invalid input format");
                uint256 txid = uint256S(s.substr(0, sep));
                uint32_t vout = std::stoi(s.substr(sep + 1));
                tx.vin.push_back(CTxIn(COutPoint(txid, vout)));

            } else if (arg.find("outaddr=") == 0) {
                std::string s = arg.substr(8);
                size_t sep = s.find(':');
                if (sep == std::string::npos)
                    throw std::runtime_error("Invalid outaddr format");
                CAmount amount = AmountFromValue(UniValue(s.substr(0, sep)));
                CTxDestination dest = DecodeDestination(s.substr(sep + 1));
                if (!IsValidDestination(dest))
                    throw std::runtime_error("Invalid address");
                tx.vout.emplace_back(amount, GetScriptForDestination(dest));

            } else if (arg.find("outdata=") == 0) {
                std::string s = arg.substr(8);
                size_t sep = s.find(':');
                if (sep == std::string::npos)
                    throw std::runtime_error("Invalid outdata format");
                CAmount amount = AmountFromValue(UniValue(s.substr(0, sep)));
                std::vector<unsigned char> data = ParseHex(s.substr(sep + 1));
                CScript scriptPubKey = CScript() << OP_RETURN << data;
                tx.vout.emplace_back(amount, scriptPubKey);

            } else if (arg.find("sign=") == 0) {
                std::string privkeyStr = arg.substr(5);
                CKey key;
                CBitcoinSecret secret;
                if (!secret.SetString(privkeyStr))
                    throw std::runtime_error("Invalid private key");
                key = secret.GetKey();

                CBasicKeyStore keystore;
                keystore.AddKey(key);
                int nHashType = SIGHASH_ALL;

                for (size_t i = 0; i < tx.vin.size(); ++i) {
                    const CTxIn& txin = tx.vin[i];
                    // Normally, you'd fetch previous output scriptPubKey from blockchain/mempool
                    // Here we mock it for simplicity (replace with real lookup in real usage)
                    CScript prevPubKey; // = get from UTXO
                    SignatureData sigdata;
                    ProduceSignature(TransactionSignatureCreator(&keystore, &tx, i, 100000, nHashType), prevPubKey, sigdata);
                    UpdateInput(tx.vin[i], sigdata);
                }

            } else if (arg == "sendraw") {
                throw std::runtime_error("sendraw not implemented in this build");
            } else {
                throw std::runtime_error("Unknown argument: " + arg);
            }
        }

        std::cout << EncodeTxToHex(tx) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
