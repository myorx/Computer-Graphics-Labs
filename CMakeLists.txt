cmake_minimum_required(VERSION 3.18)
project(ComputerGraphicsLabs LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_path(GLM_INCLUDE_DIR glm/glm.hpp REQUIRED)

foreach(lab IN ITEMS 1 2 3)
    add_executable(lab${lab} Lab${lab}/main.cpp)
    target_compile_features(lab${lab} PRIVATE cxx_std_17)
    target_link_libraries(lab${lab} PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
endforeach()

target_include_directories(lab2 PRIVATE "${GLM_INCLUDE_DIR}")
target_include_directories(lab3 PRIVATE "${GLM_INCLUDE_DIR}")
