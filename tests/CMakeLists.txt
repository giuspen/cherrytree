add_compile_options(-Wall -Wextra -Wunreachable-code -Wuninitialized)

if(USE_SHARED_GTEST_GMOCK)
  message("Using gtest, gmock from: SHARED LIB")
  pkg_check_modules(GTEST gtest REQUIRED)
  pkg_check_modules(GMOCK gmock REQUIRED)
else()
  message("Using gtest, gmock from: BUNDLED SRC")
  add_subdirectory(
    googletest
  )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/src/ct
)

if(USE_SHARED_GTEST_GMOCK)
  include_directories(
    SYSTEM ${GTEST_INCLUDE_DIRS}
    SYSTEM ${GMOCK_INCLUDE_DIRS}
  )
else()
  mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
  )

  set_target_properties(gtest PROPERTIES FOLDER extern)
  set_target_properties(gtest_main PROPERTIES FOLDER extern)
  set_target_properties(gmock PROPERTIES FOLDER extern)
  set_target_properties(gmock_main PROPERTIES FOLDER extern)
endif()

macro(package_add_test TESTNAME)
  # create an executable in which the tests will be stored
  add_executable(${TESTNAME} ${ARGN})

  if(USE_SHARED_GTEST_GMOCK)
    target_link_libraries(${TESTNAME}
      ${GTEST_LIBRARIES}
      ${GMOCK_LIBRARIES}
      cherrytree_shared
    )
  else()
    target_link_libraries(${TESTNAME}
      gtest
      gmock
      gtest_main
      cherrytree_shared
    )
  endif()

  gtest_discover_tests(${TESTNAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )
  set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

# some tests don't work in TRAVIS, so turn them off
if(DEFINED ENV{TRAVIS})
  add_definitions(-D_TRAVIS)
endif()

package_add_test(run_tests_no_x
  tests_main.cpp
  tests_clipboard.cpp
  tests_encoding.cpp
  tests_filesystem.cpp
  tests_misc_utils.cpp
  tests_tmp_n_p7zip.cpp
  tests_types.cpp
  tests_lists.cpp
)

package_add_test(run_tests_with_x_1
  tests_main.cpp
  tests_exports.cpp
  ../src/ct/icons.gresource.cc
)

package_add_test(run_tests_with_x_2
  tests_main.cpp
  tests_read_write.cpp
  tests_3rd_party.cpp
  ../src/ct/icons.gresource.cc
)

if(AUTO_RUN_TESTING)
  add_custom_command(TARGET run_tests_no_x POST_BUILD
    COMMAND ${CMAKE_BINARY_DIR}/run_tests_no_x
  )
  if(NOT DEFINED ENV{FLATPAK_BUILDER_BUILDDIR})
    add_custom_command(TARGET run_tests_with_x_1 POST_BUILD
      COMMAND ${CMAKE_BINARY_DIR}/run_tests_with_x_1
    )
    add_custom_command(TARGET run_tests_with_x_2 POST_BUILD
      COMMAND ${CMAKE_BINARY_DIR}/run_tests_with_x_2
    )
  endif()
endif()

set_target_properties(run_tests_no_x PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(run_tests_with_x_1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(run_tests_with_x_2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
