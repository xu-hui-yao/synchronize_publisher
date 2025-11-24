cmake_minimum_required(VERSION 3.16)

project(monitor VERSION 1.0)

add_executable(${PROJECT_NAME}
        main.cpp
        monitor.cpp
        monitor.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        infinite_sense_core
)
