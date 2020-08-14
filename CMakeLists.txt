cmake_minimum_required(VERSION 2.8.3)
project(csapex_python)

include(ExternalProject)

find_package(PythonLibs REQUIRED)


set(NUMPYOPENCV_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/external/numpy-converter/libnp_opencv_converter.so)
set(NUMPYOPENCV_MODULE ${CMAKE_CURRENT_BINARY_DIR}/external/numpy-converter/np_opencv_module.so)
set(NUMPYOPENCV_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

add_subdirectory(external)

find_package(catkin REQUIRED COMPONENTS
  csapex csapex_opencv csapex_point_cloud
)

find_package(Boost REQUIRED COMPONENTS python)


find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
add_definitions(-DQT_NO_KEYWORDS)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
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
    src/python_wrapper.cpp
)
target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${catkin_LIBRARIES}
  ${NUMPYOPENCV_LIBRARY}
  Qt5::Core Qt5::Gui Qt5::Widgets
)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/external/numpy-converter
)
add_dependencies(${PROJECT_NAME} numpyopencv)

SET(PY_PROJECT_NAME ${PROJECT_NAME})
set(PYTHON_PACKAGE_NAME "csapex")


set(TARGET_NAME ${PY_PROJECT_NAME})

get_property(PYLIB_OUTPUT_FILE TARGET ${TARGET_NAME} PROPERTY LOCATION)
get_filename_component(PYLIB_OUTPUT_NAME ${PYLIB_OUTPUT_FILE} NAME_WE)
set(PYLIB_SO_NAME ${PYLIB_OUTPUT_NAME}.so)


if(${PYTHON_VERSION_STRING} VERSION_GREATER_EQUAL 3.0.0)
  set(DIST_DIR python3/dist-packages)
else()
  set(DIST_DIR python2.7/dist-packages)
endif()

install(TARGETS ${TARGET_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${DIST_DIR}/${PYTHON_PACKAGE_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${DIST_DIR}/${PYTHON_PACKAGE_NAME}
)

# Cause the library to be output in the correct directory.
set(PYTHON_LIB_DIR ${CATKIN_DEVEL_PREFIX}/lib/${DIST_DIR}/${PYTHON_PACKAGE_NAME})
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

add_library(${PROJECT_NAME}_core
    src/register_python.cpp
)

target_link_libraries(${PROJECT_NAME}_core
    ${PROJECT_NAME}
    ${catkin_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
)


add_library(${PROJECT_NAME}_node
    src/python_node.cpp
)

target_link_libraries(${PROJECT_NAME}_node
    ${PROJECT_NAME}
    ${catkin_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

add_library(${PROJECT_NAME}_qt
    src/python_node_adapter.cpp
)

target_link_libraries(${PROJECT_NAME}_qt
    ${PROJECT_NAME}
    ${catkin_LIBRARIES}
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

#
# INSTALL
#

install(FILES plugins.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(TARGETS ${PROJECT_NAME}_core
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
install(TARGETS ${PROJECT_NAME}_node
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
install(TARGETS ${PROJECT_NAME}_qt
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})

#install(DIRECTORY include/${PROJECT_NAME}/
#        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
