cmake_minimum_required (VERSION 3.0)

#Environment
set (PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)

set (NAME "unit_test")
project (${NAME})

set(ALL_LIBS
    communication
)

include_directories(${PROJECT_ROOT}/src/communication)

file(GLOB ${NAME}_HEADERS *.hpp)
file(GLOB ${NAME}_SOURCES *.cpp)

add_executable(${NAME}
    ${${NAME}_HEADERS} 
    ${${NAME}_SOURCES}
)

target_link_libraries(${NAME}
	${ALL_LIBS}
)

