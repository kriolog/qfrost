set(qfrost_undo_itemviews_SRCS
    boundaryconditionsremovecommand.cpp
    soilsremovecommand.cpp
    itemsremovecommand.cpp
    itemaddcommand.cpp
    soileditcommand.cpp
    itemeditcommand.cpp
)

add_library(qfrost_undo_itemviews STATIC ${qfrost_undo_itemviews_SRCS})
qt5_use_modules(qfrost_undo_itemviews Widgets)
target_link_libraries(qfrost_undo_itemviews)
