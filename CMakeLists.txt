cmake_minimum_required(VERSION 3.5)

project(wsjcpp-logger)

# include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

#### BEGIN_WSJCPP_APPEND
#### END_WSJCPP_APPEND

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${wsjcpp-logger_SOURCE_DIR})

# Sources

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")
list (APPEND WSJCPP_INCLUDE_DIRS "src/wsjcpp-logger")

list (APPEND WSJCPP_SOURCES "src/wsjcpp-logger/wsjcpp_logger.cpp")

list (APPEND WSJCPP_SOURCES "src/main.cpp")

include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable ("wsjcpp-logger" ${WSJCPP_SOURCES})

target_link_libraries("wsjcpp-logger" -lpthread ${WSJCPP_LIBRARIES} )

install(
    TARGETS
        "wsjcpp-logger"
    RUNTIME DESTINATION
        /usr/bin
)


