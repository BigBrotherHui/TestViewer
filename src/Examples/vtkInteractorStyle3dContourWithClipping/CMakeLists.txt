
cmake_minimum_required(VERSION 2.8)

PROJECT(vtkInteractorStyle3dContourTest)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(
  MAIN_DIRECTORY
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/CutAlongPolylineFilter
  ${PROJECT_BINARY_DIR}/CutAlongPolylineFilter
)
include_directories(
  ${MAIN_DIRECTORY}
)

set( 
  WP_SRC
  WorldPointPicker.cxx
  AbstractInteractorStyle.cxx
  AbstractInteractorStyle3D.cxx
  vtkInteractorStyle3dContour.cpp
  vtkInteractorStyle3dContour2.cpp
)

set(
  WP_INC
  AbstractInteractorStyle.h
  AbstractInteractorStyle3D.h
  vtkInteractorStyle3dContour.h
  vtkInteractorStyle3dContour2.h
  
)

add_subdirectory(CutAlongPolylineFilter)

add_executable(vtkInteractorStyle3dContourTest 
  MACOSX_BUNDLE
  ${WP_INC}
  ${WP_SRC}
  )
set(target_directory "Examples")

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
if(VTK_LIBRARIES)
  target_link_libraries(vtkInteractorStyle3dContourTest ${VTK_LIBRARIES})
else()
  target_link_libraries(vtkInteractorStyle3dContourTest vtkHybrid vtkWidgets)
endif()

target_link_libraries(
  vtkInteractorStyle3dContourTest
  CutAlongPolyLineFilter
)