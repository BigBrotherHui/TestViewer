cmake_minimum_required(VERSION 3.22)
project(Algorithm LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Core REQUIRED)
find_package(VTK REQUIRED)
find_package(Eigen3 REQUIRED)
tidyFolder()


add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

if(NOT MODULE_EXPORT_DEFINE)
    set(MODULE_EXPORT_DEFINE ${PROJECT_NAME}_EXPORT)
endif()
set(_export_macro_name )
    set(_export_macro_names
    EXPORT_MACRO_NAME ${MODULE_EXPORT_DEFINE}
    NO_EXPORT_MACRO_NAME ${PROJECT_NAME}_NO_EXPORT
    DEPRECATED_MACRO_NAME ${PROJECT_NAME}_DEPRECATED
    NO_DEPRECATED_MACRO_NAME ${PROJECT_NAME}_NO_DEPRECATED
    )
generate_export_header(${PROJECT_NAME}
    ${_export_macro_names}
    EXPORT_FILE_NAME ${PROJECT_NAME}Exports.h
)
#设置后期生成事件
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(target_directory "Examples")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cmd /c "${CMAKE_BINARY_DIR}/postEvent.bat $(SolutionDir) $(SolutionName) $(ProjectDir) $(ProjectName) $(Platform) $(Configuration) $(TargetExt) ${target_directory}"
    )
    set(target_directory "Applications")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cmd /c "${CMAKE_BINARY_DIR}/postEvent.bat $(SolutionDir) $(SolutionName) $(ProjectDir) $(ProjectName) $(Platform) $(Configuration) $(TargetExt) ${target_directory}"
    )
else()
    # 其他系统的操作
    # ...
endif()


#设置输出目录（属性-》常规-》输出目录）（不需要改动）
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
    ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EIGEN3_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core ${VTK_LIBRARIES} Eigen3::Eigen)
