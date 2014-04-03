add_subdirectory(itemviews)
add_subdirectory(binders)

set(qfrost_undo_SRCS
	nextcheckstatecommand.cpp
	setdatecommand.cpp
	readfromcomputationdatacommand.cpp
	setboundaryconditionscommand.cpp
	changeboundarypolygonscommand.cpp
	blockwitholdparameters.cpp
	setblockssoilcommand.cpp
	setblockstemperaturecommand.cpp
	setblocksthawedpartcommand.cpp
	removeblockscommand.cpp
	addblockscommand.cpp
)

add_library(qfrost_undo STATIC ${qfrost_undo_SRCS})
qt5_use_modules(qfrost_undo Widgets)
target_link_libraries(qfrost_undo qfrost_undo_itemviews qfrost_undo_binders)
