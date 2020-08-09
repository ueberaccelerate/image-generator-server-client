cmake_minimum_required(VERSION 3.10)
project( image-generator CXX )

add_executable( image-generator )

target_include_directories( image-generator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources( image-generator PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include/Server.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Connection.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Connection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries( image-generator   PRIVATE cppwarnings Boost::headers Boost::program_options common )

if( UNIX )
target_link_libraries( image-generator PRIVATE pthread)
endif()