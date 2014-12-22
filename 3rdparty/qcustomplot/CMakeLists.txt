cmake_minimum_required(VERSION 2.8.8)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

set(qcustomplot_SRCS qcustomplot.cpp)

add_library(qcustomplot STATIC ${qcustomplot_SRCS})

qt5_use_modules(qcustomplot Widgets PrintSupport)
