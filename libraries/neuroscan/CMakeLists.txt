cmake_minimum_required(VERSION 3.1)
project(Neuroscan)

set(SRC
  src/errmes.c
  src/matrix_m.c
  
  src/matrix2m.c
  src/ns_cnt.c
)

add_library(neuroscan STATIC ${SRC})
