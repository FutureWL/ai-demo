cmake_minimum_required(VERSION 3.20)
project(CppServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Boost root if not found automatically
if(NOT DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT "C:/local/boost_1_83_0")
endif()

find_package(Boost REQUIRED COMPONENTS system)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "Boost found: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
else()
    message(FATAL_ERROR "Boost not found. Please install Boost library.")
endif()

add_executable(cpp_server main.cpp)

target_link_libraries(cpp_server PRIVATE Boost::system)

# Add compile definitions for Windows
if(WIN32)
    target_compile_definitions(cpp_server PRIVATE BOOST_ALL_NO_LIB _WIN32_WINNT=0x0601)
endif()
