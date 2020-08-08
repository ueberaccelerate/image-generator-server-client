cmake_minimum_required(VERSION 3.10)
project( sendrecieveapp CXX )
# force set release
set(CMAKE_BUILD_TYPE Release)
option( ENABLED_TEST  "The test enabled" OFF)

include(cmake/warnings.cmake)
add_library( cppwarnings INTERFACE)
set_project_warinigs(cppwarnings)
target_compile_features( cppwarnings INTERFACE cxx_std_17 )

add_library(common
	include/resource/config.hpp
)

include(cmake/Dependency.cmake)

add_executable( sender )



add_subdirectory(sendrecieveapp)


target_include_directories( sender   PRIVATE include )
target_include_directories( reciever PRIVATE include )


target_link_libraries( common   PRIVATE cppwarnings propertysdk  )
target_link_libraries( sender   PRIVATE cppwarnings Boost::headers Boost::program_options common )
target_link_libraries( reciever PRIVATE cppwarnings Boost::headers common)


if( UNIX )
target_link_libraries( sender   PRIVATE pthread)
target_link_libraries( reciever PRIVATE pthread)
endif()

if ( ENABLED_TEST )
	add_subdirectory(test)
endif()
