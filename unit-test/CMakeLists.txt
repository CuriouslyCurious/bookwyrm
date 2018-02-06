add_executable(bookwyrm-test main.cpp)

target_include_directories(bookwyrm-test BEFORE PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(bookwyrm-test bookwyrm-core)
