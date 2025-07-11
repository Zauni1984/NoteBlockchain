// Copyright (c) 2014-2024 The NoteCoin Developers
// Distributed under the MIT software license.

#ifndef NOTECOIN_CHAINPARAMSBASE_H
#define NOTECOIN_CHAINPARAMSBASE_H

#include <memory>
#include <string>
#include <vector>

/**
 * CBaseChainParams defines the base parameters shared between notecoin-cli and notecoind,
 * such as RPC port and data directory for each network.
 */
class CBaseChainParams
{
public:
    /** Chain name identifiers */
    static const std::string MAIN;
    static const std::string TESTNET;
    static const std::string REGTEST;

    /** Accessors */
    const std::string& DataDir() const { return strDataDir; }
    int RPCPort() const { return nRPCPort; }

protected:
    CBaseChainParams() = default;

    int nRPCPort = 0;
    std::string strDataDir;
};

/**
 * Creates and returns a std::unique_ptr<CBaseChainParams> for the given chain.
 * @param chain The name of the chain (main, test, regtest).
 * @throws std::runtime_error if the chain is unsupported.
 */
std::unique_ptr<CBaseChainParams> CreateBaseChainParams(const std::string& chain);

/**
 * Adds help messages related to chain selection to the usage string.
 */
void AppendParamsHelpMessages(std::string& strUsage, bool debugHelp = true);

/**
 * Returns the currently selected chain parameters.
 * This is constant after startup (except in unit tests).
 */
const CBaseChainParams& BaseParams();

/**
 * Selects and sets the base parameters for the given chain.
 */
void SelectBaseParams(const std::string& chain);

/**
 * Determines the chain name from command line options -regtest or -testnet.
 * @return One of CBaseChainParams::MAIN, TESTNET, or REGTEST.
 */
std::string ChainNameFromCommandLine();

#endif // NOTECOIN_CHAINPARAMSBASE_H
