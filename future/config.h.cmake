
#pragma once

/* Name of package */
#define PACKAGE "${PROJECT_NAME}"

/* Name of package */
#define PACKAGE_NAME "${PROJECT_NAME}"

/* The domain to use with gettext */
#define GETTEXT_PACKAGE "${PROJECT_NAME}"

/* Localization directory */
#define CHERRYTREE_LOCALEDIR "${CMAKE_INSTALL_PREFIX}/${PACKAGE_LOCALE_DIR}"

/* data directory */
#define CHERRYTREE_DATADIR "${CMAKE_INSTALL_PREFIX}/${CHERRYTREE_SHARE_INSTALL}"


/* always defined to indicate that i18n is enabled */
#cmakedefine ENABLE_NLS 1

/* folder with data for tests */
#define _UNITTEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data"


