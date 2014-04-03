find_package(MathGL 1.11 REQUIRED Qt)

include_directories(${MATHGL_INCLUDE_DIRS})

set(qfrost_plot_SRCS
	areaplotdialog.cpp
	areadraw.cpp
	curvedraw.cpp
	curveplotdialog.cpp
	plotdialog.cpp)

qt4_automoc(${qfrost_plot_SRCS})

add_library(qfrost_plot STATIC ${qfrost_plot_SRCS})
target_link_libraries(qfrost_plot ${MATHGL_LIBRARIES})
