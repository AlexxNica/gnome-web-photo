bin_PROGRAMS = gnome-web-photo

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	gfx		\
	layout		\
	locale		\
	necko		\
	view		\
	webbrwsr	\
	widget		\
	xpcom

gnome_web_photo_SOURCES = \
	Components.cpp	\
	Components.h	\
	Listener.cpp	\
	Listener.h	\
	Prefs.cpp	\
	Prefs.h		\
	Printer.cpp	\
	Printer.h	\
	Writer.cpp	\
	Writer.h	\
	main.cpp

gnome_web_photo_CPPFLAGS = \
	$(addprefix -I$(GECKO_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DSHARE_DIR=\"$(pkgdatadir)\"   	\
	-DLOCALEDIR=\"$(datadir)/locale\"	\
	-DGECKO_HOME=\"$(GECKO_HOME)\"	\
	-DGECKO_PREFIX=\"$(GECKO_PREFIX)\"	\
	$(AM_CPPFLAGS)

gnome_web_photo_CXXFLAGS = \
	$(DEPENDENCY_CFLAGS) 	\
	$(GECKO_CFLAGS)	\
	$(AM_CXXFLAGS)

gnome_web_photo_LDFLAGS = \
	-R$(GECKO_HOME)	\
	$(AM_LDFLAGS)

gnome_web_photo_LDADD = \
	$(DEPENDENCY_LIBS)	\
	$(GECKO_LIBS)		\
	$(GFX_LIBS)


install-exec-hook: gnome-web-photo
	cd $(DESTDIR)$(bindir) && \
	rm -f gnome-web-thumbnail gnome-web-print && \
	$(LN_S) -f gnome-web-photo gnome-web-thumbnail && \
	$(LN_S) -f gnome-web-photo gnome-web-print

uninstall-hook:
	cd $(DESTDIR)$(bindir) && \
	rm -f gnome-web-thumbnail gnome-web-print
