bin_PROGRAMS = gnome-web-photo gnome-web-thumbnail

mozilla_include_subdirs = \
	.		\
	content		\
	docshell	\
	dom		\
	gfx		\
	layout		\
	locale		\
	view		\
	webbrwsr	\
	widget		\
	xpcom

# -- Photographer -------------------------------------------------------------

gnome_web_photo_SOURCES = \
	Listener.cpp	\
	Listener.h	\
	Writer.cpp	\
	Writer.h	\
	main.cpp

gnome_web_photo_CPPFLAGS = \
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DSHARE_DIR=\"$(pkgdatadir)\"   	\
	-DMOZILLA_HOME=\"$(MOZILLA_HOME)\"	\
	-DMOZILLA_PREFIX=\"$(MOZILLA_PREFIX)\"	\
	$(AM_CPPFLAGS)

gnome_web_photo_CXXFLAGS = \
	$(DEPENDENCY_CFLAGS) 	\
	$(MOZILLA_CFLAGS)	\
	$(AM_CXXFLAGS)

gnome_web_photo_LDFLAGS = -R$(MOZILLA_HOME)

gnome_web_photo_LDADD = \
	$(DEPENDENCY_LIBS)	\
	$(MOZILLA_LIBS)		\
	$(AM_LDFLAGS)

# -- Thumbnailer --------------------------------------------------------------

gnome_web_thumbnail_SOURCES = \
	Listener.cpp	\
	Listener.h	\
	Writer.cpp	\
	Writer.h	\
	main.cpp

gnome_web_thumbnail_CPPFLAGS = \
	$(addprefix -I$(MOZILLA_INCLUDE_ROOT)/,$(mozilla_include_subdirs))	\
	-DTHUMBNAILER				\
	-DSHARE_DIR=\"$(pkgdatadir)\"   	\
	-DMOZILLA_HOME=\"$(MOZILLA_HOME)\"	\
	-DMOZILLA_PREFIX=\"$(MOZILLA_PREFIX)\"	\
	$(AM_CPPFLAGS)

gnome_web_thumbnail_CXXFLAGS = \
	$(DEPENDENCY_CFLAGS) 	\
	$(MOZILLA_CFLAGS)	\
	$(AM_CXXFLAGS)

gnome_web_thumbnail_LDFLAGS = -R$(MOZILLA_HOME)

gnome_web_thumbnail_LDADD = \
	$(DEPENDENCY_LIBS)	\
	$(MOZILLA_LIBS)		\
	$(AM_LDFLAGS)
