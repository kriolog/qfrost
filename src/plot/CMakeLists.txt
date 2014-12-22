set(qfrost_plot_SRCS
	curveplotdialog.cpp
)

add_library(qfrost_plot
	STATIC
	${qfrost_plot_SRCS})
qt5_use_modules(qfrost_plot Widgets)
target_link_libraries(qfrost_plot)
