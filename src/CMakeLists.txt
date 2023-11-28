file(GLOB_RECURSE SRC_DIR_LIST "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.cpp")
# set(CMAKE_CXX_FLAGS "-fcoroutines ${CMAKE_CXX_FLAGS}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(continue main.cpp ${SRC_DIR_LIST})
target_link_libraries(continue uring)
set_target_properties(continue PROPERTIES
        CXX_STANDARD 20
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

add_executable(main main.cpp)
target_link_libraries(main continue)
