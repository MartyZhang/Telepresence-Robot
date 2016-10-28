cmake_minimum_required(VERSION 2.8)

# Find the required packages
find_package(OpenCV REQUIRED)

# Add sources
set(SOURCES
    "videoStream.cpp"
)


# Build the executable
add_executable(videoStreamer ${SOURCES})

# Add module include directories
target_include_directories(videoStreamer
    PUBLIC
        ${OpenCV_INCLUDE_DIRS}
)

# Link required libraries
target_link_libraries(videoStreamer ${OpenCV_LIBS})