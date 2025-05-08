cmake_minimum_required(VERSION 3.10)
project(sdl_project C)

set(CMAKE_C_STANDARD 11)

# Set source files
set(SOURCE_FILES
        src/main.c
        src/RenderWindow.c
        src/Piece.c
        src/Events.c
        src/util.c
)

# Add executable
add_executable(program ${SOURCE_FILES})

# Find SDL2 packages
if(APPLE)
    # macOS specific configuration
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/opt/homebrew/include -I/opt/homebrew/include/SDL2")

    # Find SDL2 libraries
    find_library(SDL2_LIBRARY SDL2 HINTS /opt/homebrew/lib)
    find_library(SDL2_IMAGE_LIBRARY SDL2_image HINTS /opt/homebrew/lib)

    # Add include directories
    include_directories(/opt/homebrew/include /opt/homebrew/include/SDL2)

    # Link libraries
    target_link_libraries(program ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY})
else()
    # Linux/Windows configuration
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(program ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif()