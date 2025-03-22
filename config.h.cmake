
#pragma once

/* Name of package */
#define PACKAGE "${PROJECT_NAME}"

/* Name of package */
#define PACKAGE_NAME "${PROJECT_NAME}"

/* Version of package */
#define PACKAGE_VERSION "${CT_VERSION}"
#define PACKAGE_VERSION_WINDOWS ${CT_VERSION_WINDOWS}
#define PACKAGE_VERSION_WINDOWS_STR "${CT_VERSION}.0"

/* The domain to use with gettext */
#define GETTEXT_PACKAGE "${PROJECT_NAME}"

/* Localization directory */
#define CHERRYTREE_LOCALEDIR "${OPT_PRE_CMAKE_INSTALL_PREFIX}${CMAKE_INSTALL_PREFIX}/${PACKAGE_LOCALE_DIR}"

/* data directory */
#define CHERRYTREE_DATADIR "${OPT_PRE_CMAKE_INSTALL_PREFIX}${CMAKE_INSTALL_PREFIX}/${CHERRYTREE_SHARE_INSTALL}"

/* always defined to indicate that i18n is enabled */
#cmakedefine ENABLE_NLS 1

/* folder with root CMakeLists.txt */
#define _CMAKE_SOURCE_DIR "${CMAKE_SOURCE_DIR}"
#define _CMAKE_BINARY_DIR "${CMAKE_BINARY_DIR}"
