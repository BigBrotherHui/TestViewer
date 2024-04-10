cmake_minimum_required(VERSION 3.22)
project(testviewer LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Concurrent)
find_package(OpenMP REQUIRED)
find_package(ITK REQUIRED)
find_package(VTK REQUIRED)
find_package(DCMTK REQUIRED)

file(GLOB SOURCE_FILES "*.cpp" "*.cxx" "*.txx")
file(GLOB HEADER_FILES "*.h")
file(GLOB UI_FILES "*.ui")
file(GLOB QRC_FILES "*.qrc")

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

include_directories(${CMAKE_SOURCE_DIR} ${DCMTK_INCLUDE_DIRS})
link_directories(${DCMTK_CMAKE_INSTALL_PREFIX}/${DCMTK_CMAKE_INSTALL_LIBDIR})
file(GLOB SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(subdir ${SUBDIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
    endif()
endforeach()
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

#设置输出目录（属性-》常规-》输出目录）（不需要改动）
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
    ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Applications")

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core Qt5::Concurrent ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${DCMTK_LIBRARIES} Core
)
