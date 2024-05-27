find_package(MITK REQUIRED)
create_module(
DEPENDS MitkCore MitkQtWidgets
PACKAGE_DEPENDS
    PRIVATE VTK ITK
)
add_definitions(-DITK_IO_FACTORY_REGISTER_MANAGER)
