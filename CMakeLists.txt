cmake_minimum_required(VERSION 3.22)

set(PROJECTION_NAME DicomViewer)

project(${PROJECTION_NAME})
include(GenerateExportHeader)
set(CMAKE_CXX_STANDARD 17)
set(MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME 1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(MODULES_PACKAGE_DEPENDS_DIRS ${CMAKE_SOURCE_DIR}/cmake/packagedepends)
set_property( GLOBAL PROPERTY USE_FOLDERS ON)

option(DicomViewer_BUILD_Examples "Enable Build Examples" OFF)
mark_as_advanced(DicomViewer_BUILD_Examples)
option(DicomViewer_BUILD_ "Enable Build _" OFF)
mark_as_advanced(DicomViewer_BUILD_)

add_subdirectory(src)



