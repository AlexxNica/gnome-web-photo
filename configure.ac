# Copyright © 2000-2004 Marco Pesenti Gritti
# Copyright © 2003, 2004, 2005 Christian Persch
# Copyright © 2011 Vincent Untz
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

AC_INIT([GNOME Web Photo],[0.10.1],[mailto:epiphany-list@gnome.org],[gnome-web-photo])

GNOME_COMMON_INIT

AC_PREREQ([2.59])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.9 dist-bzip2 no-dist-gzip])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

IT_PROG_INTLTOOL([0.35.0])

GLIB_REQUIRED=2.14.0
GTK2_REQUIRED=2.20.0
GTK3_REQUIRED=2.99.3
WEBKIT_REQUIRED=1.1.23

AC_ENABLE_SHARED([yes])
AC_ENABLE_STATIC([no])

AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

AC_ISC_POSIX

AC_PROG_LN_S
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AC_C_BIGENDIAN

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])

MORE_WARN_FLAGS=
DEPRECATION_FLAGS=

if test "x$enable_maintainer_mode" = "xyes"; then
	AC_DEFINE([MAINTAINER_MODE],[1],[Define to enable 'maintainer-only' behaviour])
	enable_debug=yes
        DEPRECATION_FLAGS="-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGCONF_DISABLE_DEPRECATED -DGNOME_VFS_DISABLE_DEPRECATED -DBONOBO_UI_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED -DLIBGLADE_DISABLE_DEPRECATED -DPANGO_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED"
fi

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 3.0)])],
  [case "$with_gtk" in
     2.0|3.0) ;;
     *) AC_MSG_ERROR([invalid gtk version specified]) ;;
   esac],
  [with_gtk=3.0])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
  2.0)
    DEPENDENCY_PC="glib-2.0 >= $GLIB_REQUIRED gtk+-2.0 >= $GTK2_REQUIRED webkit-1.0 >= $WEBKIT_REQUIRED gconf-2.0"
    GTKUNIXPRINT_PC="gtk+-unix-print-2.0"
    ;;
  3.0)
    DEPENDENCY_PC="glib-2.0 >= $GLIB_REQUIRED gtk+-3.0 >= $GTK3_REQUIRED webkitgtk-3.0 >= $WEBKIT_REQUIRED"
    GTKUNIXPRINT_PC="gtk+-unix-print-3.0"
    AC_DEFINE([HAVE_GNOME3], 1,
              [Use GNOME 3 technologies])
    ;;
esac
AM_CONDITIONAL(HAVE_GNOME3, test "$with_gtk" == 3.0)

PKG_CHECK_MODULES([DEPENDENCY], [$DEPENDENCY_PC])
AC_SUBST([DEPENDENCY_CFLAGS])
AC_SUBST([DEPENDENCY_LIBS])

PKG_CHECK_MODULES([GTKUNIXPRINT], [$GTKUNIXPRINT_PC],
                  [with_gtk_unix_print="yes"], [with_gtk_unix_print="no"])
AC_SUBST([GTKUNIXPRINT_CFLAGS])
AC_SUBST([GTKUNIXPRINT_LIBS])
if test "$with_gtk_unix_print" == "yes"; then
  AC_DEFINE([HAVE_GTK_UNIX_PRINT], 1,
            [Build with gtk-unix-print features])
fi

# *****
# GConf
# *****

if test "$with_gtk" == 2.0; then
  AC_PATH_PROG([GCONFTOOL], [gconftool-2], [no])

  if test "x$GCONFTOOL" = xno; then
    AC_MSG_ERROR([gconftool-2 not found])
  fi

fi

AM_GCONF_SOURCE_2

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

# *****************
# Add warning flags
# *****************

AM_CPPFLAGS="$AM_CPPFLAGS $DEPRECATION_FLAGS"
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS $MORE_WARN_FLAGS"
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

# ********************
# Internationalisation
# ********************

# Translators: new languages must be added to the po/LINGUAS file

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
