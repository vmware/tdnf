TESTSUITE = $(srcdir)/testsuite
EXTRA_DIST = init.at $(TESTSUITE)

#testsuites
TESTSUITE_AT = testsuite.at
TESTSUITE_AT += version.at
TESTSUITE_AT += list.at
EXTRA_DIST += $(TESTSUITE_AT)

#testsuite data
EXTRA_DIST += data/RPMS/tdnf-default-1.0.0.x86_64.rpm

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te
AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(srcdir)/package.m4 init.at $(TESTSUITE_AT)
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	@{ \
	echo '# Signature of the current package.'; \
	echo 'm4_define([AT_PACKAGE_NAME],      [$(PACKAGE_NAME)])'; \
	echo 'm4_define([AT_PACKAGE_TARNAME],   [$(PACKAGE_TARNAME)])'; \
	echo 'm4_define([AT_PACKAGE_VERSION],   [$(PACKAGE_VERSION)])'; \
	echo 'm4_define([AT_PACKAGE_STRING],    [$(PACKAGE_STRING)])'; \
	echo 'm4_define([AT_PACKAGE_BUGREPORT]  [$(PACKAGE_BUGREPORT)])'; \
	echo 'm4_define([AT_PACKAGE_URL],       [$(PACKAGE_URL)])'; \
	} >'$(srcdir)/package.m4'
EXTRA_DIST += package.m4

DISTCLEANFILES = atconfig

check-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)
     
installcheck-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' \
	$(TESTSUITEFLAGS)
     
clean-local:
	test ! -f '$(TESTSUITE)' || \
	$(SHELL) '$(TESTSUITE)' --clean

