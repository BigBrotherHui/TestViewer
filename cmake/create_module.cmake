function(create_module)

    set(_macro_params
      EXPORT_DEFINE          # export macro name for public symbols of this module (DEPRECATED)
      DESCRIPTION            # a description for this module
     )

    set(_macro_multiparams
      DEPENDS                # list of modules this module depends on: [PUBLIC|PRIVATE|INTERFACE] <list>
      PACKAGE_DEPENDS        # list of "packages this module depends on (e.g. Qt, VTK, etc.): [PUBLIC|PRIVATE|INTERFACE] <package-list>
      TARGET_DEPENDS         # list of CMake targets this module should depend on: [PUBLIC|PRIVATE|INTERFACE] <list>
      ADDITIONAL_LIBS        # list of addidtional private libraries linked to this module.
     )

    set(_macro_options
     )

    cmake_parse_arguments(MODULE "${_macro_options}" "${_macro_params}" "${_macro_multiparams}" ${ARGN})

    set(MODULE_NAME ${MODULE_UNPARSED_ARGUMENTS})

    if(NOT MODULE_NAME)
        if(MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME)
            get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        else()
            message(SEND_ERROR "The module name must not be empty")
        endif()
    endif()

    set(MODULE_TARGET ${MODULE_NAME})

    parse_package_args(${MODULE_PACKAGE_DEPENDS})
    
    check_module_dependencies(MODULES ${MODULE_DEPENDS}
                                   PACKAGES ${PACKAGE_NAMES}
                                   MISSING_DEPENDENCIES_VAR _MISSING_DEP
                                   PACKAGE_DEPENDENCIES_VAR PACKAGE_NAMES)
    
    if(NOT MODULE_EXPORT_DEFINE)
        set(MODULE_EXPORT_DEFINE ${MODULE_NAME}_EXPORT)
    endif()

    tidyFolder()

    add_library(${MODULE_TARGET} SHARED ${PROJECT_SOURCES})
    set_target_properties(${MODULE_TARGET} PROPERTIES FOLDER "Modules")
    set_target_properties(${MODULE_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
    ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
    )

    if(MODULE_ADDITIONAL_LIBS)
        target_link_libraries(${MODULE_TARGET} PRIVATE ${MODULE_ADDITIONAL_LIBS})
    endif()

    set(_export_macro_name )
        set(_export_macro_names
        EXPORT_MACRO_NAME ${MODULE_EXPORT_DEFINE}
        NO_EXPORT_MACRO_NAME ${MODULE_NAME}_NO_EXPORT
        DEPRECATED_MACRO_NAME ${MODULE_NAME}_DEPRECATED
        NO_DEPRECATED_MACRO_NAME ${MODULE_NAME}_NO_DEPRECATED
        )
    generate_export_header(${MODULE_NAME}
        ${_export_macro_names}
        EXPORT_FILE_NAME ${MODULE_NAME}Exports.h
    )

    target_include_directories(${MODULE_TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    set(_module_property_type PUBLIC)

    set(DEPENDS "${MODULE_DEPENDS}")
    if(DEPENDS OR MODULE_PACKAGE_DEPENDS)
        use_modules(TARGET ${MODULE_TARGET}
                        MODULES ${DEPENDS}
                        PACKAGES ${MODULE_PACKAGE_DEPENDS}
                        )
    endif()
    target_include_directories(${MODULE_TARGET} ${_module_property_type} .)
    file(GLOB SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach(subdir ${SUBDIRS})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
            target_include_directories(${MODULE_TARGET} ${_module_property_type} ${subdir})
        endif()
    endforeach()

    if(_MISSING_DEP)
        if(MODULE_DESCRIPTION)
            set(MODULE_DESCRIPTION "${MODULE_DESCRIPTION} (missing dependencies: ${_MISSING_DEP})")
        endif()
    else()
        set(MODULE_DESCRIPTION "(missing dependencies: ${_MISSING_DEP})")
    endif()

    set(MODULE_NAME ${MODULE_NAME} PARENT_SCOPE)
    set(MODULE_TARGET ${MODULE_TARGET} PARENT_SCOPE)
endfunction()
