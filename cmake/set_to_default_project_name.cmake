function(set_to_default_project_name)
    get_filename_component(DEFAULT_PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(PROJ_NAME ${DEFAULT_PROJECT_NAME} CACHE STRING "Name of the project" FORCE)
    message(STATUS "PROJECT_NAME not defined. Using folder name: ${PROJECT_NAME}")
endfunction()