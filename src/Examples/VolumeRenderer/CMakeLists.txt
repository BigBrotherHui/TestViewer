cmake_minimum_required(VERSION 2.8.11)

project(VolumeRenderer)

find_package(OpenGL REQUIRED)
find_package(CTK REQUIRED)

# VTK is required
find_package(VTK REQUIRED)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
## Instruct CMake to run moc automatically when needed.
#set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
#find_package(Qt5Widgets)
#find_package(Qt5OpenGL)
#find_package(Qt5Xml)
#find_package(Qt5UiTools)
#find_package(Qt5XmlPatterns)
#find_package(Qt5Concurrent)
#find_package(Qt5Script)
#find_package(Qt5ScriptTools)
find_package(Qt5 REQUIRED Core Widgets OpenGL Xml UiTools XmlPatterns Concurrent Script)
# Use the compile definitions defined in the Qt 5 Widgets module

# Add compiler flags for building executables (-fPIE)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


# Qt and project source code
file(GLOB Project_source_files
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
file(GLOB Project_header_files
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
QT5_WRAP_CPP(Project_moc_source_files ${Project_header_files})
file(GLOB Project_ui_files
  ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)
QT5_WRAP_UI(Project_ui_header_files ${Project_ui_files})
source_group(ui FILES ${Project_ui_files} ${Project_ui_header_files})
source_group(moc FILES ${Project_moc_source_files})
#SET(Project_qrc_files YourProject.qrc) 
#QT5_ADD_RESOURCES(Project_qrc_compiled ${Project_qrc_files})

# Add the include directories for VTK and Qt 5 Widgets module to the compile lines.
#include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CTK_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${Project_source_files} ${Project_ui_header_files}
	${Project_header_files} ${Project_moc_source_files}
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")
# Use the Widgets module from Qt 5.
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL Qt5::Xml Qt5::UiTools Qt5::XmlPatterns Qt5::Concurrent Qt5::Script
${VTK_LIBRARIES} ${OPENGL_LIBRARIES} ${CTK_LIBRARIES})
