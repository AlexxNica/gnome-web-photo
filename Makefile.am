SUBDIRS = src data po

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = 			\
	autogen.sh		\
	ChangeLog		\
	COPYING.README		\
	COPYING.LGPL		\
	HACKING			\
	gnome-web-photo.doap

DISTCLEANFILES =

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install --with-gecko=$(GECKO)

ChangeLog:
	$(AM_V_GEN) if test -f $(top_srcdir)/.git/HEAD; then \
		git log --stat > $@; \
	fi

.PHONY: ChangeLog
