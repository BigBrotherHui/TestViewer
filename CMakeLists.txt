cmake_minimum_required(VERSION 3.22)

set(PROJECTION_NAME DicomViewer)

project(${PROJECTION_NAME})
include(GenerateExportHeader)
#设置平台位数版本
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "Platform for generator" FORCE)
endif()
file(COPY ${CMAKE_SOURCE_DIR}/dependence/script/postEvent.bat DESTINATION ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME 1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(MODULES_PACKAGE_DEPENDS_DIRS ${CMAKE_SOURCE_DIR}/cmake/packagedepends)
set_property( GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)



