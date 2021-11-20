cmake_minimum_required(VERSION 3.10)

set(EXE_NAME prefix_tree)

project(${EXE_NAME} VERSION 1.0.0 DESCRIPTION "prefix_tree")

add_compile_options(
   -std=gnu11
  -O3 
  -Wall 
  -Wextra 
  -Werror
  -g 
  -D_GNU_SOURCE 
)

include(GNUInstallDirs)

SET(SOURCES 
  main.c
  prefix_tree.c
  prefix_tree.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} 
  PROPERTIES 
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME ${EXE_NAME}
)

install(TARGETS ${PROJECT_NAME} 
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

