
AC_INIT([Hello World], [0.1], [giuspen@gmail.com],
        [helloworld], [http://www.giuspen.net])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 -Wall no-define])

AC_CONFIG_HEADERS([config.h])

# check for Cpp compiler
AC_PROG_CXX

PKG_CHECK_MODULES([HELLOWORLD], [gtkmm-3.0 >= 3.0.0])

# change default prefix
AC_PREFIX_DEFAULT("/usr")

AC_CONFIG_FILES([po/Makefile.in Makefile])


IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=helloworld
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                   [The domain to use with gettext])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18])

HELLOWORLD_LOCALEDIR=[${datadir}/locale]
AC_SUBST(HELLOWORLD_LOCALEDIR)


AC_OUTPUT
