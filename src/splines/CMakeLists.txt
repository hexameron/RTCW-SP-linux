cmake_minimum_required(VERSION 2.8)

# Get all C source files
file(GLOB CXX_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

# Sort the C sources into a neat list
list(SORT CXX_SOURCES)

# Set the compiler to use
set_source_files_properties(${CXX_SOURCES} PROPERTIES LANGUAGE CXX COMPILE_FLAGS "-Wno-write-strings")

# Add to our library.
add_library(splines.a STATIC ${CXX_SOURCES})
set_target_properties(splines.a PROPERTIES LINKER_LANGUAGE CXX PREFIX "" SUFFIX "")
