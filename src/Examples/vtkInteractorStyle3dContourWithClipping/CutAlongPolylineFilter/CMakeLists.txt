cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0020 NEW)

PROJECT(CutAlongPolylineFilter)
#VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})


#SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(TEST_CUTALONGPOLYLINEFILTER_SRC   ${CMAKE_CURRENT_SOURCE_DIR}/TestCutAlongPolyLineFilter.cxx)
SET(
	CUT_ALONG_POLYLINE_FILTER_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/CutAlongPolyLineFilter.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/CutAlongPolyLineFilter2.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/vtkLoopedTubeFilter.cxx
	)
SET(
	CUT_ALONG_POLYLINE_FILTER_INC
	 ${CMAKE_CURRENT_SOURCE_DIR}/CutAlongPolyLineFilter.h
	 ${CMAKE_CURRENT_SOURCE_DIR}/CutAlongPolyLineFilter2.h
	${CMAKE_CURRENT_SOURCE_DIR}/vtkLoopedTubeFilter.h
	 
	 )

add_library(CutAlongPolyLineFilter 	
	${CUT_ALONG_POLYLINE_FILTER_INC} 
	${CUT_ALONG_POLYLINE_FILTER_SRC}
	)

add_executable(Test_CutAlongPolyLineFilter 
	${CMAKE_CURRENT_SOURCE_DIR}/TestCutAlongPolyLineFilter.cxx
	)
set(target_directory "Examples")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET Test_CutAlongPolyLineFilter POST_BUILD
    COMMAND cmd /c "${CMAKE_BINARY_DIR}/postEvent.bat $(SolutionDir) $(SolutionName) $(ProjectDir) $(ProjectName) $(Platform) $(Configuration) $(TargetExt) ${target_directory}"
)
else()

endif()
set_target_properties(Test_CutAlongPolyLineFilter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
	LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(TARGET Test_CutAlongPolyLineFilter PROPERTY VS_DEBUGGER_COMMAND ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>/${target_directory}/Test_CutAlongPolyLineFilter.exe)
    set_property(TARGET Test_CutAlongPolyLineFilter PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}/$<CONFIGURATION>/${target_directory})
endif()
set_target_properties(Test_CutAlongPolyLineFilter PROPERTIES FOLDER ${target_directory})
target_link_libraries(CutAlongPolyLineFilter ${VTK_LIBRARIES})
target_link_libraries(Test_CutAlongPolyLineFilter  ${VTK_LIBRARIES} CutAlongPolyLineFilter)

#Grouping in MSVC
SOURCE_GROUP("Test"	FILES ${TEST_CUTALONGPOLYLINEFILTER_SRC})
SOURCE_GROUP("Files" 	FILES ${TEST_IO_MANAGER_INC}  ${TEST_IO_MANAGER_SRC} )
