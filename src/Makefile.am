bin_PROGRAMS = gnome-web-photo

gnome_web_photo_SOURCES =		\
	gnome-web-photo.c		\
	photo-offscreen-window.c	\
	photo-offscreen-window.h

gnome_web_photo_CPPFLAGS = \
	$(DEPENDENCY_CFLAGS)			\
	$(GTKUNIXPRINT_CFLAGS)			\
	-DPKGDATADIR=\"$(pkgdatadir)\"		\
	-DLOCALEDIR=\"$(datadir)/locale\"	\
	$(AM_CPPFLAGS)

gnome_web_photo_CFLAGS = \
	$(AM_CFLAGS)

gnome_web_photo_LDFLAGS = \
	$(AM_LDFLAGS)

gnome_web_photo_LDADD = \
	$(DEPENDENCY_LIBS) \
	$(GTKUNIXPRINT_LIBS)

install-exec-hook: gnome-web-photo
	cd $(DESTDIR)$(bindir) && \
	rm -f gnome-web-thumbnail gnome-web-print && \
	$(LN_S) -f gnome-web-photo gnome-web-thumbnail && \
	$(LN_S) -f gnome-web-photo gnome-web-print

uninstall-hook:
	cd $(DESTDIR)$(bindir) && \
	rm -f gnome-web-thumbnail gnome-web-print

-include $(top_srcdir)/git.mk
