include_directories(../)
add_executable(sf sf-executable.hpp sf-executable.cpp)
target_link_libraries(sf sunflower)