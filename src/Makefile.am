bin_PROGRAMS = gnome-web-photo

gecko_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	embed_base	\
	gfx		\
	gtkembedmoz	\
	layout		\
	locale		\
	necko		\
	pipnss		\
	pref		\
	uriloader	\
	view		\
	webbrwsr	\
	widget		\
	windowwatcher	\
	xpcom

gnome_web_photo_SOURCES = \
	Components.cpp	\
	Components.h	\
	Embed.cpp	\
	Embed.h		\
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
	$(DEPENDENCY_CFLAGS)			\
	$(addprefix -I$(GECKO_INCLUDE_ROOT)/,$(gecko_include_subdirs))	\
	-DSHARE_DIR=\"$(pkgdatadir)\"   	\
	-DLOCALEDIR=\"$(datadir)/locale\"	\
	-DGECKO_HOME=\"$(GECKO_HOME)\"	\
	-DGECKO_PREFIX=\"$(GECKO_PREFIX)\"	\
        -DXPCOM_GLUE_USE_NSPR			\
        -DXPCOM_GLUE				\
	$(AM_CPPFLAGS)

gnome_web_photo_CXXFLAGS = \
	$(DEPENDENCY_CFLAGS) 	\
	$(LIBXUL_INCLUDES)	\
	$(LIBXUL_CXXFLAGS)	\
	$(AM_CXXFLAGS)

gnome_web_photo_LDFLAGS = \
	-R$(LIBXUL_LIBDIR)	\
	$(AM_LDFLAGS)

gnome_web_photo_LDADD = \
	$(DEPENDENCY_LIBS)	\
	$(JPEG_LIBS)		\
	$(LIBXUL_LIBS)		\
	-lxpcomglue

install-exec-hook: gnome-web-photo
	cd $(DESTDIR)$(bindir) && \
	rm -f gnome-web-thumbnail gnome-web-print && \
	$(LN_S) -f gnome-web-photo gnome-web-thumbnail && \
	$(LN_S) -f gnome-web-photo gnome-web-print

uninstall-hook:
	cd $(DESTDIR)$(bindir) && \
	rm -f gnome-web-thumbnail gnome-web-print
