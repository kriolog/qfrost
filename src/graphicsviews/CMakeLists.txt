set(qfrost_graphicsviews_SRCS
	arrow.cpp
	block.cpp
	blockportable.cpp
	boundarypolygon.cpp
	boundarypolygonportable.cpp
	cross.cpp
	colorbar.cpp
	colorgenerator.cpp
	nonscalableitem.cpp
	ruler.cpp
	scene.cpp
	view.cpp
	viewbase.cpp
)

add_library(qfrost_graphicsviews
	STATIC
	${qfrost_graphicsviews_SRCS})
qt5_use_modules(qfrost_graphicsviews Widgets)
target_link_libraries(qfrost_graphicsviews qfrost_core)
