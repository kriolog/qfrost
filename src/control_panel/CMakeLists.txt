set(qfrost_control_panel_SRCS
    checkboxwithundo.cpp
	dateeditwithundo.cpp
	computationcontrol.cpp
	controlpanel.cpp
	startingconditions.cpp)

add_library(qfrost_control_panel STATIC ${qfrost_control_panel_SRCS})
qt5_use_modules(qfrost_control_panel Widgets)
target_link_libraries(qfrost_control_panel)
