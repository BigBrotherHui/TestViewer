cmake_minimum_required(VERSION 2.8.12)

project(MedicalImageTool)

find_package(ITK REQUIRED)
find_package(VTK REQUIRED)
find_package(CTK REQUIRED)

find_package(Qt5 REQUIRED Core Widgets OpenGL Xml UiTools XmlPatterns Concurrent Script)


# Set Qt file directory of source code
set(Qt_HDR_files
	About.h
	Crosshair.h
	DistanceMeasurement.h
	VolumeRenderer.h
	ImageToSTL.h
	STLSet.h
	MedicalImageTool.h)
	
set(Qt_SRC_files
	main.cpp
	About.cpp
	Crosshair.cpp
	DistanceMeasurement.cpp
	VolumeRenderer.cpp
	ImageToSTL.cpp
	STLSet.cpp
	MedicalImageTool.cpp)

set(Qt_RES_files
	About.qrc
	Crosshair.qrc
	DistanceMeasurement.qrc
	VolumeRenderer.qrc
	ImageToSTL.qrc
	STLSet.qrc
	MedicalImageTool.qrc)
	
set(Qt_UI_files
	About.ui
	Crosshair.ui
	DistanceMeasurement.ui
	VolumeRenderer.ui
	ImageToSTL.ui
	STLSet.ui
	MedicalImageTool.ui)

qt5_wrap_ui(MOC_UI ${Qt_UI_files})
qt5_wrap_cpp(MOC_SRC ${Qt_HDR_files})
qt5_add_resources(MOC_RES ${Qt_RES_files})	

source_group("Form Files" FILES ${Qt_UI_files})
source_group("Generated Files" FILES ${MOC_UI} ${MOC_SRC})
source_group("Resources Files" FILES ${Qt_RES_files} ${MOC_RES})


# Add the include directories for VTK and Qt 4 Widgets module to the compile lines.
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CTK_INCLUDE_DIRS})
				   
add_executable(${PROJECT_NAME} 
			   ${Qt_HDR_files} ${Qt_SRC_files} ${Qt_UI_files} ${Qt_QRC_files}
			   ${MOC_UI} ${MOC_SRC} ${MOC_RES} 
			   )
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL Qt5::Xml Qt5::UiTools Qt5::XmlPatterns Qt5::Concurrent Qt5::Script ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${CTK_LIBRARIES})		   

			   
				   
