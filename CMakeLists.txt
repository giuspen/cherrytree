cmake_minimum_required(VERSION 3.5)

project(cherrytree)

file(READ "debian/changelog" CHANGELOG)
string(REGEX MATCH "cherrytree +\\(([0-9]+)\\.([0-9]+)\\.([0-9]+)[-+]" _ ${CHANGELOG})
set(CT_VERSION ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3})
set(CT_VERSION_WINDOWS ${CMAKE_MATCH_1},${CMAKE_MATCH_2},${CMAKE_MATCH_3},0)
message(STATUS "CT_VERSION = ${CT_VERSION}")

# APPLE has issues with installing in /usr/bin because of SIP
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND APPLE)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "..." FORCE)
elseif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif()

if(NOT PYTHON_EXEC)
  set(PYTHON_EXEC "python3")
endif(NOT PYTHON_EXEC)

if(SNAP_OPT_PRE_CMAKE_INSTALL_PREFIX)
  set(OPT_PRE_CMAKE_INSTALL_PREFIX "${SNAP_OPT_PRE_CMAKE_INSTALL_PREFIX}")
  add_compile_options(-D_SNAP_BUILD)
endif()

option(USE_NLS "Add locales support" ON)
option(USE_SHARED_FMT_SPDLOG "Use shared fmt and spdlog (not those bundled)" ON)
option(USE_SHARED_GTEST_GMOCK "Use shared gtest and gmock (not those bundled)" OFF)
if(WIN32)
  option(USE_VTE "Add virtual terminal support" OFF)
else()
  option(USE_VTE "Add virtual terminal support" ON)
endif()

# used to install locale, packagers might overwrite this
if(USE_NLS)
  if(NOT PACKAGE_LOCALE_DIR)
    set(PACKAGE_LOCALE_DIR "share/locale")
  endif(NOT PACKAGE_LOCALE_DIR)
endif()

# used to install data, packagers might overwrite this
if(NOT SHARE_INSTALL)
  set(SHARE_INSTALL "share" CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
endif(NOT SHARE_INSTALL)
set(CHERRYTREE_SHARE_INSTALL "${SHARE_INSTALL}/cherrytree")

set(CPACK_PACKAGE_NAME "cherrytree")
set(CPACK_PACKAGE_CONTACT "Giuseppe Penone <giuspen@gmail.com>")
set(CPACK_PACKAGE_VENDOR ${CPACK_PACKAGE_CONTACT})
set(CPACK_PACKAGE_VERSION ${CT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Hierarchical Note Taking Application, featuring Rich Text and Syntax Highlighting.")
set(CPACK_PACKAGE_DESCRIPTION "A hierarchical note taking application, featuring rich text and syntax highlighting, storing data in either a single file (xml or sqlite) or multiple files and directories
   Cherrytree is not just about having a place to write notes and to-do items and keeping them organized, it's also a place you can store links, pictures, tables, even entire documents. It can be your one program for all the miscellaneous information you have and want to keep. All those little bits of information you have scattered around your hard drive can be conveniently placed into a Cherrytree document where you can easily find it.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_CONTACT})
set(CPACK_CREATE_DESKTOP_LINKS ${CPACK_PACKAGE_CONTACT})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_CONTACT})
set(CPACK_STRIP_FILES TRUE)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "texlive-latex-base, dvipng")
if(EXISTS "/etc/fedora-release")
  set(CPACK_RPM_PACKAGE_REQUIRES "gtkmm30, gtksourceviewmm3, gspell, libxml++, libcurl, uchardet, fmt, spdlog")
  if(USE_VTE)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, vte291")
  endif()
else()
  set(CPACK_RPM_PACKAGE_REQUIRES "libgtkmm-3_0-1, libgtksourceviewmm-3_0-0, libgspell-1-2, libxml++-2_6-2, libsqlite3-0, libcurl4, libuchardet0, libfmt8, libspdlog1")
  if(USE_VTE)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libvte-2_91-0")
  endif()
endif()
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.giuspen.net/cherrytree/")
set(CPACK_RPM_PACKAGE_URL "https://www.giuspen.net/cherrytree/")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst")

include(CPack)

set(CMAKE_CXX_STANDARD 17)

if(USE_SHARED_FMT_SPDLOG)
  add_definitions(-DSHARED_FMT_SPDLOG)
  add_definitions(-DSPDLOG_FMT_EXTERNAL)
endif()
add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
if(USE_NLS)
  add_definitions(-DHAVE_NLS)
endif()
if(USE_VTE)
  add_definitions(-DHAVE_VTE)
endif()

# temporarily disable these warnings: Only <gtksourceview/gtksource.h> can be included directly.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-Wno-cpp)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options("-Wno-#warnings")
  add_compile_options(-Wthread-safety)
  add_compile_options(-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR)
endif()

if(APPLE)
  link_directories(/usr/local/lib /usr/local/opt/gettext/lib)
  include_directories(/usr/local/opt/gettext/include)
  set(CMAKE_MACOSX_RPATH 1)
  set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/lib/pkgconfig:/opt/X11/lib/pkgconfig:/usr/local/opt/libxml2/lib/pkgconfig:/usr/local/opt/libxml++/lib/pkgconfig")
  # to fix libxml++2.6 compilation
  add_compile_options(-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR)
endif()

add_compile_options(-Wno-deprecated -Wno-deprecated-declarations)
if(CMAKE_BUILD_TYPE STREQUAL "")
  add_compile_options(-O2)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0 -g)
