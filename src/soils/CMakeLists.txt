set(qfrost_soils_SRCS
	unfrozenwaterwidget.cpp
	sortedpointswidget.cpp
	sortedpointsmodel.cpp
	soileditdialog.cpp
	checkboxdelegate.cpp
	soil.cpp
	soilswidget.cpp
	soilspanel.cpp
	soilsmodel.cpp)

add_library(qfrost_soils STATIC ${qfrost_soils_SRCS})
qt5_use_modules(qfrost_soils Widgets)
target_link_libraries(qfrost_soils)
