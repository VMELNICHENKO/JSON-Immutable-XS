CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(JSON-Immutable-XS VERSION 1.1.5 LANGUAGES CXX)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

# add_subdirectory(../../meia/plus ../../var/cmake)

file(GLOB allHeader RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src")
file(GLOB externalHeader RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "external")
file(GLOB_RECURSE allSource RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${allSource})
target_link_libraries(${PROJECT_NAME} plus)
target_include_directories(${PROJECT_NAME} PUBLIC ${externalHeader} ${allHeader} "/usr/local/perl5.34.0/lib/5.34.0/x86_64-linux/CORE/" "../meia/plus/XS-Install")
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)