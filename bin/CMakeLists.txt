include_directories(../)
add_executable(sfexe sf-executable.hpp sf-executable.cpp)
target_link_libraries(sfexe sunflower)

set_target_properties(sfexe PROPERTIES OUTPUT_NAME sunflower)

install(
    TARGETS sfexe
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    DESTINATION bin
    COMPONENT Runtime
)