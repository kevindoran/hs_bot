project(hypersonic_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(gtest_SOURCE_DIR lib/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_subdirectory(${gtest_SOURCE_DIR})

add_executable(runTests
#        input_parser_test.cpp
        mechanics_test.cpp
        board_test.cpp
        bot_test.cpp
        annealing_bot_test.cpp
        )
target_link_libraries(runTests gtest gtest_main)
target_link_libraries(runTests hypersonic)

add_executable(profile
        bot_test.cpp)
target_link_libraries(profile gtest gtest_main)
target_link_libraries(profile hypersonic)