endif()

# Create the configuration files config.h in the root dir
configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_SOURCE_DIR}/config.h)

option(BUILD_TESTING "Build tests" ON)
# the ppa build server must not build google test because that would add those files to the debian package
if(BUILD_TESTING AND NOT DEFINED ENV{DEB_BUILD_OPTIONS})
  option(AUTO_RUN_TESTING "Auto run tests" ON)
  # I haven't found yet a way to run the unit tests with necessary xvfb or xvfb-run in the flatpak build
  if(AUTO_RUN_TESTING AND NOT DEFINED ENV{FLATPAK_BUILDER_BUILDDIR})
    message("Build tests ON, auto run ON")
  else()
    message("Build tests ON, auto run OFF")
  endif()
else()
  message("Build tests OFF")
endif()

include(FindPkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0 REQUIRED)
pkg_check_modules(GTKSVMM gtksourceviewmm-3.0 REQUIRED)
pkg_check_modules(GLIBMM glibmm-2.4 REQUIRED)
pkg_check_modules(PANGOMM pangomm-1.4 REQUIRED)
pkg_check_modules(GSPELL gspell-1 REQUIRED)
pkg_check_modules(LIBXML libxml++-2.6 REQUIRED)
pkg_check_modules(SQLITE sqlite3 REQUIRED)
pkg_check_modules(CURL libcurl REQUIRED)
pkg_check_modules(UCHARDET uchardet REQUIRED)
pkg_check_modules(FRIBIDI fribidi REQUIRED)
if(USE_SHARED_FMT_SPDLOG)
  message("Using fmt, spdlog from: SHARED LIB")
  pkg_check_modules(FMT fmt>=6.0 REQUIRED)
  pkg_check_modules(SPDLOG spdlog>=1.5 REQUIRED)
else()
  message("Using fmt, spdlog from: BUNDLED SRC")
  add_subdirectory(src/spdlog)
endif()
if(USE_VTE)
  pkg_check_modules(VTE vte-2.91 REQUIRED)
endif()

add_subdirectory(src/7za)

include(FindPackageHandleStandardArgs)

# For both src and tests targets
include_directories(
  SYSTEM ${GTKMM_INCLUDE_DIRS}
  SYSTEM ${GTKSVMM_INCLUDE_DIRS}
  SYSTEM ${GLIBMM_INCLUDE_DIRS}
  SYSTEM ${PANGOMM_INCLUDE_DIRS}
  SYSTEM ${GSPELL_INCLUDE_DIRS}
  SYSTEM ${LIBXML_INCLUDE_DIRS}
  SYSTEM ${SQLITE_INCLUDE_DIRS}
  SYSTEM ${CURL_INCLUDE_DIRS}
  SYSTEM ${UCHARDET_INCLUDE_DIRS}
  SYSTEM ${FRIBIDI_INCLUDE_DIRS}
  . # config.h
)
if(USE_SHARED_FMT_SPDLOG)
  include_directories(
    SYSTEM ${FMT_INCLUDE_DIRS}
    SYSTEM ${SPDLOG_INCLUDE_DIRS}
  )
else()
  include_directories(
    src/spdlog/include
  )
endif()
if(USE_VTE)
  include_directories(
    SYSTEM ${VTE_INCLUDE_DIRS}
  )
