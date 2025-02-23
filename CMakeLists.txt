cmake_minimum_required (VERSION 3.16)
project(QppCAD CXX C)

set(CMAKE_PREFIX_PATH "D:/envs/Qt5/5.15.2/msvc2019_64/lib/cmake/Qt5")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmakes/cpack_config.cmake)

set(PRODUCT_OUT_DIR ".")
set(CMAKE_INSTALL_PREFIX ${PRODUCT_OUT_DIR})
set(CPACK_INSTALL_PREFIX "QppCAD")
set(DEPLOY_DIR ${CMAKE_SOURCE_DIR}/deploy)
file(MAKE_DIRECTORY ${DEPLOY_DIR})


SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "\${ORIGIN}")

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type (default RelWithDebInfo)" FORCE)
endif()
message("-- Build type: ${CMAKE_BUILD_TYPE}, config type: ${CMAKE_CONFIGURATION_TYPES}")


set(QPPCAD_TESTS "OFF" CACHE BOOL "build test suite and minor stuff")
set(WITH_VTUNE "OFF" CACHE BOOL "build with VTUNE instrumentation")

add_subdirectory(src/launcher)

set(ADDITIONAL_LIBRARIES "")
message("-- With VTune api: ${WITH_VTUNE}")
if(WITH_VTUNE)
    set(VTUNE_TOP_DIR "/home/$ENV{USERNAME}/intel/vtune_profiler")
    include_directories(${VTUNE_TOP_DIR}/sdk/include)
    add_library(ittnotify STATIC IMPORTED)
    set_target_properties(ittnotify PROPERTIES IMPORTED_LOCATION ${VTUNE_TOP_DIR}/sdk/lib64/libittnotify.a)
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ittnotify)
    add_definitions(-DWITH_VTUNE_INSTRUMENTATION)
endif()

add_definitions(-DFMT_HEADER_ONLY)
if(NOT MSVC)
  add_definitions(-fPIC)
  set(CMAKE_CXX_FLAGS "-std=c++17 -fno-omit-frame-pointer -fvisibility=hidden -pthread")
  #set(CMAKE_CXX_FLAGS "-Wextra -Wpedantic -std=c++17 -fno-omit-frame-pointer -fvisibility=hidden -pthread")
  set(CMAKE_CXX_FLAGS_DEBUG
      "-DQPPCAD_DEBUG ${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer -g3 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE
      "-DQPPCAD_RELEASE ${CMAKE_CXX_FLAGS} -O3 -ffast-math -fno-finite-math-only")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
      "-DQPPCAD_RELEASE ${CMAKE_CXX_FLAGS} -O3 -ffast-math -fno-finite-math-only -g3 -ggdb")
elseif(MSVC)
  add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE -DHAVE_SNPRINTF)
  add_compile_options("/std:c++17")
  add_compile_options("/W0")
  add_compile_options("/bigobj")
endif()

#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
set(OpenGL_GL_PREFERENCE "GLVND")
set(CMAKE_CXX_STANDARD 17)

macro(set_option option value)
  set(${option} ${value} CACHE INTERNAL "" FORCE)
endmacro()

#to prevent building qpp python bindings
set_option(PYTHON_BINDINGS ON)
set_option(BUILD_TESTS OFF)
set_option(BUILD_EXAMPLES OFF)
set_option(BUILD_QPP_TESTS OFF)

find_package(Doxygen)

add_subdirectory(deps/qpp)

include_directories(src/)
include_directories(data/)
include_directories(deps/qpp/modules)
include_directories(deps/qpp/deps/pybind11/include)
include_directories(deps/qpp/deps/fmtlib/include)
include_directories(deps/qpp/deps/pugixml/src)
include_directories(deps/qpp/deps/CppNumericalSolvers/include)
include_directories(deps/qpp/deps/eigen3)
include_directories(deps/qpp/deps)
include_directories(deps/fifo_map)
include_directories(deps)
include_directories(${PYTHON_INCLUDE_DIRS})

