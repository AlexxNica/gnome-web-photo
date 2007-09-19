SUBDIRS = src data po

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = 			\
	autogen.sh		\
	intltool-merge.in	\
	intltool-update.in	\
	intltool-extract.in	\
	ChangeLog		\
	ChangeLog-20090919	\
	COPYING.README		\
	COPYING.LGPL		\
	HACKING			\
	MAINTAINERS

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install --with-gecko=$(GECKO)

# Build ChangeLog from SVN history (r101 was the first ChangeLog-less checkin).
# Only build this when in an svn checkout.

ChangeLog:
	@if test -f $(top_srcdir)/.svn/entries; then \
		svn log -v --xml -r HEAD:101 http://svn.gnome.org/svn/epiphany | \
			xsltproc --stringparam strip-prefix "gnome-web-photo/trunk" \
				 --stringparam include-rev "yes" $(top_srcdir)/svn2cl.xsl - > $@; \
	fi

.PHONY: ChangeLog
