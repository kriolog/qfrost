set(qfrost_itemviews_SRCS
    itemstabledialog.cpp
    itemswidget.cpp
    item.cpp
    itemeditdialog.cpp
	itemsmodel.cpp
	naturalsortfilterproxymodel.cpp)

add_library(qfrost_itemviews STATIC ${qfrost_itemviews_SRCS})
qt5_use_modules(qfrost_itemviews Widgets)
target_link_libraries(qfrost_itemviews)
