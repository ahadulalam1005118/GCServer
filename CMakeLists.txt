cmake_minimum_required(VERSION 3.12)
file(GLOB server_SRC
        "*.h"
        "*.cpp"
        )
project(GCEventServer)
set(CMAKE_CXX_STANDARD 11)

add_executable(GCEventServer ${server_SRC})
target_link_libraries(GCEventServer "-lpthread")