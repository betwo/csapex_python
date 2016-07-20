cmake_minimum_required(VERSION 2.8.3)
project(csapex_python)

include(ExternalProject)

find_package(PythonLibs REQUIRED)

ExternalProject_add(numpyopencv
  GIT_REPOSITORY https://github.com/spillai/numpy-opencv-converter.git
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_COMMAND cp np_opencv_converter.so  ${CMAKE_CURRENT_BINARY_DIR}
  SOURCE_DIR numpy-opencv
  BINARY_DIR numpy-opencv-build
  CMAKE_ARGS ${COMMON_EXTERNAL_PROJECT_ARGS}
  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
  INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}

  CMAKE_ARGS -DPYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS} -DPYTHON_LIBRARY=${PYTHON_LIBRARY}
)
set(NUMPYOPENCV_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/numpy-opencv-build/np_opencv_converter.so)
set(NUMPYOPENCV_MODULE ${CMAKE_CURRENT_BINARY_DIR}/numpy-opencv-build/np_opencv_module.so)
set(NUMPYOPENCV_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)


find_package(catkin REQUIRED COMPONENTS
  csapex csapex_vision csapex_point_cloud
)

find_package(Boost REQUIRED COMPONENTS python)


find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES csapex_python
  CATKIN_DEPENDS csapex
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS}
)



catkin_python_setup()

add_library(${PROJECT_NAME}
    src/python_apex_api.cpp
)
target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${catkin_LIBRARIES}
  ${NUMPYOPENCV_LIBRARY}
  csapex
  csapex_qt
  Qt5::Core Qt5::Gui Qt5::Widgets
)

SET(PY_PROJECT_NAME ${PROJECT_NAME})
set(PYTHON_PACKAGE_NAME "csapex")


set(TARGET_NAME ${PY_PROJECT_NAME})

get_property(PYLIB_OUTPUT_FILE TARGET ${TARGET_NAME} PROPERTY LOCATION)
get_filename_component(PYLIB_OUTPUT_NAME ${PYLIB_OUTPUT_FILE} NAME_WE)
set(PYLIB_SO_NAME ${PYLIB_OUTPUT_NAME}.so)

SET(DIST_DIR dist-packages)

install(TARGETS ${TARGET_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python2.7/${DIST_DIR}/${PYTHON_PACKAGE_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python2.7/${DIST_DIR}/${PYTHON_PACKAGE_NAME}
)

# Cause the library to be output in the correct directory.
set(PYTHON_LIB_DIR ${CATKIN_DEVEL_PREFIX}/lib/python2.7/${DIST_DIR}/${PYTHON_PACKAGE_NAME})
add_custom_command(TARGET ${TARGET_NAME}
  POST_BUILD
  COMMAND mkdir -p ${PYTHON_LIB_DIR} && cp -v ${PYLIB_OUTPUT_FILE} ${PYTHON_LIB_DIR}/${PYLIB_SO_NAME}
  WORKING_DIRECTORY ${CATKIN_DEVEL_PREFIX}
  COMMENT "Copying library files to python directory" )
add_custom_command(TARGET numpyopencv
  POST_BUILD
  COMMAND mkdir -p ${PYTHON_LIB_DIR} && cp -v ${NUMPYOPENCV_MODULE} ${PYTHON_LIB_DIR}/np_opencv_module.so
  WORKING_DIRECTORY ${CATKIN_DEVEL_PREFIX}
  COMMENT "Copying library files to python directory" )



add_library(csapex_python_node
    src/python_node.cpp
)

target_link_libraries(csapex_python_node
    csapex_python
    ${catkin_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

add_library(csapex_python_qt
    src/python_node_adapter.cpp
)

target_link_libraries(csapex_python_qt
    csapex_python
    ${catkin_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

#
# INSTALL
#

install(FILES plugins.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(TARGETS csapex_python_node
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
install(TARGETS csapex_python_qt
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})

install(TARGETS csapex_python
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})

#install(DIRECTORY include/${PROJECT_NAME}/
#        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
