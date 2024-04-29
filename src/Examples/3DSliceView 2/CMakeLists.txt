cmake_minimum_required(VERSION 2.6)

PROJECT(3DSliceView2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
FIND_PACKAGE(VTK REQUIRED)
FIND_PACKAGE(ITK REQUIRED)
find_package(Qt5 REQUIRED Core Widgets)
tidyFolder()
ADD_EXECUTABLE(${PROJECT_NAME} ${PROJECT_SOURCES}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ${ITK_LIBRARIES} Qt5::Widgets Qt5::Core Algorithm Qt5::Widgets
${VTK_LIBRARIES} ${OPENGL_LIBRARIES})

set(target_directory "Examples")
#设置后期生成事件
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cmd /c "${CMAKE_BINARY_DIR}/postEvent.bat $(SolutionDir) $(SolutionName) $(ProjectDir) $(ProjectName) $(Platform) $(Configuration) $(TargetExt) ${target_directory}"
)
else()
    # 其他系统的操作
    # ...
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
)
# 生成 Visual Studio 项目文件时，设置调试路径
#if (CMAKE_GENERATOR MATCHES "Visual Studio")
#    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>/${target_directory}/${PROJECT_NAME}.exe)
#    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>/${target_directory})
#endif()
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${target_directory})

