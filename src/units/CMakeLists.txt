set(qfrost_units_SRCS
    physicalpropertydelegate.cpp
    physicalpropertyspinbox.cpp
    units.cpp
    unitsmenu.cpp
)

add_library(qfrost_units STATIC ${qfrost_units_SRCS})
qt5_use_modules(qfrost_units Widgets)
target_link_libraries(qfrost_units)
