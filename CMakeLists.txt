CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

project(squirrel_cpp)

add_definitions(-DSQVM_STACK_TRACE=1)
include_directories(${SQUIRREL_INCLUDE})
link_directories(${SQUIRREL_LIB_DIR})

set(${PROJECT_NAME}_headers "sq_vm.h" "sq_console_base.h" "sq_text_console.h")
set(${PROJECT_NAME}_sources "sq_vm.cpp" "sq_text_console.cpp" "test.cpp")

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources})

target_link_libraries(${PROJECT_NAME} sqstdlib_static squirrel_static)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

