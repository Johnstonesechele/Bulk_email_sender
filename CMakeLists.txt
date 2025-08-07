cmake_minimum_required(VERSION 3.16)
project(BulkEmailApp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Qt6 setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/EmailManager.cpp
    src/CampaignManager.cpp
    src/SMTPClient.cpp
    src/DatabaseManager.cpp
)

set(HEADERS
    src/MainWindow.h
    src/EmailManager.h
    src/CampaignManager.h
    src/SMTPClient.h
    src/DatabaseManager.h
)

# UI files
set(UI_FILES
    ui/MainWindow.ui
    ui/EmailComposer.ui
    ui/CampaignHistory.ui
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network
)

# Windows specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Add Windows icon
    target_sources(${PROJECT_NAME} PRIVATE resources/app.rc)
endif()

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)