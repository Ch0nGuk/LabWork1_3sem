cmake_minimum_required(VERSION 3.16)
project(LabWork1_3sem LANGUAGES CXX)

add_executable(lab_app main.cpp UI.cpp tests.cpp)
set_target_properties(lab_app PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
