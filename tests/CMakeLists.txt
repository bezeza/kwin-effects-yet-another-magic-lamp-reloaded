# Unit tests for the pure, KWin-free parts of the effect. Run with ctest.
add_executable(coordinatemappertest
    coordinatemappertest.cc
    ../src/geometry/CoordinateMapper.cc
)
target_include_directories(coordinatemappertest PRIVATE ../src)
target_link_libraries(coordinatemappertest Qt6::Core Qt6::Test)
add_test(NAME coordinatemappertest COMMAND coordinatemappertest)
