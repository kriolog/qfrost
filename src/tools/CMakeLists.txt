set(qfrost_tools_SRCS
	polyline.cpp
	polygonalselection.cpp
	boundaryconditionsapplicator.cpp
	boundaryellipsecreator.cpp
	growingpolygon.cpp
	boundarypolygoncreator.cpp
	blockcreator.cpp
	anchor.cpp
	lug.cpp
	tool.cpp
	rectangulartool.cpp
	rectangularselection.cpp)

add_library(qfrost_tools STATIC ${qfrost_tools_SRCS})
qt5_use_modules(qfrost_tools Widgets)
target_link_libraries(qfrost_tools)
