cmake_minimum_required(VERSION 3.10)
project( sender CXX )

add_executable( sender )

target_include_directories( sender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources( sender PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include/Server.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/Connection.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/TimerThread.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Connection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TimerThread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries( sender   PRIVATE cppwarnings Boost::headers Boost::program_options common )

if( UNIX )
target_link_libraries( sender PRIVATE pthread)
endif()