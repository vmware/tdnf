# The `:;' works around a Bash 3.2 bug when the output is not writeable.
$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	:;{ \
	echo '# Signature of the current package.' && \
	echo 'm4_define([AT_PACKAGE_NAME],' && \
	echo '  [$(PACKAGE_NAME)])' && \
	echo 'm4_define([AT_PACKAGE_TARNAME],' && \
	echo '  [$(PACKAGE_TARNAME)])' && \
	echo 'm4_define([AT_PACKAGE_VERSION],' && \
	echo '  [$(PACKAGE_VERSION)])' && \
	echo 'm4_define([AT_PACKAGE_STRING],' && \
	echo '  [$(PACKAGE_STRING)])' && \
	echo 'm4_define([AT_PACKAGE_BUGREPORT],' && \
	echo '  [$(PACKAGE_BUGREPORT)])'; \
	echo 'm4_define([AT_PACKAGE_URL],' && \
	echo '  [$(PACKAGE_URL)])'; \
	} >'$(srcdir)/package.m4'


TESTSUITE = $(srcdir)/testsuite
EXTRA_DIST = init.at $(srcdir)/package.m4 $(TESTSUITE)

#testsuites
TESTSUITE_AT = testsuite.at
TESTSUITE_AT += version.at
TESTSUITE_AT += list.at
TESTSUITE_AT += install.at
TESTSUITE_AT += erase.at
TESTSUITE_AT += update.at
TESTSUITE_AT += downgrade.at
EXTRA_DIST += $(TESTSUITE_AT)

DISTCLEANFILES = atconfig testsuite

check-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(abs_top_srcdir)/tools/cli' \
	$(TESTSUITEFLAGS)
     
installcheck-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(abs_top_srcdir)/tools/cli' \
	$(TESTSUITEFLAGS)
     
clean-local:
	test ! -f '$(TESTSUITE)' || \
	$(SHELL) '$(TESTSUITE)' --clean
	rm -rf testroot

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te
AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(srcdir)/package.m4 init.at $(TESTSUITE_AT)
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@
