cmake_minimum_required(VERSION 2.8.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

project(QFrost)
set(ORGANIZATION_NAME "www.qfrost.net")
set(ORGANIZATION_DOMAIN "www.qfrost.net")
set(COPYRIGHT_YEARS "2010-2013")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_subdirectory(3rdparty/qtsingleapplication)
set(QTSINGLEAPPLICATION_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qtsingleapplication)
set(QTSINGLEAPPLICATION_LIBRARIES qtsingleapplication)

add_subdirectory(3rdparty/HierarchicalHeaderView)
set(HIERARCHICALHEADERVIEW_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/HierarchicalHeaderView)
set(HIERARCHICALHEADERVIEW_LIBRARIES HierarchicalHeaderView)

include_directories(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(src)
