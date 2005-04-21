@INTLTOOL_SCHEMAS_RULE@

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files = thumbnailer.schemas.in
schema_DATA = $(schema_in_files:%.in=%)

install-data-local:
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $(schema_DATA) ; do \
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p >&1 > /dev/null; \
	done \
	fi
endif

prefsdir = $(pkgdatadir)
prefs_DATA = prefs.js

EXTRA_DIST = \
	$(schema_in_files)	\
	$(prefs_DATA)

CLEANFILES = $(schema_DATA)
DISTCLEANFILES = $(schema_DATA)
MAINTAINERCLEANFILES = $(schema_DATA)
