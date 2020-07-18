
add_compile_options(-Wall -Wextra -Wunreachable-code -Wuninitialized)

include_directories(
    ${CPPUTEST_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/ct
)

set(CT_TEST_FILES
    tests_main.cpp
    tests_misc_utils.cpp
    tests_tmp_n_p7zip.cpp
    tests_types.cpp
    tests_clipboard.cpp
    tests_filesystem.cpp
    tests_encoding.cpp
    tests_read_write.cpp
)

# some tests doesn't work in TRAVIS, so turn off them
if(DEFINED ENV{TRAVIS})
    add_definitions(-D_TRAVIS)
endif()

add_executable(run_tests ${CT_TEST_FILES})
target_link_libraries(run_tests 
    ${CPPUTEST_LIBRARIES}
    cherrytree_shared
)

add_test(run_tests run_tests)
