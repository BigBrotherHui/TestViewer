cmake_minimum_required(VERSION 3.20)
project(DRRGenerator2 LANGUAGES CXX)


find_program(CCACHE_PROGRAM ccache)
#if(CCACHE_PROGRAM)
#  message(STATUS "Set up ccache ...")
#  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
#endif()
#include(CheckLanguage)
#if(NOT CUDAToolkit_FOUND)
#    include(FindCUDAToolkit)
#endif()
#include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
#check_language(CUDA)
#set(CMAKE_CUDA_STANDARD 17)
#set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Concurrent)
find_package(ITK REQUIRED)
find_package(OpenMP REQUIRED)
find_package(VTK REQUIRED)

file(GLOB_RECURSE SOURCE_FILES *.cu *.cpp *.cxx)
file(GLOB_RECURSE HEADER_FILES *.cuh *.h *.hpp *.hxx)
file(GLOB_RECURSE UI_FILES "*.ui")
file(GLOB_RECURSE QRC_FILES "*.qrc")

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PROJECT_SOURCES
        ${SOURCE_FILES} ${HEADER_FILES} ${UI_FILES} ${QRC_FILES}
)
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})
source_group("Forms" REGULAR_EXPRESSION ".ui$")
source_group("Generated Files" REGULAR_EXPRESSION "ui_.*\\.h$|moc.*\\.cpp$|\\.stamp$|\\.rule$")


ADD_EXECUTABLE(${PROJECT_NAME} ${PROJECT_SOURCES})
#set_property(TARGET ${PROJECT_NAME} PROPERTY CUDA_ARCHITECTURES native)

#cuda_add_executable(${PROJECT_NAME} kernel.cu)
#target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
set(target_directory "Plugins")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cmd /c "${CMAKE_BINARY_DIR}/postEvent.bat $(SolutionDir) $(SolutionName) $(ProjectDir) $(ProjectName) $(Platform) $(Configuration) $(TargetExt) ${target_directory}"
)
else()

endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>/${target_directory}/${PROJECT_NAME}.exe)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>/${target_directory})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${target_directory})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core Qt5::Concurrent ${ITK_LIBRARIES} ${VTK_LIBRARIES})