endif()

add_subdirectory(src/ct)

if(BUILD_TESTING AND NOT DEFINED ENV{DEB_BUILD_OPTIONS})
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
endif()

# rebuild man pages
set(MANFILE_FULL "${CMAKE_SOURCE_DIR}/data/cherrytree.1")
set(MANFILE_FULL_GZ "${MANFILE_FULL}.gz")
add_custom_command(
  OUTPUT ${MANFILE_FULL_GZ}
  COMMAND gzip -f -k --best -n "${MANFILE_FULL}"
)
add_custom_target(man_pages ALL DEPENDS ${MANFILE_FULL_GZ})

# rebuild languages binaries
if(USE_NLS)
  execute_process(
    COMMAND ${PYTHON_EXEC} ${CMAKE_SOURCE_DIR}/po/i18n_po_to_mo.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
endif()

# rebuild metainfo xml
execute_process(
  COMMAND ${PYTHON_EXEC} ${CMAKE_SOURCE_DIR}/scripts/metainfo_xml_write.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# set icon on macos
if(APPLE)
  install(CODE "execute_process(COMMAND ${PYTHON_EXEC} ${CMAKE_SOURCE_DIR}/scripts/set_icon_macos.py ${CMAKE_SOURCE_DIR}/icons/cherrytree.ico ${CMAKE_INSTALL_PREFIX}/bin/cherrytree)")
endif(APPLE)

# install other files
if(${CMAKE_SYSTEM_NAME} MATCHES Linux|.*BSD|DragonFly|Darwin)
  if(NOT APPLE)
    # share data
    install(FILES "${CMAKE_SOURCE_DIR}/data/cherrytree.desktop"
            DESTINATION "${SHARE_INSTALL}/applications")
    install(FILES "${CMAKE_SOURCE_DIR}/icons/cherrytree.svg"
            DESTINATION "${SHARE_INSTALL}/icons/hicolor/scalable/apps")
    install(FILES "${MANFILE_FULL_GZ}"
            DESTINATION "${SHARE_INSTALL}/man/man1")
    install(FILES
            "${CMAKE_SOURCE_DIR}/data/cherrytree.mime"
            "${CMAKE_SOURCE_DIR}/data/cherrytree.keys"
            DESTINATION "${SHARE_INSTALL}/mime-info")
    install(FILES "${CMAKE_SOURCE_DIR}/data/net.giuspen.cherrytree.metainfo.xml"
            DESTINATION "${SHARE_INSTALL}/metainfo")
  endif(NOT APPLE)

  # share/cherrytree language-specs
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/language-specs
          DESTINATION ${CHERRYTREE_SHARE_INSTALL})
  # share/cherrytree styles
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/styles
          DESTINATION ${CHERRYTREE_SHARE_INSTALL})
  # share/cherrytree data
  install(FILES
          "${CMAKE_SOURCE_DIR}/data/script3.js"
          "${CMAKE_SOURCE_DIR}/data/styles4.css"
          "${CMAKE_SOURCE_DIR}/data/user-style.xml"
          DESTINATION "${CHERRYTREE_SHARE_INSTALL}/data")
  # share/cherrytree icons
  install(FILES
          "${CMAKE_SOURCE_DIR}/icons/ct_home.svg"
          DESTINATION "${CHERRYTREE_SHARE_INSTALL}/icons")
  install(DIRECTORY "${CMAKE_SOURCE_DIR}/icons/Breeze_Dark_icons"
          DESTINATION "${CHERRYTREE_SHARE_INSTALL}/icons")
  install(DIRECTORY "${CMAKE_SOURCE_DIR}/icons/Breeze_Light_icons"
          DESTINATION "${CHERRYTREE_SHARE_INSTALL}/icons")

  # locale data
  if(USE_NLS)
    file(GLOB children "${CMAKE_SOURCE_DIR}/po/*")
    foreach(child ${children})
      if(IS_DIRECTORY ${child})
        if(NOT DEFINED ENV{FLATPAK_BUILDER_BUILDDIR})
          install(DIRECTORY ${child} DESTINATION "${PACKAGE_LOCALE_DIR}")
        else()
          install(DIRECTORY ${child} DESTINATION "${CHERRYTREE_SHARE_INSTALL}/locale")
        endif()
      endif()
    endforeach()
  endif()
endif()

# uninstall target
# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