file(GLOB_RECURSE ide_cpp_qppcad "src/qppcad/*.cpp")
file(GLOB_RECURSE ide_hpp_qppcad "src/qppcad/*.hpp")
#file(GLOB_RECURSE ide_t1 "tests/*/*.*pp")
#file(GLOB_RECURSE ide_t2 "tests/*.*pp")

add_custom_target(cad_files SOURCES ${ide_cpp_qppcad} ${ide_hpp_qppcad})

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
  message(WARNING "ERROR: OpenGL not found")
endif(NOT OPENGL_FOUND)
set(GL_LIBRARY GL  X11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src/qppcad/"
    OUTPUT_VARIABLE _build_version
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  message( STATUS "GIT hash: ${_build_version}")
else()
  message(STATUS "GIT not found")
  set(_build_version 1)
endif()

string(TIMESTAMP _time_stamp)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets OpenGL Svg Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets OpenGL Svg Charts)

set(_qppcad_qt_build_version ${Qt5Widgets_VERSION})
configure_file(${CMAKE_SOURCE_DIR}/src/qppcad/core/gitversion_ex.h.in
               ${CMAKE_SOURCE_DIR}/src/qppcad/core/gitversion_ex.h @ONLY)

message(STATUS "qt include: ${Qt5Core_INCLUDE_DIRS}")
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Charts_INCLUDE_DIRS})
include_directories(deps/qpp/tests)

qt5_add_resources(QRCS data/resources.qrc)

#gather ws_item src
file(GLOB_RECURSE SRC_WS_ITEM "src/qppcad/ws_item/*.cpp")

#gather ui src
file(GLOB_RECURSE SRC_UI "src/qppcad/ui/*.cpp")

#gather tools src
file(GLOB_RECURSE SRC_TOOLS "src/qppcad/tools/*.cpp")

#gather python src
file(GLOB_RECURSE SRC_PYTHON_EMBEDDED "src/qppcad/python/*.cpp")

#gather render src
file(GLOB_RECURSE SRC_RENDER "src/qppcad/render/*.cpp")

#qppcad test suite and other minor stuff
#build sflow tests
#build qppcad application
if (QPPCAD_TESTS)
    add_executable(
        sflow-tests
        src/qppcad/sflow/sflow_test.cpp
        src/qppcad/sflow/sflow.cpp
        src/qppcad/sflow/sflow_node.cpp
        src/qppcad/sflow/sflow_base_nodes_int.cpp
        src/qppcad/sflow/sflow_base_nodes_float.cpp
        src/qppcad/sflow/sflow_base_nodes_vec3.cpp
        src/qppcad/sflow/sflow_base_nodes_xgeom.cpp
        src/qppcad/sflow/sflow_parameter.cpp
        src/qppcad/core/type_info.cpp
        src/qppcad/core/string_hash_register.cpp
        src/qppcad/core/qpp_object.cpp
        src/qppcad/core/history_stream.cpp
        )

    add_executable(
        qppcad-tests
        src/qppcad/core/history_stream.cpp
        src/qppcad/core/qppcad_tests.cpp
        )

    add_executable(
        qppcad-misc-tests
        src/qppcad/ui/color_map.cpp
        src/qppcad/core/qppcad_misc_tests.cpp
        )

    target_link_libraries(qppcad-tests qpp)
    target_link_libraries(sflow-tests qpp)
endif()

