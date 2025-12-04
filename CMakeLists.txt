cmake_minimum_required(VERSION 3.16)
project(BulkEmailApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

# Set up Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/emailmanager.cpp
    src/emailcampaign.cpp
    src/database.cpp
    src/emailvalidator.cpp
    src/emailtemplate.cpp
    src/templatemanager.cpp
    src/templateeditor.cpp
    src/htmlemailbuilder.cpp
)

set(HEADERS
    src/mainwindow.h
    src/emailmanager.h
    src/emailcampaign.h
    src/database.h
    src/emailvalidator.h
    src/emailtemplate.h
    src/templatemanager.h
    src/templateeditor.h
    src/htmlemailbuilder.h
)

set(UI_FILES
    src/mainwindow.ui
)

# Create executable
add_executable(BulkEmailApp ${SOURCES} ${HEADERS} ${UI_FILES})

# Link Qt6 libraries
target_link_libraries(BulkEmailApp 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network 
    Qt6::Sql
)

# Include directories
target_include_directories(BulkEmailApp PRIVATE src)

# Windows-specific settings
if(WIN32)
    set_target_properties(BulkEmailApp PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()