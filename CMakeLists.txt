cmake_minimum_required(VERSION 3.28)
project(DataStorm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Sql
        REQUIRED)

add_executable(DataStorm src/main.cpp
        src/insertwindow.h src/insertwindow.cpp src/insertwindow.ui
        src/Util.h

)
target_link_libraries(DataStorm
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Sql
)

# Mettre un icon à l'application
# MACOS
#if (APPLE)
#    set_target_properties(DataStorm PROPERTIES
#            MACOSX_BUNDLE TRUE
#            MACOSX_BUNDLE_ICON_FILE DataStorm.icns
#    )
#    set_source_files_properties(DataStorm.icns PROPERTIES
#            MACOSX_PACKAGE_LOCATION "Resources"
#    )
#endif (APPLE)
