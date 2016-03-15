set(qfrost_boundary_conditions_SRCS
	monthstableview.cpp
	monthstablewidget.cpp
	monthstablemodel.cpp
    monthstableexpander.cpp
    monthstablesetter.cpp
    monthstableplot.cpp
	boundarycondition.cpp
	boundaryconditioneditdialog.cpp
	boundaryconditionsmodel.cpp
	boundaryconditionspanel.cpp
	boundaryconditionswidget.cpp
	annualspline.cpp
	yearlyparamswidget.cpp
	yearlyparamstabledialog.cpp
	yearlyparamstablemodel.cpp)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

add_library(qfrost_boundary_conditions
	STATIC
	${qfrost_boundary_conditions_SRCS})
qt5_use_modules(qfrost_boundary_conditions Widgets PrintSupport)
target_link_libraries(qfrost_boundary_conditions)
