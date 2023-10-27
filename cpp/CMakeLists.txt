cmake_minimum_required(VERSION 2.8)

set(APP_NAME main)
project( ${APP_NAME} ) 

add_executable( ${APP_NAME} ${APP_NAME}.cpp )

# OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( ${APP_NAME} PRIVATE ${OpenCV_LIBS} gflags -lstdc++fs)

# OnnxRuntime
set(ONNXRUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/externals/onnxruntime-linux-x64-1.15.1")
include_directories(${ONNXRUNTIME_ROOT}/include)
target_link_libraries(${APP_NAME} PRIVATE ${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so.1.15.1)

# Link head
target_include_directories( ${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)