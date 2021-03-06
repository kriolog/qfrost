#add_definitions ( -Wall -Wextra -pedantic -Woverloaded-virtual
#                  -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast
#                  -Winit-self -Wunreachable-code -Wswitch-enum
#)

find_package(Boost 1.47.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(core)

include_directories(${QTSINGLEAPPLICATION_INCLUDE_DIRS})
include_directories(${HIERARCHICALHEADERVIEW_INCLUDE_DIRS})
include_directories(${QCUSTOMPLOT_INCLUDE_DIRS})

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

include(qfrost_version)

add_subdirectory(graphicsviews)
add_subdirectory(control_panel)
add_subdirectory(itemviews)
add_subdirectory(soils)
add_subdirectory(boundary_conditions)
add_subdirectory(tools)
add_subdirectory(tools_panel)
add_subdirectory(undo)
add_subdirectory(units)
add_subdirectory(computations)
add_subdirectory(plot)

set(qfrost_SRCS
    about.cpp
    application.cpp
    backgrounddialog.cpp
    blockscountlabel.cpp
    boundarypolygoncalc.cpp
    correctedstyle.cpp
    dialog.cpp
    main.cpp
    mainwindow.cpp
    pointonboundarypolygon.cpp
    positionlabel.cpp
    qfrost.cpp
    recentfilesmenu.cpp
    smartdoublespinbox.cpp
    toolbar.cpp
    welcomedialog.cpp
)

if(WIN32)
	set(qfrost_SRCS ${qfrost_SRCS} correctedstyle.cpp)
endif()

set(QFROST_LIBRARIES
	qfrost_core
	qfrost_graphicsviews
	qfrost_control_panel
	qfrost_computations
	qfrost_soils
	qfrost_itemviews
	qfrost_boundary_conditions
	qfrost_tools
	qfrost_plot
	qfrost_tools_panel
	qfrost_undo
	qfrost_units
	${QTSINGLEAPPLICATION_LIBRARIES}
	${HIERARCHICALHEADERVIEW_LIBRARIES}
        ${QCUSTOMPLOT_LIBRARIES}
)

if(CMAKE_BUILD_TYPE MATCHES Release)
	add_definitions(-DQT_NO_DEBUG_OUTPUT)
	if(WIN32)
		set(GUI_TYPE WIN32)
	elseif(APPLE)
		set(GUI_TYPE MACOSX_BUNDLE)
	endif()
endif()

set(L10N_RES ${CMAKE_CURRENT_BINARY_DIR}/l10n.qrc)
configure_file(../l10n/l10n.qrc ${L10N_RES} COPYONLY)


set(EMBED_QT_L10N_DEFAULT OFF)
if(WIN32)
	set(EMBED_QT_L10N_DEFAULT ON)
endif()
option(EMBED_QT_L10N "Embed needed Qt l10n files to executable" ${EMBED_QT_L10N_DEFAULT})
if(EMBED_QT_L10N)
	set(QT_L10N_RES ${CMAKE_CURRENT_BINARY_DIR}/qt_l10n.qrc)
	configure_file(../l10n/qt_l10n.qrc ${QT_L10N_RES} COPYONLY)
	set(L10N_RES ${L10N_RES} ${QT_L10N_RES})
	get_target_property(_qmake Qt5::qmake LOCATION)
	execute_process( COMMAND ${_qmake} -query QT_INSTALL_TRANSLATIONS OUTPUT_VARIABLE QT_TRANSLATIONS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE )
	STRING(REGEX REPLACE "\\\\" "/" QT_TRANSLATIONS_DIR ${QT_TRANSLATIONS_DIR}) 
	configure_file(${QT_TRANSLATIONS_DIR}/qtbase_ru.qm
		${CMAKE_CURRENT_BINARY_DIR}
		COPYONLY)
endif()

configure_file(main.h.in
	${CONFIGURE_HEADER}
	${CMAKE_CURRENT_BINARY_DIR}/main.h)

set(TRANSLATIONS ../l10n/qfrost_ru.ts)

qt5_add_translation(QM_FILES ${TRANSLATIONS})
qt5_add_resources(QRC_SOURCES
	../res/resources.qrc
	../res/icons.qrc
	${L10N_RES})

set(qfrost_target qfrost)
if(WIN32)
	set(qfrost_target QFrost)
endif()

add_version_info(${qfrost_target} qfrost_SRCS)
add_executable(${qfrost_target}
	${GUI_TYPE}
	${qfrost_SRCS}
	${QRC_SOURCES}
	${QM_FILES})

set_target_properties(${qfrost_target}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

qt5_use_modules(${qfrost_target} Widgets)
target_link_libraries(${qfrost_target} ${QFROST_LIBRARIES})
