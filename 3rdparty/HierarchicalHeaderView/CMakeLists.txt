cmake_minimum_required(VERSION 2.8.8)

find_package(Qt5Widgets REQUIRED)

set(HIERARCHICALHEADERVIEW-SOURCES
	HierarchicalHeaderView.cpp)

set(HIERARCHICALHEADERVIEW-MOC-HEADERS
	HierarchicalHeaderView.h)

qt5_wrap_cpp(HIERARCHICALHEADERVIEW-SOURCES-MOC ${HIERARCHICALHEADERVIEW-MOC-HEADERS})

add_library(HierarchicalHeaderView
	STATIC
	${HIERARCHICALHEADERVIEW-SOURCES}
	${HIERARCHICALHEADERVIEW-SOURCES-MOC})

qt5_use_modules(HierarchicalHeaderView Widgets)
