cmake_minimum_required(VERSION 3.1.0)

project(cherrytree)
set(CT_VERSION "0.1.0")


set(CPACK_PACKAGE_NAME "cherrytree")

set(CMAKE_CXX_STANDARD 17)

# temporarily disable these warnings: Only <gtksourceview/gtksource.h> can be included directly.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-Wno-cpp)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options("-Wno-#warnings")
  add_compile_options(-Wthread-safety)
endif()

#add_compile_options(-pthread -Wall -Wextra -Wno-unused-parameter)
add_compile_options(-lm  -Wno-deprecated -Wno-deprecated-declarations)
add_definitions(-D_CT_VERSION="${CT_VERSION}")
add_definitions(-DGETTEXT_PACKAGE="${CPACK_PACKAGE_NAME}")
add_definitions(-DCHERRYTREE_LOCALEDIR="")


if(CMAKE_BUILD_TYPE STREQUAL "")
  add_compile_options(-O3)
endif()

#option(BUILD_TESTING "Build tests")

include(FindPkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0 REQUIRED)
pkg_check_modules(GTKSVMM gtksourceviewmm-3.0 REQUIRED)
pkg_check_modules(GLIBMM glibmm-2.4 REQUIRED)
pkg_check_modules(PANGOMM pangomm-1.4 REQUIRED)
pkg_check_modules(GSPELL gspell-1 REQUIRED)
pkg_check_modules(LIBXML libxml++-2.6 REQUIRED)
pkg_check_modules(SQLITE sqlite3 REQUIRED)
pkg_check_modules(CPPUTEST cpputest  REQUIRED)

add_subdirectory(src/7za)


include(FindPackageHandleStandardArgs)

# For both src and tests targets
include_directories(
  ${GTKMM_INCLUDE_DIRS}
  ${GTKSVMM_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${PANGOMM_INCLUDE_DIRS}
  ${GSPELL_INCLUDE_DIRS}
  ${LIBXML_INCLUDE_DIRS}
  ${SQLITE_INCLUDE_DIRS}
  include_directories(.) # to find src/fmt
)

add_subdirectory(src/ct)

enable_testing()
add_subdirectory(tests)

