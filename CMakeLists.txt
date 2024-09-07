cmake_minimum_required(VERSION 3.28)
project(DataStorm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Release mode")

    # Compiler flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto -ffast-math -funroll-loops -fno-rtti")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto -ffast-math -funroll-loops  -fno-rtti")

    # Linker flags
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")

    # General Linker flags for all builds
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -O3 -DNDEBUG -march=native -flto")

else ()
    message("Debug mode")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Sql REQUIRED)

add_executable(DataStorm
        src/main.cpp
        src/insertwindow.h
        src/insertwindow.cpp
        src/insertwindow.ui
        src/datastorm.h
        src/CsvLoader.cpp
        src/CsvLoader.h
        src/DatabaseHelper.cpp
        src/DatabaseHelper.h
)

target_link_libraries(DataStorm
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Sql
)

if (APPLE)
    set_target_properties(DataStorm PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_ICON_FILE "app.icns"
    )
    set_source_files_properties("assets/app.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif ()