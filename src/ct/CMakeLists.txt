
add_compile_options(-Wall -Wextra -Wunreachable-code -Wuninitialized)

if(USE_NLS)
    add_definitions(-DHAVE_NLS)
endif()

# Generating icons
execute_process(
    COMMAND ${PYTHON_EXEC} ${CMAKE_SOURCE_DIR}/scripts/gresource_to_c.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Generating rc file (with icon) for windows
if(MINGW)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cherrytree_rc.o
                       COMMAND windres.exe -I${CMAKE_SOURCE_DIR}/icons 
                                           -i${CMAKE_SOURCE_DIR}/data/windows.rc
                                           -o ${CMAKE_CURRENT_BINARY_DIR}/cherrytree_rc.o )
    set(CHERRY_EXE_RC ${CMAKE_CURRENT_BINARY_DIR}/cherrytree_rc.o)
endif(MINGW)


set(CT_SHARED_FILES
    ct_actions_edit.cc
    ct_actions_export.cc
    ct_actions_file.cc
    ct_actions_find.cc
    ct_actions_format.cc
    ct_actions_others.cc
    ct_actions_import.cc
    ct_actions_tree.cc
    ct_actions_view.cc
    ct_actions_help.cc
    ct_app.cc
    ct_clipboard.cc
    ct_codebox.cc
    ct_config.cc
    ct_dialogs.cc
    ct_export2html.cc
    ct_export2pdf.cc
    ct_export2txt.cc
    ct_image.cc
    ct_imports.cc
    ct_list.cc
    ct_main_win.cc
    ct_menu.cc
    ct_misc_utils.cc
    ct_p7za_iface.cc
    ct_pref_dlg.cc
    ct_state_machine.cc
    ct_storage_control.cc
    ct_storage_sqlite.cc
    ct_storage_xml.cc
    ct_table.cc
    ct_treestore.cc
    ct_widgets.cc
    ct_parser_text.cc
    ct_parser_md.cc
    ct_parser_html.cc
    ct_parser.cc
    ct_process.cc
    ct_filesystem.cc
    ct_column_edit.cc
)

add_library(cherrytree_shared STATIC ${CT_SHARED_FILES})
target_link_libraries(cherrytree_shared
    ${GTKMM_LIBRARIES}
    ${GTKSVMM_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${PANGOMM_LIBRARIES}
    ${GSPELL_LIBRARIES}
    ${LIBXML_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${CURL_LIBRARIES}
    ${UCHARDET_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    pthread
    7za_static
)

target_include_directories(cherrytree_shared PUBLIC "..")

add_executable(cherrytree ct_main.cc icons.gresource.cc ${CHERRY_EXE_RC})
target_link_libraries(cherrytree cherrytree_shared)

if(APPLE)
    target_link_libraries(cherrytree "-framework Foundation -framework AppKit")
elseif(WIN32)
    target_link_libraries(cherrytree "-mwindows")
endif()

set_target_properties(cherrytree PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# install cherrytree
install(TARGETS cherrytree RUNTIME DESTINATION bin)
