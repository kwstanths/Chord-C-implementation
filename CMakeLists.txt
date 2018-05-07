cmake_minimum_required (VERSION 3.0)


set(NAME "chord")
PROJECT(${NAME})

add_subdirectory(src/communication/)
add_subdirectory(src/chord_emu)
add_subdirectory(src/unit_test)
