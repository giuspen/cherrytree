
AC_INIT([CherryTree], [0.1], [giuspen@gmail.com],
        [cherrytree], [http://www.giuspen.com])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10])

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR

PKG_CHECK_MODULES([CHERRYTREE], [gtkmm-3.0 gtksourceviewmm-3.0 glibmm-2.4 libxml++-2.6 sqlite3 cpputest])

AC_PREFIX_DEFAULT("/usr")

AC_CONFIG_FILES([po/Makefile.in Makefile])


IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=cherrytree
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                   [The domain to use with gettext])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18])

CHERRYTREE_LOCALEDIR=[${datadir}/locale]
AC_SUBST(CHERRYTREE_LOCALEDIR)


AC_OUTPUT


python3 scripts/gresource_to_c.py
