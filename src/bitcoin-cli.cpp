// Optimierte und modernisierte Version von bitcoin-cli.cpp
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// SPDX-License-Identifier: MIT

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <chainparamsbase.h>
#include <clientversion.h>
#include <fs.h>
#include <rpc/client.h>
#include <rpc/protocol.h>
#include <util.h>
#include <utilstrencodings.h>
#include <support/events.h>

#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include <univalue.h>
#include <memory>
#include <iostream>
#include <thread>
#include <stdexcept>

static constexpr const char* DEFAULT_RPCCONNECT = "127.0.0.1";
static constexpr int DEFAULT_HTTP_CLIENT_TIMEOUT = 900;
static constexpr bool DEFAULT_NAMED = false;
static constexpr int CONTINUE_EXECUTION = -1;

std::string HelpMessageCli() {
    auto defaultBaseParams = CreateBaseChainParams(CBaseChainParams::MAIN);
    auto testnetBaseParams = CreateBaseChainParams(CBaseChainParams::TESTNET);

    std::string usage;
    usage += HelpMessageGroup("Options:");
    usage += HelpMessageOpt("-?", "This help message");
    usage += HelpMessageOpt("-conf=<file>", strprintf("Specify config file (default: %s)", BITCOIN_CONF_FILENAME));
    usage += HelpMessageOpt("-datadir=<dir>", "Specify data directory");
    usage += HelpMessageOpt("-getinfo", "Get general information from remote node");
    AppendParamsHelpMessages(usage);
    usage += HelpMessageOpt("-named", strprintf("Use named RPC arguments (default: %s)", DEFAULT_NAMED));
    usage += HelpMessageOpt("-rpcconnect=<ip>", strprintf("Connect to node at <ip> (default: %s)", DEFAULT_RPCCONNECT));
    usage += HelpMessageOpt("-rpcport=<port>", strprintf("Connect to JSON-RPC on <port> (default: %u or testnet: %u)", defaultBaseParams->RPCPort(), testnetBaseParams->RPCPort()));
    usage += HelpMessageOpt("-rpcwait", "Wait for RPC server to be ready");
    usage += HelpMessageOpt("-rpcuser=<user>", "Username for JSON-RPC");
    usage += HelpMessageOpt("-rpcpassword=<pw>", "Password for JSON-RPC");
    usage += HelpMessageOpt("-rpcclienttimeout=<n>", strprintf("Timeout for HTTP requests (default: %d)", DEFAULT_HTTP_CLIENT_TIMEOUT));
    usage += HelpMessageOpt("-stdinrpcpass", "Read RPC password from stdin");
    usage += HelpMessageOpt("-stdin", "Read extra args from stdin");
    usage += HelpMessageOpt("-rpcwallet=<wallet>", "Send RPC to specific wallet on node");

    return usage;
}

class CConnectionFailed : public std::runtime_error {
public:
    explicit CConnectionFailed(const std::string& msg) : std::runtime_error(msg) {}
};

int AppInitRPC(int argc, char* argv[]) {
    gArgs.ParseParameters(argc, argv);
    if (argc < 2 || gArgs.IsArgSet("-?") || gArgs.IsArgSet("-h") || gArgs.IsArgSet("-help") || gArgs.IsArgSet("-version")) {
        std::string usage = strprintf("%s RPC client version %s\n", PACKAGE_NAME, FormatFullVersion());
        if (!gArgs.IsArgSet("-version")) {
            usage += "\nUsage:\n";
            usage += "  notecoin-cli [options] <command> [params]\n";
            usage += "  notecoin-cli [options] -named <command> [name=value]\n";
            usage += "  notecoin-cli [options] help\n";
            usage += "  notecoin-cli [options] help <command>\n\n";
            usage += HelpMessageCli();
        }
        std::cout << usage;
        return (argc < 2) ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    if (!fs::is_directory(GetDataDir(false))) {
        std::cerr << "Error: Specified data directory does not exist." << std::endl;
        return EXIT_FAILURE;
    }

    try {
        gArgs.ReadConfigFile(gArgs.GetArg("-conf", BITCOIN_CONF_FILENAME));
        SelectBaseParams(ChainNameFromCommandLine());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (gArgs.GetBoolArg("-rpcssl", false)) {
        std::cerr << "Error: SSL mode for RPC is deprecated." << std::endl;
        return EXIT_FAILURE;
    }

    return CONTINUE_EXECUTION;
}

int main(int argc, char* argv[]) {
    SetupEnvironment();
    if (!SetupNetworking()) {
        std::cerr << "Error: Initializing networking failed" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        int initResult = AppInitRPC(argc, argv);
        if (initResult != CONTINUE_EXECUTION) return initResult;
    } catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInitRPC()");
        return EXIT_FAILURE;
    }

    try {
        return CommandLineRPC(argc, argv);
    } catch (const std::exception& e) {
        PrintExceptionContinue(&e, "CommandLineRPC()");
        return EXIT_FAILURE;
    }
}
