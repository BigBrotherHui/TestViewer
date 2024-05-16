set_to_default_project_name()
tidyFolder()
project(${PROJ_NAME})
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(MITK 2023.04 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(ITK REQUIRED)
find_package(VTK REQUIRED)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
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
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core ${ITK_LIBRARIES} ${VTK_LIBRARIES}
MitkCore MitkQtWidgets MitkBoundingShape MitkDataTypesExt MitkSegmentation MitkSceneSerialization
)
