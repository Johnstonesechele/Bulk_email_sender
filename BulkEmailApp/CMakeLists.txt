cmake_minimum_required(VERSION 3.16)
project(BulkEmailApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt libraries
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
find_package(CURL REQUIRED)

add_executable(BulkEmailApp
    src/main.cpp
    src/MainWindow.cpp
    src/EmailSender.cpp
    src/EmailCampaign.cpp
)

target_include_directories(BulkEmailApp PRIVATE src)

target_link_libraries(BulkEmailApp PRIVATE Qt6::Widgets Qt6::Sql CURL::libcurl)

# Enable static build if desired (optional)
# set_target_properties(BulkEmailApp PROPERTIES Qt6_POSITION_INDEPENDENT_CODE ON)