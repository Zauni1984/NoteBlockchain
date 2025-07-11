// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util/system.h>
#include <util/strencodings.h>

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "NoteCoin Reboot 2025 – Back to the roots";
    const CScript genesisOutputScript = CScript() << ParseHex("04ffff001d0104") << OP_CHECKSIG;

    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 0 << CScriptNum(999) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";

        consensus.nSubsidyHalvingInterval = 302400; // 420 Tage bei 2 Min Blockzeit
        consensus.BIP34Height = 1;
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;

        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // KawPoW Limit
        consensus.nPowTargetSpacing = 120; // 2 Minuten
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;

        consensus.nLWMAHeight = 0; // Sofort aktiv

        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow = 2016;

        consensus.nMinimumChainWork = uint256S("00");
        consensus.defaultAssumeValid = uint256S("00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;

        nDefaultPort = 34567;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1710000000, 2083236893, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("PUT-YOUR-MAINNET-GENESIS-HASH-HERE"));
        assert(genesis.hashMerkleRoot == uint256S("PUT-YOUR-MAINNET-MERKLE-ROOT-HERE"));

        vSeeds.push_back("dnsseed.notecoin.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 53); // N
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 5);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "nt";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                {0, consensus.hashGenesisBlock}
            }
        };

        chainTxData = ChainTxData{
            1710000000, // timestamp
            0,
            0.0
        };
    }
};

class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";

        consensus.nSubsidyHalvingInterval = 302400;
        consensus.BIP34Height = 1;
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;

        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetSpacing = 120;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;

        consensus.nLWMAHeight = 0;

        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow = 2016;

        consensus.nMinimumChainWork = uint256S("00");
        consensus.defaultAssumeValid = uint256S("00");

        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0xfa;
        pchMessageStart[2] = 0xdb;
        pchMessageStart[3] = 0xf9;

        nDefaultPort = 34568;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1710000001, 2, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("PUT-YOUR-TESTNET-GENESIS-HASH-HERE"));
        assert(genesis.hashMerkleRoot == uint256S("PUT-YOUR-TESTNET-MERKLE-ROOT-HERE"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back("testnet-seed.notecoin.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111); // m or n
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tnt";

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, consensus.hashGenesisBlock}
            }
        };

        chainTxData = ChainTxData{
            1710000001,
            0,
            0.0
        };
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == "main") {
        return std::make_unique<CMainParams>();
    } else if (chain == "test") {
        return std::make_unique<CTestNetParams>();
    }

    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    globalChainParams = CreateChainParams(network);
}
