// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2024 The NoteCoin Developers
// Distributed under the MIT software license.

#include <chainparamsbase.h>

#include <tinyformat.h>
#include <util.h>
#include <util/translation.h>

#include <memory>
#include <cassert>

const std::string CBaseChainParams::MAIN    = "main";
const std::string CBaseChainParams::TESTNET = "test";
const std::string CBaseChainParams::REGTEST = "regtest";

void AppendParamsHelpMessages(std::string& strUsage, bool debugHelp)
{
    strUsage += HelpMessageGroup(_("Chain selection options:"));
    strUsage += HelpMessageOpt("-testnet", _("Use the test chain"));
    if (debugHelp) {
        strUsage += HelpMessageOpt("-regtest", _("Enter regression test mode, allowing instant blocks. "
                                                 "Used for testing and app development."));
    }
}

// -----------------------------
// BaseChainParams Implementierung
// -----------------------------

class CBaseMainParams : public CBaseChainParams {
public:
    CBaseMainParams() {
        nRPCPort = 9332;
        strDataDir = "main";
    }
};

class CBaseTestNetParams : public CBaseChainParams {
public:
    CBaseTestNetParams() {
        nRPCPort = 19332;
        strDataDir = "testnet4";
    }
};

class CBaseRegTestParams : public CBaseChainParams {
public:
    CBaseRegTestParams() {
        nRPCPort = 19443;
        strDataDir = "regtest";
    }
};

static std::unique_ptr<CBaseChainParams> globalChainBaseParams;

const CBaseChainParams& BaseParams()
{
    assert(globalChainBaseParams);
    return *globalChainBaseParams;
}

std::unique_ptr<CBaseChainParams> CreateBaseChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN) {
        return std::make_unique<CBaseMainParams>();
    } else if (chain == CBaseChainParams::TESTNET) {
        return std::make_unique<CBaseTestNetParams>();
    } else if (chain == CBaseChainParams::REGTEST) {
        return std::make_unique<CBaseRegTestParams>();
    }

    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectBaseParams(const std::string& chain)
{
    globalChainBaseParams = CreateBaseChainParams(chain);
}

std::string ChainNameFromCommandLine()
{
    const bool fRegTest = gArgs.GetBoolArg("-regtest", false);
    const bool fTestNet = gArgs.GetBoolArg("-testnet", false);

    if (fRegTest && fTestNet) {
        throw std::runtime_error("Invalid combination of -regtest and -testnet.");
    }

    if (fRegTest)  return CBaseChainParams::REGTEST;
    if (fTestNet)  return CBaseChainParams::TESTNET;

    return CBaseChainParams::MAIN;
}
