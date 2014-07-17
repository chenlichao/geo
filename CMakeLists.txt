cmake_minimum_required(VERSION 2.8)

project( GEO )
set (GEO_VERSION_MAJOR 0)
set (GEO_VERSION_MINOR 3)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "cmake")

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(APPLE)
	set(CMAKE_CXX_FLAGS "-stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif(APPLE)



SET(Boost_NO_BOOST_CMAKE ON)

FIND_PACKAGE( Boost COMPONENTS program_options system REQUIRED)


#FILE (GLOB SRCS "*.cpp")
##FILE (GLOB_RECURSE SRCS "*.cpp")

add_executable(allpair "allpair.cpp" )
add_executable(nbyn "nbyn.cpp" )
add_executable(contour "contour.cpp" )

include_directories(${GEO_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

target_link_libraries( allpair  ${Boost_LIBRARIES})
target_link_libraries( nbyn ${Boost_LIBRARIES})
target_link_libraries( contour ${Boost_LIBRARIES})

get_cmake_property(_variableNames VARIABLES)


foreach (_variableName ${_variableNames})
	message(STATUS "${_variableName}=${${_variableName}}")
endforeach()


