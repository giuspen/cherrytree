
## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

## Set the default command-line flags for the C preprocessor to the value
## obtained from pkg-config via PKG_CHECK_MODULES in configure.ac.  These
## flags are passed to the compiler for both C and C++, in addition to the
## language-specific options.
AM_CPPFLAGS = $(CHERRYTREE_CFLAGS) -DCHERRYTREE_LOCALEDIR=\"${CHERRYTREE_LOCALEDIR}\" -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\"

## Define an executable target, which will be installed into the
## directory named by the predefined variable $(bindir).
bin_PROGRAMS = cherrytree

## Define the non executable data that needs to be installed
## and have a define tell to our software where that dir is
#appdatadir = $(datadir)/@PACKAGE@
#appdata_DATA = img/@PACKAGE@.glade
#AM_CXXFLAGS = -DDATADIR=\"$(datadir)\"

## Define the list of source files for the target. The file extension
## .c is recognized by Automake, and causes it to produce rules which invoke
## the C compiler to produce an object file (.o) from each source file. The
## header files (.h) do not result in object files by themselves, but will be
## included in distribution archives of the project.
cherrytree_SOURCES = src/main.cc \
                     src/ct_app.cc src/ct_app.h \
                     src/main_win.cc src/main_win.h \
                     src/treestore.cc src/treestore.h \
                     src/ct_sqlite3_rw.cc src/ct_xml_rw.cc src/ct_doc_rw.h \
                     src/str_utils.cc src/str_utils.h \
                     src/icons.gresource.cc

## Set the library dependencies for the "example" target to the value obtained
## from pkg-config via PKG_CHECK_MODULES in configure.ac.  These libraries are
## passed to the linker in addition to the other linker flags.
cherrytree_LDADD = $(CHERRYTREE_LIBS)

## Define an independent executable script for inclusion in the distribution
## archive. It will not be installed on an end user's system, however.
dist_noinst_SCRIPTS = autogen.sh

dist_doc_DATA = README


SUBDIRS = po


INTLTOOL_FILES = intltool-extract.in \
                 intltool-merge.in \
                 intltool-update.in

EXTRA_DIST = $(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
                 intltool-merge \
                 intltool-update \
                 po/.intltool-merge-cache
