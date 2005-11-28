# Copyright (C) 2000-2004 Marco Pesenti Gritti
# Copyright (C) 2003, 2004, 2005 Christian Persch
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

AC_INIT([GNOME Web Photo],[0.2],[mailto:epiphany-list@gnome.org],[gnome-web-photo])

GNOME_COMMON_INIT

AC_PREREQ([2.59])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.9 dist-bzip2 no-dist-gzip])

# AC_SUBST([ACLOCAL_AMFLAGS], ["-I $ac_macro_dir \${ACLOCAL_FLAGS}"])
AC_SUBST([ACLOCAL_AMFLAGS], ["\${ACLOCAL_FLAGS}"])

AM_MAINTAINER_MODE

IT_PROG_INTLTOOL([0.34])

GLIB_REQUIRED=2.6.0
GTK_REQUIRED=2.6.3
LIBXML_REQUIRED=2.6.12
LIBGNOMEVFS_REQUIRED=2.9.2

AC_ENABLE_SHARED([yes])
AC_ENABLE_STATIC([no])

AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

AC_ISC_POSIX

AC_PROG_LN_S
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

# AC_PATH_PROG([GLIB_GENMARSHAL],[glib-genmarshal])
# AC_PATH_PROG([GLIB_MKENUMS],[glib-mkenums])

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_CXX_WARNINGS

MORE_WARN_FLAGS=
DEPRECATION_FLAGS=
MOZILLA_WARN_CXXFLAGS="-Wno-ctor-dtor-privacy -Wno-non-virtual-dtor"

if test "x$enable_maintainer_mode" = "xyes"; then
	AC_DEFINE([MAINTAINER_MODE],[1],[Define to enable 'maintainer-only' behaviour])
	enable_debug=yes
        DEPRECATION_FLAGS="-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGCONF_DISABLE_DEPRECATED -DGNOME_VFS_DISABLE_DEPRECATED -DBONOBO_UI_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED -DLIBGLADE_DISABLE_DEPRECATED -DPANGO_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED"
	MOZILLA_WARN_CXXFLAGS="-Wall -Wconversion -Wpointer-arith -Wcast-align -Woverloaded-virtual -Wsynth $MOZILLA_WARN_CXXFLAGS"
fi

PKG_CHECK_MODULES([DEPENDENCY], [\
		  glib-2.0 >= $GLIB_REQUIRED \
		  gtk+-2.0 >= $GTK_REQUIRED \
		  libxml-2.0 >= $LIBXML_REQUIRED \
		  gnome-vfs-2.0 >= $LIBGNOMEVFS_REQUIRED \
		  gnome-vfs-module-2.0 \
		  gconf-2.0 \
		  libpng])
AC_SUBST([DEPENDENCY_CFLAGS])
AC_SUBST([DEPENDENCY_LIBS])

# *****
# GConf
# *****

# Specify the gconf configuration source, 
# default to xml::$(sysconfdir)/gconf/gconf.xml.defaults

AC_PATH_PROG([GCONFTOOL], [gconftool-2], [no])

if test "x$GCONFTOOL" = xno; then
	AC_MSG_ERROR([gconftool-2 not found])
fi

AM_GCONF_SOURCE_2


# *******
# Mozilla
# *******

GECKO_INIT([GECKO])

AC_SUBST([GECKO])
AC_SUBST([GECKO_FLAVOUR])
AC_SUBST([GECKO_INCLUDE_ROOT])
AC_SUBST([GECKO_HOME])
AC_SUBST([GECKO_PREFIX])

case "$GECKO" in
seamonkey) min_version=1.0 ;;
*firefox) min_version=1.4 ;;
xulrunner) min_version=1.8 ;;
esac

PKG_CHECK_MODULES([GECKO], [$GECKO-gtkmozembed $GECKO-xpcom >= $min_version])
AC_SUBST([GECKO_CFLAGS])
AC_SUBST([GECKO_LIBS])

# In xulrunner, gkgfx is in libxul; for all others we need to 
# explicitly link against it

if test "$GECKO" != "xulrunner"; then
	GFX_LIBS="-lgkgfx"
	AC_SUBST([GFX_LIBS])
fi

# Check for PSM headers

AC_MSG_CHECKING([for mozilla security compoment])
GECKO_COMPILE_IFELSE([pipnss],
        [AC_LANG_PROGRAM(
                [[#include <nsIX509Cert.h>]],
                [[nsIX509Cert *c;
                  c->GetIssuer (nsnull);]]
        )],
        [AC_DEFINE([HAVE_PSM], [1],[Define if you have the mozilla NSS headers installed]) have_psm=yes],
        [have_psm=no])
AC_MSG_RESULT([$have_psm])

AM_CONDITIONAL([HAVE_PSM],[test "x$have_psm" = "xyes"])

# Needed since 1.8b2
# Define this down here so it doesn't affect the API checks above

AC_DEFINE([MOZILLA_INTERNAL_API],[1],[Define for access to internal mozilla API])

# ******************
# Portability checks
# ******************

if test "x$enable_maintainer_mode" = "xyes"; then

	AC_LANG_PUSH([C])
	FLAGS="-Wdeclaration-after-statement"
	_SAVE_CFLAGS=$CFLAGS
	CFLAGS="$CFLAGS $AM_CFLAGS"

	AC_CACHE_CHECK([for compiler $FLAGS option],
		ac_cv_have_declaration_after_statement,
		[AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([[int main () { return 0; }]])],
			[ac_cv_have_declaration_after_statement="yes"],
			[ac_cv_have_declaration_after_statement="no"],
			[ac_cv_have_declaration_after_statement="maybe"])])
	
	if test "x$ac_cv_have_declaration_after_statement" = "xyes"; then
		MORE_WARN_FLAGS="$MORE_WARN_FLAGS $FLAGS"
	fi
	CFLAGS="$_SAVE_CFLAGS"
	AC_LANG_POP([C])
fi

SUPPRESSION_CXXFLAGS="-Wconversion -Wpointer-arith -Wcast-align -Woverloaded-virtual -Wsynth -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor"

# *****************
# Add warning flags
# *****************

AM_CPPFLAGS="$AM_CPPFLAGS $DEPRECATION_FLAGS"
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS $MORE_WARN_FLAGS"
AM_CXXFLAGS="$AM_CXXFLAGS $WARN_CXXFLAGS $SUPPRESSION_CXXFLAGS"
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

# ********************
# Internationalisation
# ********************

# Translators: new languages must be added to the po/LINGUAS file
ALL_LINGUAS="`cat "$srcdir/po/LINGUAS" | grep -v '^#'`"
AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/po/LINGUAS'])

GETTEXT_PACKAGE=gnome-web-photo
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[Gettext package])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR([GNOMELOCALEDIR])

# ************
# Output files
# ************

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
po/Makefile.in
])

AC_OUTPUT
