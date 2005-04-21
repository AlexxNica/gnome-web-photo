SUBDIRS = src data po

EXTRA_DIST = 			\
	intltool-merge.in	\
	intltool-update.in	\
	intltool-extract.in	\
	COPYING.README		\
	HACKING			\
	MAINTAINERS		\
	po/LINGUAS

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install --with-mozilla=$(MOZILLA)
