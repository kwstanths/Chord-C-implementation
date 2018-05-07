cmake_minimum_required(VERSION 3.0)


# Environment
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# Project
set(NAME "communication")
project(${NAME})
	
# Sources
file(GLOB ${NAME}_HEADERS *.hpp)
file(GLOB ${NAME}_SOURCES *.cpp)

# Target
add_library(
	${NAME} ${${NAME}_HEADERS} ${${NAME}_SOURCES} 
	)

