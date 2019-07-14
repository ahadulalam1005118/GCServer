cmake_minimum_required(VERSION 3.8)
file(GLOB server_SRC
        "*.h"
        "*.cpp"
        )
project(GCServer)
set(CMAKE_CXX_STANDARD 11)

add_executable(GCServer ${server_SRC})
target_link_libraries(GCServer "-lpthread")