#build qppcad application
add_executable(
    ${PROJECT_NAME}

    src/qppcad/core/main.cpp
    src/qppcad/core/gitversion.cpp
    src/qppcad/core/ittnotify_support.cpp

    src/qppcad/core/app_state.cpp
    src/qppcad/core/app_state_event_disp.cpp

    src/qppcad/core/workspace.cpp
    src/qppcad/core/string_hash_register.cpp
    src/qppcad/core/type_info.cpp
    src/qppcad/core/qpp_object.cpp
    src/qppcad/core/register_all_things.cpp
    src/qppcad/core/gizmo.cpp
    src/qppcad/core/hotkey_manager.cpp
    src/qppcad/core/embedded_cluster_tools.cpp
    src/qppcad/core/fixture.cpp
    src/qppcad/core/platform.cpp
    src/qppcad/core/history_stream.cpp

    src/qppcad/sflow/sflow.cpp
    src/qppcad/sflow/sflow_node.cpp
    src/qppcad/sflow/sflow_base_nodes_int.cpp
    src/qppcad/sflow/sflow_base_nodes_float.cpp
    src/qppcad/sflow/sflow_base_nodes_vec3.cpp
    src/qppcad/sflow/sflow_base_nodes_xgeom.cpp
    src/qppcad/sflow/sflow_base_nodes_ws_item.cpp
    src/qppcad/sflow/sflow_parameter.cpp
    src/qppcad/sflow/sflow_parameter_ws_item.cpp

    deps/qpp/modules/pyqpp/pyqpp_linalg.cpp
    deps/qpp/modules/pyqpp/pyqpp_geom.cpp
    deps/qpp/modules/pyqpp/pyqpp_xgeom.cpp
    deps/qpp/modules/pyqpp/pyqpp_cell.cpp
    deps/qpp/modules/pyqpp/pyqpp_autosymm.cpp
    deps/qpp/modules/pyqpp/pyqpp_builders.cpp
    deps/qpp/modules/pyqpp/pyqpp_ccd.cpp
    deps/qpp/modules/pyqpp/pyqpp_opaque_types.cpp
    deps/qpp/modules/data/errors.cpp

    ${SRC_RENDER}
    ${SRC_WS_ITEM}
    ${SRC_UI}
    ${SRC_TOOLS}
    ${SRC_PYTHON_EMBEDDED}
    ${QRCS}
    resources.rc
    )

if(WIN32)
   target_link_libraries(${PROJECT_NAME}
    qpp
    pugixml
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Charts
    Qt5::Svg
    ${OPENGL_LIBRARIES}
    pybind11::embed
    ${PYTHON_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME}
    qpp
    pugixml
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Charts
    Qt5::Svg
    ${OPENGL_LIBRARIES}
    pybind11::embed
    ${PYTHON_LIBRARIES}
    ${ADDITIONAL_LIBRARIES}
    pthread
    dl
    util
    m)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC -DQPPCAD_PY_EXPORT -DEXTENDED_SFLOW)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_SOURCE_DIR}/docs/doxygen.conf"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
        COMMENT "Building user's documentation into doxyDoc build dir..."
    )
endif()

#-${_time_stamp}-${_build_version}
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${_build_version}-py312")

get_target_property(_QMAKE_EXE_PATH Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_QT_BIN_PATH "${_QMAKE_EXE_PATH}" DIRECTORY)
find_program(_DEPLOYQT_EXE windeployqt HINTS "${_QT_BIN_PATH}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E env PATH="${_QT_BIN_PATH}"
        ${_DEPLOYQT_EXE} $<TARGET_FILE:${PROJECT_NAME}> --dir ${DEPLOY_DIR}
        COMMENT "Running windeployqt..."
        )

# install(TARGETS pyqpp RUNTIME DESTINATION ${PRODUCT_OUT_DIR})
install(FILES ${CMAKE_SOURCE_DIR}/data/images/qpp128x128.ico DESTINATION ${PRODUCT_OUT_DIR})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${PRODUCT_OUT_DIR})
install(DIRECTORY ${DEPLOY_DIR}/ DESTINATION ${PRODUCT_OUT_DIR} FILES_MATCHING PATTERN "*")

install(FILES LICENSE README.md DESTINATION doc)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/refs DESTINATION data)

include(CPack)
