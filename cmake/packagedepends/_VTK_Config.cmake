find_package(VTK COMPONENTS ${VTK_REQUIRED_COMPONENTS_BY_MODULE} REQUIRED)

list(LENGTH VTK_REQUIRED_COMPONENTS_BY_MODULE listlength)
if(${listlength} EQUAL 0)
  list(APPEND ALL_LIBRARIES ${VTK_LIBRARIES})
else()
  foreach(vtk_module ${VTK_REQUIRED_COMPONENTS_BY_MODULE})
    list(APPEND ALL_LIBRARIES "VTK::${vtk_module}")
  endforeach()
endif()

if(ALL_LIBRARIES)
  vtk_module_autoinit(TARGETS ${MODULE_NAME} MODULES ${ALL_LIBRARIES})
endif()
