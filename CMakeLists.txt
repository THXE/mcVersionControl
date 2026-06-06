cmake_minimum_required(VERSION 3.16)
project(MCVersionControl VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 path
list(APPEND CMAKE_PREFIX_PATH "D:/qt/6.11.1/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

set(SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/UI/TitleBar.h
    src/UI/TitleBar.cpp
    src/UI/ProjectPanel.h
    src/UI/ProjectPanel.cpp
    src/UI/ProjectCard.h
    src/UI/ProjectCard.cpp
    src/UI/VersionPanel.h
    src/UI/VersionPanel.cpp
    src/Core/ProjectManager.h
    src/Core/ProjectManager.cpp
    src/Core/VersionManager.h
    src/Core/VersionManager.cpp
    src/Utils/JsonHelper.h
    src/Utils/JsonHelper.cpp
    src/Utils/FileScanner.h
    src/Utils/FileScanner.cpp
    src/Utils/ScrollBarHoverFilter.h
    src/Utils/ScrollBarHoverFilter.cpp
    src/Utils/ThemeManager.h
    src/Utils/ThemeManager.cpp
)

set(RESOURCES
    res/resources.qrc
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/UI
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/Utils
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    dwmapi
)

# Copy required DLLs
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
