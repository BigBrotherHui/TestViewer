project(DRRGenerator)
cmake_minimum_required(VERSION 2.8)
ADD_DEFINITIONS(
    -std=c++11
    -O3
)
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
find_package(OpenMP)
find_package(Eigen3 REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
find_package( OpenCV REQUIRED )
add_executable(${PROJECT_NAME} main.cpp DRRgenerator.cpp )
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS} )
target_include_directories(${PROJECT_NAME} PRIVATE ${EIGEN3_INCLUDE_DIR})
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
target_link_libraries( ${PROJECT_NAME}   ${OpenCV_LIBS}  Eigen3::Eigen)


