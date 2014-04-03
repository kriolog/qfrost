cmake_minimum_required(VERSION 2.8.8)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(SINGLEAPP-SOURCES
	qtlocalpeer.cpp
	qtlockedfile.cpp
	qtsingleapplication.cpp
	qtsinglecoreapplication.cpp)

set(SINGLEAPP-MOC-HEADERS
	qtlocalpeer.h
	qtsingleapplication.h
	qtsinglecoreapplication.h)

if(WIN32)
	set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_win.cpp)
elseif(WIN32)
	set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_unix.cpp)
endif()

qt5_wrap_cpp(SINGLEAPP-SOURCES-MOC ${SINGLEAPP-MOC-HEADERS})

add_library(qtsingleapplication
	STATIC
	${SINGLEAPP-SOURCES}
	${SINGLEAPP-SOURCES-MOC})
	
qt5_use_modules(qtsingleapplication Widgets Network)
