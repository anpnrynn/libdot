cmake_minimum_required(VERSION 3.10)
project(libdot_cpp11 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dot STATIC src/dot.cpp src/dot_c_api.cpp)
target_include_directories(dot PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(dot PROPERTIES OUTPUT_NAME dot)

add_executable(dot_dump examples/dot_dump.cpp)
target_link_libraries(dot_dump PRIVATE dot)

add_executable(dot_tests tests/test_dot.cpp)
target_link_libraries(dot_tests PRIVATE dot)

enable_testing()
add_test(NAME dot_tests COMMAND dot_tests)

include(GNUInstallDirs)
install(TARGETS dot dot_dump
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/libdot_cpp11)
