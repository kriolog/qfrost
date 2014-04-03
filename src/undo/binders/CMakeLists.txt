set(qfrost_undo_binders_SRCS
    undobinderqcombobox.cpp
)

add_library(qfrost_undo_binders STATIC ${qfrost_undo_binders_SRCS})
qt5_use_modules(qfrost_undo_binders Widgets)
target_link_libraries(qfrost_undo_binders)
