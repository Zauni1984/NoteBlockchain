AM_CPPFLAGS = -I$(srcdir) -I$(builddir) $(BOOST_CPPFLAGS) $(BDB_CPPFLAGS) $(SSL_CFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS)
AM_LDFLAGS = $(LDFLAGS)

bin_PROGRAMS = notecoind note-cli note-tx
lib_LIBRARIES = libnoteconsensus.a

noinst_LIBRARIES = \
  libnote_common.a \
  libnote_util.a \
  libnote_crypto.a \
  libnote_pow.a

libnote_common_a_SOURCES = \
  base58.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  key.cpp \
  keystore.cpp \
  netaddress.cpp \
  netbase.cpp \
  policy/feerate.cpp \
  protocol.cpp \
  scheduler.cpp \
  script/sign.cpp \
  script/standard.cpp \
  version.cpp

libnote_util_a_SOURCES = \
  util.cpp \
  utilmoneystr.cpp \
  utilstrencodings.cpp \
  utiltime.cpp \
  sync.cpp \
  support/cleanse.cpp \
  support/lockedpool.cpp \
  fs.cpp \
  random.cpp \
  threadinterrupt.cpp

libnote_crypto_a_SOURCES = \
  crypto/sha256.cpp \
  crypto/sha512.cpp \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha256.cpp \
  crypto/ripemd160.cpp \
  crypto/scrypt.cpp \
  crypto/progpow/progpow.cpp \
  crypto/progpow/progpow_helpers.cpp

libnote_pow_a_SOURCES = \
  pow.cpp \
  kawpow/kawpow.cpp \
  kawpow/kawpow_hash.cpp

libnoteconsensus_a_SOURCES = \
  arith_uint256.cpp \
  primitives/block.cpp \
  primitives/transaction.cpp \
  pubkey.cpp \
  script/interpreter.cpp \
  script/script.cpp \
  script/script_error.cpp \
  script/bitcoinconsensus.cpp \
  consensus/merkle.cpp

notecoind_SOURCES = \
  bitcoind.cpp

notecoind_LDADD = \
  libnote_common.a \
  libnote_util.a \
  libnote_crypto.a \
  libnote_pow.a \
  libnoteconsensus.a \
  $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS)

note_cli_SOURCES = \
  bitcoin-cli.cpp

note_cli_LDADD = \
  libnote_common.a \
  libnote_util.a \
  libnote_crypto.a \
  $(BOOST_LIBS) $(SSL_LIBS)

note_tx_SOURCES = \
  bitcoin-tx.cpp

note_tx_LDADD = \
  libnote_common.a \
  libnote_util.a \
  libnote_crypto.a \
  libnoteconsensus.a \
  $(BOOST_LIBS)

EXTRA_DIST = \
  kawpow/kawpow.cpp \
  kawpow/kawpow_hash.cpp \
  crypto/progpow/progpow.cpp \
  crypto/progpow/progpow_helpers.cpp

CLEANFILES = *.o *.lo *.la *.a *.gcda *.gcno
