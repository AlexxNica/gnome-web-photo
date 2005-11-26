@INTLTOOL_SCHEMAS_RULE@

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files = thumbnailer.schemas.in
schema_DATA = $(schema_in_files:%.in=%)

install-data-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $^ ; do \
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$$p >&1 > /dev/null; \
	done \
	fi
endif

cssdir = $(pkgdatadir)
css_DATA = style.css

prefsdir = $(pkgdatadir)
prefs_DATA = prefs.js

EXTRA_DIST = \
	$(schema_in_files)	\
	$(css_DATA)		\
	$(prefs_DATA)

CLEANFILES = $(schema_DATA)
DISTCLEANFILES = $(schema_DATA)
MAINTAINERCLEANFILES = $(schema_DATA)
