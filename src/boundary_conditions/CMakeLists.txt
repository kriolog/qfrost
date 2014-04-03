set(qfrost_boundary_conditions_SRCS
    monthstableview.cpp
	monthstablewidget.cpp
	monthstablemodel.cpp
	boundarycondition.cpp
	boundaryconditioneditdialog.cpp
	boundaryconditionsmodel.cpp
	boundaryconditionspanel.cpp
	boundaryconditionswidget.cpp)

add_library(qfrost_boundary_conditions
	STATIC
	${qfrost_boundary_conditions_SRCS})
qt5_use_modules(qfrost_boundary_conditions Widgets)
target_link_libraries(qfrost_boundary_conditions)
