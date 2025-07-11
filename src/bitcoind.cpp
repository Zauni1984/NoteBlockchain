// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2024 NoteCoin Developers
// Distributed under the MIT software license, see COPYING or http://opensource.org/licenses/MIT.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <chainparams.h>
#include <clientversion.h>
#include <compat.h>
#include <fs.h>
#include <rpc/server.h>
#include <init.h>
#include <noui.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <httpserver.h>
#include <httprpc.h>

#include <boost/thread.hpp>
#include <csignal>
#include <cstdio>

/*! \mainpage NoteCoin Core Daemon Developer Documentation
 *
 * \section intro_sec Introduction
 *
 * This is the developer documentation for the NoteCoin full node daemon.
 * NoteCoin is a decentralized cryptocurrency, using peer-to-peer technology
 * to operate without a central authority. This daemon maintains the network,
 * validates transactions, and connects miners and users to the blockchain.
 *
 * Licensed under the MIT license.
 */

void WaitForShutdown()
{
    while (!ShutdownRequested()) {
        MilliSleep(200);
    }
    Interrupt();
}

bool AppInit(int argc, char* argv[])
{
    bool success = false;

    // Parse CLI parameters
    gArgs.ParseParameters(argc, argv);

    // Show help or version
    if (gArgs.IsArgSet("-?") || gArgs.IsArgSet("-h") ||
        gArgs.IsArgSet("-help") || gArgs.IsArgSet("-version")) {

        std::string header = strprintf("%s Daemon version %s\n", PACKAGE_NAME, FormatFullVersion());

        if (gArgs.IsArgSet("-version")) {
            header += FormatParagraph(LicenseInfo());
        } else {
            header += "\nUsage:\n"
                      "  notecoind [options]                     Start NoteCoin Daemon\n\n" +
                      HelpMessage(HMM_BITCOIND);
        }

        std::fprintf(stdout, "%s", header.c_str());
        return true;
    }

    try {
        if (!fs::is_directory(GetDataDir(false))) {
            std::fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n",
                         gArgs.GetArg("-datadir", "").c_str());
            return false;
        }

        try {
            gArgs.ReadConfigFile(gArgs.GetArg("-conf", BITCOIN_CONF_FILENAME));
        } catch (const std::exception& e) {
            std::fprintf(stderr, "Error reading configuration file: %s\n", e.what());
            return false;
        }

        try {
            SelectParams(ChainNameFromCommandLine());
        } catch (const std::exception& e) {
            std::fprintf(stderr, "Error: %s\n", e.what());
            return false;
        }

        for (int i = 1; i < argc; ++i) {
            if (!IsSwitchChar(argv[i][0])) {
                std::fprintf(stderr, "Error: Unexpected token '%s'. See notecoind -h for options.\n", argv[i]);
                return false;
            }
        }

        gArgs.SoftSetBoolArg("-server", true);
        InitLogging();
        InitParameterInteraction();

        if (!AppInitBasicSetup()) return false;
        if (!AppInitParameterInteraction()) return false;
        if (!AppInitSanityChecks()) return false;

        if (gArgs.GetBoolArg("-daemon", false)) {
#if HAVE_DECL_DAEMON
            std::fprintf(stdout, "NoteCoin server starting\n");
            if (daemon(1, 0)) {
                std::fprintf(stderr, "Error: daemon() failed: %s\n", std::strerror(errno));
                return false;
            }
#else
            std::fprintf(stderr, "Error: -daemon not supported on this OS\n");
            return false;
#endif
        }

        if (!AppInitLockDataDirectory()) return false;
        success = AppInitMain();
    } catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInit()");
    } catch (...) {
        PrintExceptionContinue(nullptr, "AppInit()");
    }

    if (!success) {
        Interrupt();
    } else {
        WaitForShutdown();
    }

    Shutdown();
    return success;
}

int main(int argc, char* argv[])
{
    SetupEnvironment();
    noui_connect(); // Connect signal handlers
    return AppInit(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
