set(qfrost_plot_SRCS
    curveplot.cpp
	curveplotdialog.cpp
)

add_library(qfrost_plot
	STATIC
	${qfrost_plot_SRCS})
qt5_use_modules(qfrost_plot Widgets PrintSupport)
target_link_libraries(qfrost_plot)
