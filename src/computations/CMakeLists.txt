set(qfrost_computations_SRCS computationdata.cpp blockslogger.cpp computationthread.cpp)

add_library(qfrost_computations STATIC ${qfrost_computations_SRCS})
qt5_use_modules(qfrost_computations Widgets)
target_link_libraries(qfrost_computations)
