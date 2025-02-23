cmake_minimum_required (VERSION 3.16)
project(QppCAD_Launcher CXX C)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets OpenGL Svg Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets OpenGL Svg Charts)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Charts_INCLUDE_DIRS})

# WIN32
add_executable(${PROJECT_NAME} WIN32 laucher_main.cpp ${CMAKE_SOURCE_DIR}/resources.rc)
# target_compile_options(${PROJECT_NAME} PRIVATE "/utf8")

target_link_libraries(${PROJECT_NAME}
    qpp
    pugixml
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Charts
    Qt5::Svg
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${PRODUCT_OUT_DIR})
