schema_in_files = thumbnailer.schemas.in
thumbnailer_DATA_files = gnome-web-photo.thumbnailer

cssdir = $(pkgdatadir)
css_DATA = style.css

if !HAVE_GNOME3
@INTLTOOL_SCHEMAS_RULE@

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = $(schema_in_files:%.in=%)

install-data-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $^ ; do \
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p >&1 > /dev/null; \
	done \
	fi
endif
endif

if HAVE_GNOME3
thumbnailerdir = $(datadir)/thumbnailers
thumbnailer_DATA = $(thumbnailer_DATA_files)
endif

EXTRA_DIST = \
	$(schema_in_files)		\
	$(thumbnailer_DATA_files)	\
	$(css_DATA)

CLEANFILES = $(schema_DATA)
DISTCLEANFILES = $(schema_DATA)
MAINTAINERCLEANFILES = $(schema_DATA)

-include $(top_srcdir)/git.mk
