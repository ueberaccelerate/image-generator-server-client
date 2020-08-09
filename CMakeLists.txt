cmake_minimum_required(VERSION 3.10)
project( sendrecieveapp CXX )
# force set release
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

option( SEND_RECIVE_APP_ENABLED_TEST  "The test enabled" OFF)

include(cmake/warnings.cmake)
include(cmake/Dependency.cmake)

add_library( cppwarnings INTERFACE)
add_library(common	
	include/resource/config.hpp
	include/async/TimerThread.h
)

set_project_warinigs(cppwarnings)
target_compile_features( cppwarnings INTERFACE cxx_std_17 )

target_include_directories( common PUBLIC include )
target_link_libraries( common   PRIVATE cppwarnings propertysdk  )

add_subdirectory(src)

if ( SEND_RECIVE_APP_ENABLED_TEST )
	add_subdirectory(test)
endif()
