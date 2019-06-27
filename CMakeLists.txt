cmake_minimum_required(VERSION 3.12)
project(DMTX VERSION 0.7.5 LANGUAGES C)


macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()


add_library(dmtx dmtx.c)
target_link_libraries(dmtx -lm)

#------------------------------------------------------------------------------#
enable_testing()
add_executable(simple
  test/simple_test/simple_test.c)
target_link_libraries(simple PRIVATE dmtx)
add_test(NAME simpleTest COMMAND simple)


find_package(SDL)
find_library(SDL_GFX_LIBRARY SDL_gfx)

include_directories(${SDL_INCLUDE_DIR} ${SDL_IMAGE_INCLUDE_DIR})

add_executable(multi
  test/multi_test/dmtx.c
  test/multi_test/dmtxaccel.c
  test/multi_test/dmtxdecode2.c
  test/multi_test/dmtxhough.c
  test/multi_test/dmtxregion2.c
  test/multi_test/dmtxsobel.c
  test/multi_test/dmtxvaluegrid.c
  test/multi_test/kiss_fft.c
  test/multi_test/kiss_fftr.c
  test/multi_test/multi_test.c
  test/multi_test/multi_test.h
  test/multi_test/visualize.c)

target_link_libraries(multi PRIVATE dmtx ${SDL_LIBRARY} ${SDL_IMAGE_LIBRARY} ${SDL_GFX_LIBRARY})
add_test(NAME multiTest COMMAND multi)

#------------------------------------------------------------------------------#
# this test doesn't work yet (nothing wrong with the code - something wrong with my script)
# add_executable(unit
#   test/unit_test/unit_test.c)
# target_link_libraries(unit PRIVATE dmtx)
# add_test(NAME unitTest COMMAND unit)

