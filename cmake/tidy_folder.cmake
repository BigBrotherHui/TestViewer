macro(tidyFolder)

    file(GLOB_RECURSE SOURCE_FILES "*.cpp" "*.cxx" "*.txx")
    file(GLOB_RECURSE HEADER_FILES "*.h")
    file(GLOB_RECURSE UI_FILES "*.ui")
    file(GLOB_RECURSE QRC_FILES "*.qrc")

    foreach(_source IN ITEMS ${HEADER_FILES})
        get_filename_component(_source_path "${_source}" PATH)
        if(_source_path STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
            set(_group_path "Uncategorized Files")
        else()
            file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
        endif()
        source_group("${_group_path}/Header" FILES "${_source}")
    endforeach()

    foreach(_source IN ITEMS ${SOURCE_FILES})
        get_filename_component(_source_path "${_source}" PATH)
        if(_source_path STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
            set(_group_path "Uncategorized Files")
        else()
            file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
        endif()
        source_group("${_group_path}/Source" FILES "${_source}")
    endforeach()

    foreach(_source IN ITEMS ${UI_FILES})
        get_filename_component(_source_path "${_source}" PATH)
        if(_source_path STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
            set(_group_path "Uncategorized Files")
        else()
            file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
        endif()
        source_group("${_group_path}/UI Forms" FILES "${_source}")
    endforeach()

    foreach(_source IN ITEMS ${QRC_FILES})
        get_filename_component(_source_path "${_source}" PATH)
        if(_source_path STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
            set(_group_path "Uncategorized Files")
        else()
            file(RELATIVE_PATH _source_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source_path}")
            string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
        endif()
        source_group("${_group_path}/Resource" FILES "${_source}")
    endforeach()

    source_group("Generated Files" REGULAR_EXPRESSION "qrc_.*\\.cpp|ui_.*\\.h$|moc.*\\.cpp$|\\.stamp$|\\.rule$")
    set(PROJECT_SOURCES
        ${SOURCE_FILES} ${HEADER_FILES} ${UI_FILES} ${QRC_FILES}
    )
endmacro()