cmake_minimum_required(VERSION 3.28)
project(DataStorm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the build type if not set by the user
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_PREFIX_PATH /opt/homebrew)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_PREFIX_PATH /usr/local)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_PREFIX_PATH C:/Qt/6.2.2/msvc2019_64)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Release mode")

    # Compiler flags for Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto -ffast-math -funroll-loops -fno-rtti")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto -ffast-math -funroll-loops -fno-rtti")

    # Linker flags for Release mode
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto")

    # General Linker flags for all builds in Release mode
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -O3 -DNDEBUG -march=native -flto")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -O3 -DNDEBUG -march=native -flto")

    # treat warnings as errors in Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto -Werror")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -flto -Werror")

    # enable LTO in Release mode
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE) # For optimized release builds
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE) # For release builds with debug info
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE) # For minimal size release builds

    # Set static linking flags only on non-macOS systems
    if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(BUILD_SHARED_LIBS OFF)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -static")
        set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -static")
        set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} -static")
    endif ()

else ()
    message("Debug mode")

    # Compiler flags for Debug mode with warnings
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -Wall -Wextra -Wpedantic")
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

# Ressources
target_sources(DataStorm PRIVATE
        assets/app.icns
        assets/datastorm.png
)

# Copy assets to build directory
add_custom_command(TARGET DataStorm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:DataStorm>/assets
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX_BUNDLE_ICON_FILE app.icns)
    set(myApp_ICON assets/app.icns)
    set_source_files_properties(${myApp_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")

endif ()

# Windows specific settings
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set(WINDOWS_ICON assets/app.ico)
    file(WRITE ${CMAKE_BINARY_DIR}/datastorm.rc "IDI_ICON1 ICON \"${WINDOWS_ICON}\"\n")
    set_source_files_properties(${CMAKE_BINARY_DIR}/datastorm.rc PROPERTIES LANGUAGE RC)
    target_sources(DataStorm PRIVATE ${CMAKE_BINARY_DIR}/datastorm.rc)
endif ()


# Set the icon for macOS bundle and additional macOS properties
set_target_properties(DataStorm PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE app.icns  # Specify the icon for macOS
        MACOSX_FRAMEWORK_IDENTIFIER com.datastorm
)