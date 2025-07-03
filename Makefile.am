ACLOCAL_AMFLAGS = -I build-aux/m4
SUBDIRS = src
if ENABLE_MAN
SUBDIRS += doc/man
endif

EXTRA_DIST = \
    autogen.sh \
    COPYING \
    README.md

dist_noinst_SCRIPTS = autogen.sh

CLEANFILES = \
    test_bitcoin.coverage/ \
    total.coverage/ \
    coverage_percent.txt \
    $(top_builddir)/NoteBlockchain-Qt.app \
    *.dmg \
    *.exe

# Platform-specific packaging targets (optional)
if TARGET_DARWIN
appbundle: $(top_builddir)/NoteBlockchain-Qt.app
deploy: $(top_builddir)/NoteCoin.dmg
endif

if TARGET_WINDOWS
deploy: $(PACKAGE)-$(PACKAGE_VERSION)-win$(WINDOWS_BITS)-setup$(EXEEXT)
endif

# Default binaries
BITCOIND_BIN=$(top_builddir)/src/noted$(EXEEXT)
BITCOIN_CLI_BIN=$(top_builddir)/src/note-cli$(EXEEXT)

$(BITCOIND_BIN): FORCE
	$(MAKE) -C src $(@F)

$(BITCOIN_CLI_BIN): FORCE
	$(MAKE) -C src $(@F)

.PHONY: FORCE deploy clean-local
