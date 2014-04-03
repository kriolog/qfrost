#find_package(TBB REQUIRED)

#include_directories(
#    ${TBB_INCLUDE_DIRS}
#)

#link_directories(
#    ${TBB_LIBRARY_DIRS}
#)

set(qfrost_core_SRCS
	boundarycondition.cpp
	domain.cpp
	heatsurface.cpp
	soilblock.cpp)

add_library(qfrost_core STATIC ${qfrost_core_SRCS})
target_link_libraries(qfrost_core) #${TBB_LIBRARIES}
