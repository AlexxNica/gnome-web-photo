SUBDIRS = src data po

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = 			\
	autogen.sh		\
	intltool-merge.in	\
	intltool-update.in	\
	intltool-extract.in	\
	COPYING.README		\
	COPYING.LGPL		\
	COPYING.MPL		\
	HACKING			\
	MAINTAINERS

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install --with-gecko=$(GECKO)
