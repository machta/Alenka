cmake_minimum_required(VERSION 3.1)
project(Neuroscan)

set(SRC
  src/avgtoasc.c
  src/cnttobin.c
  src/errmes.c
  src/getepoch.c
  src/matrix_m.c
  src/cnttoasc.c
  src/eegtoasc.c
  src/getavg.c
  src/matrix2m.c
  src/ns_cnt.c
)

add_library(neuroscan STATIC ${SRC})
