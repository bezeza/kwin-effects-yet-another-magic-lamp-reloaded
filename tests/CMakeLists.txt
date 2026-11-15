# Unit tests for the pure, KWin-free parts of the effect. Run with ctest.
add_executable(coordinatemappertest
    coordinatemappertest.cc
    ../src/geometry/CoordinateMapper.cc
    ../src/geometry/RotationAdapter.cc
    ../src/geometry/ScaleAdapter.cc
)
target_include_directories(coordinatemappertest PRIVATE ../src)
target_link_libraries(coordinatemappertest Qt6::Core Qt6::Test)
add_test(NAME coordinatemappertest COMMAND coordinatemappertest)

add_executable(outputinfotest
    outputinfotest.cc
)
target_include_directories(outputinfotest PRIVATE ../src)
target_link_libraries(outputinfotest Qt6::Core Qt6::Test)
add_test(NAME outputinfotest COMMAND outputinfotest)

add_executable(scaleadaptertest
    scaleadaptertest.cc
    ../src/geometry/ScaleAdapter.cc
)
target_include_directories(scaleadaptertest PRIVATE ../src)
target_link_libraries(scaleadaptertest Qt6::Core Qt6::Test)
add_test(NAME scaleadaptertest COMMAND scaleadaptertest)

add_executable(rotationadaptertest
    rotationadaptertest.cc
    ../src/geometry/RotationAdapter.cc
)
target_include_directories(rotationadaptertest PRIVATE ../src)
target_link_libraries(rotationadaptertest Qt6::Core Qt6::Test)
add_test(NAME rotationadaptertest COMMAND rotationadaptertest)

add_executable(geometryresolvertest
    geometryresolvertest.cc
)
target_include_directories(geometryresolvertest PRIVATE ../src)
target_link_libraries(geometryresolvertest Qt6::Core Qt6::Test)
add_test(NAME geometryresolvertest COMMAND geometryresolvertest)

add_executable(windowtrackertest
    windowtrackertest.cc
)
target_include_directories(windowtrackertest PRIVATE ../src)
target_link_libraries(windowtrackertest Qt6::Core Qt6::Test)
add_test(NAME windowtrackertest COMMAND windowtrackertest)

add_executable(animationlimitstest
    animationlimitstest.cc
)
target_include_directories(animationlimitstest PRIVATE ../src)
target_link_libraries(animationlimitstest Qt6::Core Qt6::Test)
add_test(NAME animationlimitstest COMMAND animationlimitstest)

add_executable(modelgeometrytest
    modelgeometrytest.cc
)
target_include_directories(modelgeometrytest PRIVATE ../src)
target_link_libraries(modelgeometrytest Qt6::Core Qt6::Test)
add_test(NAME modelgeometrytest COMMAND modelgeometrytest)

add_executable(animationrecoverytest
    animationrecoverytest.cc
)
target_include_directories(animationrecoverytest PRIVATE ../src)
target_link_libraries(animationrecoverytest Qt6::Core Qt6::Test)
add_test(NAME animationrecoverytest COMMAND animationrecoverytest)
