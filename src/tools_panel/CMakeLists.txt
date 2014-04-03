set(qfrost_tools_panel_SRCS
	blockcreatorsettings.cpp
	rectangulartoolsettings.cpp
	toolsettings.cpp
	settingsbox.cpp
	rectangulartoolpanel.cpp
	blockcreatorpanel.cpp
	selectionpanel.cpp
	toolspanel.cpp)

add_library(qfrost_tools_panel STATIC ${qfrost_tools_panel_SRCS})
qt5_use_modules(qfrost_tools_panel Widgets)
target_link_libraries(qfrost_tools_panel)
